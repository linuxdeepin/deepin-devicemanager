// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | ControlInterface::ControlInterface | low | - | 1 | 1 |
 * | ControlInterface::disableInDevice | low | - | 1 | 2 |
 * | ControlInterface::disableOutDevice | low | - | 1 | 3 |
 * | ControlInterface::init | low | - | 1 | 1 |
 * | ControlInterface::monitorWorkingDBFlag | low | - | 1 | 5 |
 * | ControlInterface::setMonitorWorkingDBFlag | low | - | 1 | 2 |
 * | ControlInterface::updateWakeup | low | - | 1 | 2 |
 */

#include <gtest/gtest.h>

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QVariant>

#include "stubext.h"
#include "controlinterface.h"

using namespace stub_ext;

// QDBusAbstractInterface::doCall 是 Qt6 DBus 的私有导出符号（call 内联模板的最终落点）。
// 显式实例化访问私有成员的技巧已被 GCC12 拒绝，此处直接按 mangled name 取地址打桩。
extern "C" QDBusMessage _ZN22QDBusAbstractInterface6doCallEN5QDBus8CallModeERK7QStringPK8QVariantm(
    QDBusAbstractInterface *self, QDBus::CallMode mode, const QString &method,
    const QVariant *args, size_t numArgs);

namespace {
// Qt 6.8 的 QDBusAbstractInterface::call 是内联模板，内部转发到私有的 doCall；
// 测试通过 addr_pri.h 的私有函数访问补丁直接拦截 doCall
using DoCallFn = QDBusMessage (*)(QDBusAbstractInterface *, QDBus::CallMode, const QString &,
                                  const QVariant *, size_t);

// DBus 调用记录
struct DoCallRecord
{
    int calls = 0;
    QString method;
    QVariant firstArg;
    bool hasFirstArg = false;
};

// 构造合法应答消息（type == ReplyMessage），供 QDBusReply 解析
QDBusMessage validReply(const QVariant &value)
{
    QDBusMessage request = QDBusMessage::createMethodCall("org.ut.Service", "/ut/path", "", "stub");
    return request.createReply(QList<QVariant> { value });
}

// doCall 打桩：记录调用并返回指定应答；replyValid 为 false 时返回无效应答
void stubDoCall(stub_ext::StubExt &stub, DoCallRecord *record,
                const QVariant &replyValue = QVariant(), bool replyValid = false)
{
    DoCallFn fp = &_ZN22QDBusAbstractInterface6doCallEN5QDBus8CallModeERK7QStringPK8QVariantm;
    stub.set_lamda(fp,
                   [record, replyValue, replyValid](QDBusAbstractInterface *, QDBus::CallMode,
                                                    const QString &method, const QVariant *args,
                                                    size_t numArgs) -> QDBusMessage {
                       ++record->calls;
                       record->method = method;
                       if (numArgs > 0) {
                           record->firstArg = args[0];
                           record->hasFirstArg = true;
                       }
                       if (replyValid)
                           return validReply(replyValue);
                       return QDBusMessage();
                   });
}

// ControlInterface 单例的系统总线打桩，保证测试不触达真实 DBus
void stubInvalidSystemBus(stub_ext::StubExt &stub)
{
    stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                   []() -> QDBusConnection { return QDBusConnection(QString()); });
}
} // namespace

class ControlInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stubInvalidSystemBus(stub);
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ---------- ControlInterface::ControlInterface / getInstance (controlinterface.cpp:23-29) ----------
TEST_F(ControlInterfaceTest, Ctor_AfterConstruction_InitializesDBusInterfaceObject)
{
    // Arrange
    // （fixture 已打桩 systemBus 环境，无额外前置条件）
    // Act
    ControlInterface *ci = new ControlInterface();
    // Assert
    EXPECT_NE(nullptr, ci->m_iface);   // 构造完成即建立 DBus 接口对象
    EXPECT_EQ(QString("org.deepin.DeviceControl"), ci->m_iface->service());
    delete ci;
}

TEST_F(ControlInterfaceTest, GetInstance_RepeatedCalls_ReturnsSameSingletonPointer)
{
    // Arrange：首次获取单例作为基准
    ControlInterface *first = ControlInterface::getInstance();
    // Act：再次获取
    ControlInterface *second = ControlInterface::getInstance();
    // Assert
    EXPECT_EQ(first, second);
    EXPECT_NE(nullptr, second);
}

// ---------- ControlInterface::disableOutDevice (controlinterface.cpp:31-40) ----------
TEST_F(ControlInterfaceTest, DisableOutDevice_ValidIface_CallsDBusWithDevInfo)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->disableOutDevice("hwinfo-output");

    // Assert
    EXPECT_EQ(1, record.calls);
    EXPECT_EQ("disableOutDevice", record.method);
    ASSERT_TRUE(record.hasFirstArg);
    EXPECT_EQ("hwinfo-output", record.firstArg.toString());
    delete ci;
}

TEST_F(ControlInterfaceTest, DisableOutDevice_InvalidIface_SkipsDBusCall)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return false; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->disableOutDevice("hwinfo-output");

    // Assert
    EXPECT_EQ(0, record.calls);
    EXPECT_FALSE(ci->m_iface->isValid());   // 无效接口，跳过 DBus 调用
    delete ci;
}

// m_iface 为空时直接跳过 DBus 调用，不崩溃
TEST_F(ControlInterfaceTest, DisableOutDevice_NullIface_SkipsDBusCall)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    ControlInterface *ci = new ControlInterface();
    ci->m_iface = nullptr;
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->disableOutDevice("hwinfo-output");

    // Assert
    EXPECT_EQ(0, record.calls);
    EXPECT_EQ(nullptr, ci->m_iface);   // 空接口直接跳过
    delete ci;
}

// ---------- ControlInterface::disableInDevice (controlinterface.cpp:42-48) ----------
TEST_F(ControlInterfaceTest, DisableInDevice_ValidIface_CallsDBusWithoutArgs)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->disableInDevice();

    // Assert
    EXPECT_EQ(1, record.calls);
    EXPECT_EQ("disableInDevice", record.method);
    EXPECT_FALSE(record.hasFirstArg);
    delete ci;
}

TEST_F(ControlInterfaceTest, DisableInDevice_InvalidIface_SkipsDBusCall)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return false; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->disableInDevice();

    // Assert
    EXPECT_EQ(0, record.calls);
    EXPECT_FALSE(ci->m_iface->isValid());   // 无效接口，跳过 DBus 调用
    delete ci;
}

// ---------- ControlInterface::updateWakeup (controlinterface.cpp:50-56) ----------
TEST_F(ControlInterfaceTest, UpdateWakeup_ValidIface_CallsDBusWithDevInfo)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->updateWakeup("wakeup-dev");

    // Assert
    EXPECT_EQ(1, record.calls);
    EXPECT_EQ("updateWakeup", record.method);
    ASSERT_TRUE(record.hasFirstArg);
    EXPECT_EQ("wakeup-dev", record.firstArg.toString());
    delete ci;
}

TEST_F(ControlInterfaceTest, UpdateWakeup_InvalidIface_SkipsDBusCall)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return false; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->updateWakeup("wakeup-dev");

    // Assert
    EXPECT_EQ(0, record.calls);
    EXPECT_FALSE(ci->m_iface->isValid());   // 无效接口，跳过 DBus 调用
    delete ci;
}

// ---------- ControlInterface::setMonitorWorkingDBFlag (controlinterface.cpp:58-64) ----------
TEST_F(ControlInterfaceTest, SetMonitorWorkingDBFlag_ValidIface_CallsDBusWithFlag)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->setMonitorWorkingDBFlag(true);
    // Assert
    EXPECT_EQ(1, record.calls);
    EXPECT_EQ("setMonitorWorkingDBFlag", record.method);
    ASSERT_TRUE(record.hasFirstArg);
    EXPECT_TRUE(record.firstArg.toBool());

    record = DoCallRecord();
    ci->setMonitorWorkingDBFlag(false);
    EXPECT_EQ(1, record.calls);
    ASSERT_TRUE(record.hasFirstArg);
    EXPECT_FALSE(record.firstArg.toBool());
    delete ci;
}

TEST_F(ControlInterfaceTest, SetMonitorWorkingDBFlag_InvalidIface_SkipsDBusCall)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return false; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    ci->setMonitorWorkingDBFlag(true);

    // Assert
    EXPECT_EQ(0, record.calls);
    EXPECT_FALSE(ci->m_iface->isValid());   // 无效接口，跳过 DBus 调用
    delete ci;
}

// ---------- ControlInterface::monitorWorkingDBFlag (controlinterface.cpp:66-76) ----------
TEST_F(ControlInterfaceTest, MonitorWorkingDBFlag_ValidReplyFalse_ReturnsFalse)
{
    // Act
    // Arrange
    // （fixture 已打桩默认环境，无额外前置条件）
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record, QVariant(false), true);

    // Assert
    EXPECT_FALSE(ci->monitorWorkingDBFlag());
    EXPECT_EQ(1, record.calls);
    delete ci;
}

TEST_F(ControlInterfaceTest, MonitorWorkingDBFlag_ValidReplyTrue_ReturnsTrue)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record, QVariant(true), true);

    // Assert
    EXPECT_TRUE(ci->monitorWorkingDBFlag());
    EXPECT_EQ(1, record.calls);   // doCall 恰好执行一次
    delete ci;
}

// 分支：应答消息非法（QDBusReply 无效）-> 兜底返回 true
TEST_F(ControlInterfaceTest, MonitorWorkingDBFlag_InvalidReply_ReturnsTrue)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return true; });
    DoCallRecord record;
    stubDoCall(stub, &record);

    // Assert
    EXPECT_TRUE(ci->monitorWorkingDBFlag());
    EXPECT_EQ(1, record.calls);   // 回复无效仍执行一次调用
    delete ci;
}

// 分支：m_iface 无效 -> 兜底返回 true
TEST_F(ControlInterfaceTest, MonitorWorkingDBFlag_InvalidIface_ReturnsTrue)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                   [](QDBusAbstractInterface *) -> bool { return false; });

    // Assert
    EXPECT_TRUE(ci->monitorWorkingDBFlag());
    EXPECT_FALSE(ci->m_iface->isValid());   // 无效接口走兜底路径
    delete ci;
}

// 分支：m_iface 为空 -> 兜底返回 true
TEST_F(ControlInterfaceTest, MonitorWorkingDBFlag_NullIface_ReturnsTrue)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    ControlInterface *ci = new ControlInterface();
    ci->m_iface = nullptr;

    // Assert
    EXPECT_TRUE(ci->monitorWorkingDBFlag());
    EXPECT_EQ(nullptr, ci->m_iface);   // 空接口走兜底路径
    delete ci;
}
