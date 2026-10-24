// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QProcess>
#include <QRegularExpression>

#include <dlfcn.h>

#define private public
#include "DBusInterface.h"
#undef private

#define private public
#define protected public
#include "commonfunction.h"

class UT_Common : public UT_HEAD
{
public:
    void SetUp() override
    {
        // 保存静态状态，避免用例间互相干扰
        m_savedSpecialComType = Common::specialComType;
    }
    void TearDown() override
    {
        // 恢复静态状态
        Common::specialComType = m_savedSpecialComType;
    }
    int m_savedSpecialComType = -1;
    Stub stub;   // 追加用例的打桩载体(Stub 析构时自动还原补丁)
};

// ===================== formatTotalCache =====================
TEST_F(UT_Common, UT_Common_formatTotalCache_KiB)
{
    EXPECT_STREQ("120 KiB", Common::formatTotalCache("30 KB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_to_MiB)
{
    EXPECT_STREQ("2 MiB", Common::formatTotalCache("256 KB", 8).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_unit)
{
    // 无空格单位 "KiB" 命中 K 分支
    EXPECT_STREQ("2 KiB", Common::formatTotalCache("1KiB", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_MiB)
{
    // 无空格单位 "MiB" 命中 M 分支
    EXPECT_STREQ("4 MiB", Common::formatTotalCache("1MiB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_MiB_decimal)
{
    EXPECT_STREQ("3 MiB", Common::formatTotalCache("1.5MiB", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_GiB)
{
    EXPECT_STREQ("1 GiB", Common::formatTotalCache("1GiB", 1).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_TiB)
{
    // T 分支：1TiB * 1 = 1024 GiB
    EXPECT_STREQ("1024 GiB", Common::formatTotalCache("1TiB", 1).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_unknownUnit)
{
    // 带空格的 "1 MB" 单位解析为 " MB"(未知) 走 else 分支按原值当 KiB
    EXPECT_STREQ("4 KiB", Common::formatTotalCache("1 MB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_decimal)
{
    EXPECT_STREQ("4.5 KiB", Common::formatTotalCache("1.5 KiB", 3).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_noUnit_bytes)
{
    // 无单位视为字节: 512/1024=0.5 KiB/核 * 2 核 = 1 KiB
    EXPECT_STREQ("1 KiB", Common::formatTotalCache("512", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_empty)
{
    EXPECT_TRUE(Common::formatTotalCache("", 4).isEmpty());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_invalidNumber)
{
    EXPECT_TRUE(Common::formatTotalCache("abc", 4).isEmpty());
}

// ===================== parseSharedCpuCount =====================
TEST_F(UT_Common, UT_Common_parseSharedCpuCount_mixed)
{
    // 0-3(4) + 8(1) + 10-12(3) = 8
    EXPECT_EQ(8, Common::parseSharedCpuCount("0-3,8,10-12"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleRange)
{
    EXPECT_EQ(4, Common::parseSharedCpuCount("0-3"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleNumbers)
{
    EXPECT_EQ(3, Common::parseSharedCpuCount("1,2,3"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_empty)
{
    EXPECT_EQ(0, Common::parseSharedCpuCount(""));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleValue)
{
    EXPECT_EQ(1, Common::parseSharedCpuCount("5"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_withInvalid)
{
    // 0-3(4), abc 无效不计
    EXPECT_EQ(4, Common::parseSharedCpuCount("0-3,abc"));
}

// ===================== specialHString / specialVendorType =====================
TEST_F(UT_Common, UT_Common_specialHString)
{
    // ASCII {72,85,65,87,69,73} => "HUAWEI"
    EXPECT_STREQ("HUAWEI", Common::specialHString().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_specialVendorType_hw)
{
    Common::specialComType = Common::PGUW;
    // 刷新 boardVendorType 缓存，使其反映当前 specialComType
    Common::checkBoardVendorFlag();
    EXPECT_STREQ("HUAWEI", Common::specialVendorType().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_specialVendorType_normal)
{
    Common::specialComType = Common::NormalCom;
    Common::checkBoardVendorFlag();
    EXPECT_STREQ("normalmagical", Common::specialVendorType().toStdString().c_str());
}

// ===================== isHwPlatform =====================
TEST_F(UT_Common, UT_Common_isHwPlatform_unknow)
{
    Common::specialComType = Common::Unknow;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_normal)
{
    Common::specialComType = Common::NormalCom;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_custom)
{
    Common::specialComType = Common::kCustomType;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_special)
{
    Common::specialComType = Common::PGUW;
    EXPECT_TRUE(Common::isHwPlatform());
}

// ===================== checkBoardVendorFlag (specialComType 分支) =====================
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_normal)
{
    Common::specialComType = Common::NormalCom;
    EXPECT_STREQ("", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_pguw)
{
    Common::specialComType = Common::PGUW;
    EXPECT_STREQ("PGUW", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_klvv)
{
    Common::specialComType = Common::KLVV;
    EXPECT_STREQ("KLVV", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_klvu)
{
    Common::specialComType = Common::KLVU;
    EXPECT_STREQ("KLVU", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_pguv)
{
    Common::specialComType = Common::PGUV;
    EXPECT_STREQ("PGUV", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_type5)
{
    Common::specialComType = Common::kSpecialType5;
    EXPECT_STREQ("PGUX", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_custom)
{
    Common::specialComType = Common::kCustomType;
    EXPECT_STREQ("CustomType", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_default)
{
    // 未列出的特殊类型走 default 分支 => PGUW
    Common::specialComType = Common::kSpecialType6;
    EXPECT_STREQ("PGUW", Common::checkBoardVendorFlag().toStdString().c_str());
}

// ===================== tomlFilesName get/set =====================
TEST_F(UT_Common, UT_Common_tomlFilesName)
{
    Common::tomlFilesNameSet("myToml.toml");
    EXPECT_STREQ("myToml.toml", Common::tomlFilesNameGet().toStdString().c_str());
    // 恢复默认值，避免影响其它用例
    Common::tomlFilesNameSet("tomlFilesName");
}

// ===================== getArch / getArchStore =====================
TEST_F(UT_Common, UT_Common_getArch)
{
    QString arch = Common::getArch();
    EXPECT_FALSE(arch.isEmpty());
}

TEST_F(UT_Common, UT_Common_getArchStore)
{
    QString store = Common::getArchStore();
    EXPECT_FALSE(store.isEmpty());
}

// ===================== executeClientCmd =====================
TEST_F(UT_Common, UT_Common_executeClientCmd_success)
{
    QByteArray out = Common::executeClientCmd("/bin/echo", QStringList() << "hello");
    EXPECT_STREQ("hello\n", out.data());
}

TEST_F(UT_Common, UT_Common_executeClientCmd_fail)
{
    // 不存在的命令 => 失败返回空
    QByteArray out = Common::executeClientCmd("nonexistent-cmd-12345", QStringList());
    EXPECT_TRUE(out.isEmpty());
}

// ===================== 追加(Mode 2/t17): checkBoardVendorFlag/boardVendorType/
// executeClientCmd/safeReadSysFsFile/isShowScreenSize 补全分支闭环 =====================
// 分支清单(源码真实分支, commonfunction.cpp L153-214, 复杂度 17):
//   B1  specialComType != -1 -> switch 命中七种类型(NormalCom/PGUW/KLVV/KLVU/
//       PGUV/kSpecialType5/kCustomType/default->PGUW) [既有用例已覆盖]
//   B2  specialComType == -1 -> dmidecode 分支:
//       B2.1 输出含 KLVV/L540 -> KLVV
//       B2.2 输出含 KLVU -> KLVU [既有 normal 用例外的独立键, 与 KLVV 同构]
//       B2.3 输出含 PGUV/W585 -> PGUV [同构]
//       B2.4 输出含 PGUW -> PGUW [同构]
//       B2.5 readDmidecode11_String4 含 PGUX -> PGUX
//       B2.6 键仍为空且 isModeM900()/isModeW525() 命中 -> PGUW
//       B2.7 键仍为空且 M900/W525 均未命中 -> ""
//       B2.8 输出为空 -> getDeviceInfo 兜底(DBus/文件)
//   B3  boardVendorType: initBoardVendorFlag 已初始化 -> 返回缓存键
//   B4  boardVendorType: 未初始化 -> 调用 checkBoardVendorFlag [既有用例首跑已覆盖]
// usecase_count: 追加 11 个用例(文件累计 49)

// ---- 追加用例的打桩辅助: QProcess 受控输出, 不启动真实进程 ----
static QByteArray ut_cf_qprocess_stdout;
static QStringList ut_cf_capturedEnv;
static bool ut_cf_setEnvCalled = false;
static QString ut_cf_capturedWorkPath;
static int ut_cf_exitCodeStub = 0;

class UT_CommonStubHelper
{
public:
    // 桩集: start/waitForFinished/readAllStandardOutput/exitCode/setEnvironment/
    // setWorkingDirectory, 供 checkBoardVendorFlag 未知分支与 executeClientCmd 用例共用
    void install(Stub &stub)
    {
        void (QProcess::*startFunc)(const QString &, const QStringList &, QIODevice::OpenMode) =
            static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start);
        stub.set(startFunc, ut_qprocess_start_empty);

        bool (QProcess::*waitFunc)(int) = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished);
        stub.set(waitFunc, ut_qprocess_waitFinished);

        QByteArray (QProcess::*readFunc)() = static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput);
        stub.set(readFunc, ut_qprocess_readStdout);

        int (QProcess::*exitCodeFunc)() const = static_cast<int (QProcess::*)() const>(&QProcess::exitCode);
        stub.set(exitCodeFunc, ut_qprocess_exitCode);

        void (QProcess::*setEnvFunc)(const QStringList &) = static_cast<void (QProcess::*)(const QStringList &)>(&QProcess::setEnvironment);
        stub.set(setEnvFunc, ut_qprocess_setEnvironment);

        void (QProcess::*setWorkDirFunc)(const QString &) = static_cast<void (QProcess::*)(const QString &)>(&QProcess::setWorkingDirectory);
        stub.set(setWorkDirFunc, ut_qprocess_setWorkingDirectory);
    }

private:
    static void ut_qprocess_start_empty(QProcess *self, const QString &program,
                                        const QStringList &arguments, QIODevice::OpenMode mode)
    {
        Q_UNUSED(self);
        Q_UNUSED(program);
        Q_UNUSED(arguments);
        Q_UNUSED(mode);
    }
    static bool ut_qprocess_waitFinished(QProcess *self, int msecs)
    {
        Q_UNUSED(self);
        Q_UNUSED(msecs);
        return true;
    }
    static QByteArray ut_qprocess_readStdout(QProcess *self)
    {
        Q_UNUSED(self);
        return ut_cf_qprocess_stdout;
    }
    static int ut_qprocess_exitCode(QProcess *self)
    {
        Q_UNUSED(self);
        return ut_cf_exitCodeStub;
    }
    static void ut_qprocess_setEnvironment(QProcess *self, const QStringList &env)
    {
        Q_UNUSED(self);
        ut_cf_capturedEnv = env;
        ut_cf_setEnvCalled = true;
    }
    static void ut_qprocess_setWorkingDirectory(QProcess *self, const QString &dir)
    {
        Q_UNUSED(self);
        ut_cf_capturedWorkPath = dir;
    }
};

// ---- QFile::open / QIODevice::read 桩(safeReadSysFsFile 路径校验与成功路径用) ----
// 注意: QFile::open 与 QIODevice::read 均为虚函数, 成员指针携带的是 vtable 槽位
// 而非代码地址, 不能直接作为 Stub 打桩目标; 这里用 dlsym 取真实函数体地址,
// 虚分发最终落在同一函数体上, 打桩对虚调用同样生效。
static QByteArray ut_cf_fileContent;
static bool ut_cf_fileOpenResult = true;

static void *ut_qt_sym(const char *mangled)
{
    void *addr = dlsym(RTLD_DEFAULT, mangled);
    if (!addr)
        ADD_FAILURE() << "dlsym failed for " << mangled;
    return addr;
}

static bool ut_qfile_open_false(QFile *self, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    return false;
}

static bool ut_qfile_open_true(QFile *self, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    return true;
}

static QByteArray ut_qiodevice_read_canned(QIODevice *self, qint64 maxlen)
{
    Q_UNUSED(self);
    Q_UNUSED(maxlen);
    return ut_cf_fileContent;
}

// 两个桩安装器: 打开失败 / 成功读取
static void ut_stub_qfile_open_fail(Stub &stub)
{
    stub.set(ut_qt_sym("_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE"), ut_qfile_open_false);
}

static void ut_stub_qfile_read_ok(Stub &stub)
{
    stub.set(ut_qt_sym("_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE"), ut_qfile_open_true);
    stub.set(ut_qt_sym("_ZN9QIODevice4readEx"), ut_qiodevice_read_canned);
}

// ---- DBusInterface 桩(getDeviceInfo 兜底路径用, 隔离真实 DBus 连接) ----
static void ut_dbusinterface_init_noop(DBusInterface *self)
{
    Q_UNUSED(self);
}

static bool ut_dbusinterface_getInfo_false(DBusInterface *self, const QString &key, QString &info)
{
    Q_UNUSED(self);
    Q_UNUSED(key);
    info.clear();
    return false;
}

// ---- QFileInfo 桩(safeReadSysFsFile 路径校验用, 不触真实文件系统) ----
static QString ut_cf_canonicalPath;
static bool ut_cf_isFileResult = true;

static QString ut_qfileinfo_canonicalFilePath(QFileInfo *self)
{
    Q_UNUSED(self);
    return ut_cf_canonicalPath;
}

static bool ut_qfileinfo_isFile(QFileInfo *self)
{
    Q_UNUSED(self);
    return ut_cf_isFileResult;
}

// boardVendorType: 缓存已初始化时直接返回缓存键(不再重算)
TEST_F(UT_Common, UT_Common_boardVendorType_initialized_ReturnsCachedKey)
{
    // Arrange: 先调用 checkBoardVendorFlag 建立缓存
    Common::specialComType = Common::PGUW;
    QString expected = Common::checkBoardVendorFlag();
    EXPECT_STREQ("PGUW", expected.toStdString().c_str());

    // Act: 切换 specialComType 不影响已缓存结果
    Common::specialComType = Common::KLVV;
    QString type = Common::boardVendorType();

    // Assert: 命中缓存路径, 与重算值不同
    EXPECT_STREQ("PGUW", type.toStdString().c_str());
}

// checkBoardVendorFlag B2.1: dmidecode 输出含 L540 -> KLVV
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_unknow_L540_ReturnsKLVV)
{
    // Arrange
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_cf_qprocess_stdout = QByteArray("L540-Product-Name");
    Common::specialComType = Common::Unknow;

    // Act
    QString key = Common::checkBoardVendorFlag();

    // Assert
    EXPECT_STREQ("KLVV", key.toStdString().c_str());
}

// checkBoardVendorFlag B2.5: 输出无产品键但 String 4 含 PGUX -> PGUX
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_unknow_String4_ReturnsPGUX)
{
    // Arrange: 输出不含任何产品键, 但 String 4 行含 PGUX
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_cf_qprocess_stdout = QByteArray("model-plain\n String 4: board PGUX\n");
    Common::specialComType = Common::Unknow;

    // Act
    QString key = Common::checkBoardVendorFlag();

    // Assert
    EXPECT_STREQ("PGUX", key.toStdString().c_str());
}

// checkBoardVendorFlag B2.6/B2.7: 输出无键且 M900/W525 均未命中 -> 键保持空
// (源码语义: else 路径不重置 boardVendorKey, 无键时保留旧值, 因此先用
// NormalCom 分支清空粘性缓存, 再进入 unknow 路径验证保留语义)
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_unknow_unknownProduct_ReturnsEmpty)
{
    // Arrange: 先经 NormalCom 分支清空粘性 boardVendorKey
    Common::specialComType = Common::NormalCom;
    Common::checkBoardVendorFlag();

    // 无键输出 + QFile::open 桩 false(/proc/cpuinfo 与设备文件均不读)
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_stub_qfile_open_fail(stub);
    ut_cf_qprocess_stdout = QByteArray("unknown-product-x9");
    Common::specialComType = Common::Unknow;

    // Act
    QString key = Common::checkBoardVendorFlag();

    // Assert: M900(/proc/cpuinfo 无 Hardware)与 W525(dmidecode 无 String 4)均为 false, 键保持空
    EXPECT_TRUE(key.isEmpty());
}

// checkBoardVendorFlag B2.8: 输出为空 -> getDeviceInfo 兜底(DBus 桩生效)后仍无键
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_unknow_emptyOutput_FallsBackToDeviceinfo)
{
    // Arrange: 先经 NormalCom 分支清空粘性 boardVendorKey
    Common::specialComType = Common::NormalCom;
    Common::checkBoardVendorFlag();

    // 输出为空触发 getDeviceInfo; DBusInterface init/getInfo 全桩,
    // 文件兜底由 QFile::open 桩关闭 -> 整条兜底链路不触真实 DBus/文件
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_stub_qfile_open_fail(stub);
    stub.set(ADDR(DBusInterface, init), ut_dbusinterface_init_noop);
    stub.set(ADDR(DBusInterface, getInfo), ut_dbusinterface_getInfo_false);
    ut_cf_qprocess_stdout = QByteArray();
    Common::specialComType = Common::Unknow;

    // Act
    QString key = Common::checkBoardVendorFlag();

    // Assert: 兜底信息同样不含键, 粘性键保持空
    EXPECT_TRUE(key.isEmpty());
}

// executeClientCmd: useEnv=true 时环境被重写为 LANG=en_US.UTF-8/LANGUAGE=en_US
TEST_F(UT_Common, UT_Common_executeClientCmd_useEnvTrue_RewritesLocaleEnv)
{
    // Arrange
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_cf_qprocess_stdout = QByteArray("done\n");
    ut_cf_setEnvCalled = false;
    ut_cf_capturedEnv.clear();
    ut_cf_capturedWorkPath.clear();

    // Act: 附带工作目录参数
    QByteArray out = Common::executeClientCmd("fakecmd", QStringList() << "arg1", QString("/tmp/ut-workdir"));

    // Assert: 环境重写 + 工作目录透传 + 输出返回
    EXPECT_TRUE(ut_cf_setEnvCalled);
    EXPECT_TRUE(ut_cf_capturedEnv.contains(QString("LANG=en_US.UTF-8")));
    EXPECT_TRUE(ut_cf_capturedEnv.contains(QString("LANGUAGE=en_US")));
    int langCount = 0;
    for (const QString &e : ut_cf_capturedEnv) {
        if (e.startsWith("LANG="))
            ++langCount;
    }
    EXPECT_EQ(1, langCount);
    EXPECT_EQ(QString("/tmp/ut-workdir"), ut_cf_capturedWorkPath);
    EXPECT_STREQ("done\n", out.data());
}

// executeClientCmd: 退出码非 0 时返回空(失败分支)
TEST_F(UT_Common, UT_Common_executeClientCmd_nonZeroExit_ReturnsEmpty)
{
    // Arrange
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_cf_qprocess_stdout = QByteArray("partial output");
    ut_cf_exitCodeStub = 1;

    // Act
    QByteArray out = Common::executeClientCmd("fakecmd", QStringList());

    // Assert
    EXPECT_TRUE(out.isEmpty());
    ut_cf_exitCodeStub = 0;
}

// executeClientCmd: useEnv=false 时不设置环境(useEnv 分支)
TEST_F(UT_Common, UT_Common_executeClientCmd_useEnvFalse_KeepsEnvironment)
{
    // Arrange
    UT_CommonStubHelper helper;
    helper.install(stub);
    ut_cf_qprocess_stdout = QByteArray("raw\n");
    ut_cf_setEnvCalled = false;

    // Act
    QByteArray out = Common::executeClientCmd("fakecmd", QStringList(), QString(), 30000, false);

    // Assert: 未调用 setEnvironment, 输出原样返回
    EXPECT_FALSE(ut_cf_setEnvCalled);
    EXPECT_STREQ("raw\n", out.data());
}

// safeReadSysFsFile: baseDir 不在 /sys 下 -> 拒绝并输出错误
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_baseDirOutsideSys_Rejected)
{
    // Arrange
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/etc", "somefile", errOut);

    // Assert
    EXPECT_TRUE(content.isEmpty());
    EXPECT_TRUE(errOut.contains("not under /sys"));
}

// safeReadSysFsFile: baseDir 含 ".." -> 路径穿越拒绝
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_baseDirTraversal_Rejected)
{
    // Arrange
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/sys/../etc", "somefile", errOut);

    // Assert
    EXPECT_TRUE(content.isEmpty());
    EXPECT_TRUE(errOut.contains("Path traversal"));
}

// safeReadSysFsFile: fileName 含路径分隔符/反斜杠/.. -> 全部拒绝
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_fileNameTraversal_Rejected)
{
    // Arrange
    QString err1, err2, err3;

    // Act
    QString c1 = Common::safeReadSysFsFile("/sys/class", "a/b", err1);
    QString c2 = Common::safeReadSysFsFile("/sys/class", "..", err2);
    QString c3 = Common::safeReadSysFsFile("/sys/class", "a\\b", err3);

    // Assert: 三种非法文件名均拒绝且错误信息可辨识
    EXPECT_TRUE(c1.isEmpty());
    EXPECT_TRUE(err1.contains("Path traversal"));
    EXPECT_TRUE(c2.isEmpty());
    EXPECT_TRUE(err2.contains("Path traversal"));
    EXPECT_TRUE(c3.isEmpty());
    EXPECT_TRUE(err3.contains("Path traversal"));
}

// safeReadSysFsFile: 规范化路径逃逸出 /sys -> 拒绝(QFileInfo 桩控制)
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_canonicalEscape_Rejected)
{
    // Arrange: 桩返回逃逸路径
    Stub stub;
    stub.set(ADDR(QFileInfo, canonicalFilePath), ut_qfileinfo_canonicalFilePath);
    ut_cf_canonicalPath = QString("/etc/passwd");
    ut_cf_isFileResult = true;
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/sys/class", "link", errOut);

    // Assert
    EXPECT_TRUE(content.isEmpty());
    EXPECT_TRUE(errOut.contains("Path escape"));
}

// safeReadSysFsFile: 目标不是常规文件 -> 拒绝
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_notRegularFile_Rejected)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(QFileInfo, canonicalFilePath), ut_qfileinfo_canonicalFilePath);
    ut_cf_canonicalPath = QString("/sys/class/subdir");
    ut_cf_isFileResult = false;
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/sys/class", "subdir", errOut);

    // Assert
    EXPECT_TRUE(content.isEmpty());
    EXPECT_TRUE(errOut.contains("Not a regular file"));
}

// safeReadSysFsFile: 打开失败 -> 空内容与错误信息
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_openFailed_ReturnsEmptyWithError)
{
    // Arrange: QFileInfo/QFile 全桩, open 返回 false
    Stub stub;
    stub.set(ADDR(QFileInfo, canonicalFilePath), ut_qfileinfo_canonicalFilePath);
    stub.set(ADDR(QFileInfo, isFile), ut_qfileinfo_isFile);
    ut_stub_qfile_open_fail(stub);
    ut_cf_canonicalPath = QString("/sys/class/fakefile");
    ut_cf_isFileResult = true;
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/sys/class", "fakefile", errOut);

    // Assert
    EXPECT_TRUE(content.isEmpty());
    EXPECT_TRUE(errOut.contains("Open failed"));
}

// safeReadSysFsFile: 合法 sysfs 文件成功读取并做 trim, 截断到 64 字节
TEST_F(UT_Common, UT_Common_safeReadSysFsFile_validFile_ReturnsTrimmedContent)
{
    // Arrange: QFileInfo/QFile::open/QIODevice::read 全桩, 注入带空白内容
    Stub stub;
    stub.set(ADDR(QFileInfo, canonicalFilePath), ut_qfileinfo_canonicalFilePath);
    stub.set(ADDR(QFileInfo, isFile), ut_qfileinfo_isFile);
    ut_stub_qfile_read_ok(stub);
    ut_cf_canonicalPath = QString("/sys/class/fakefile");
    ut_cf_isFileResult = true;
    ut_cf_fileContent = QByteArray("  Fake Vendor Value \n");
    QString errOut;

    // Act
    QString content = Common::safeReadSysFsFile("/sys/class", "fakefile", errOut);

    // Assert
    EXPECT_STREQ("Fake Vendor Value", content.toStdString().c_str());
    EXPECT_TRUE(errOut.isEmpty());
}

// isShowScreenSize: 未接入 DConfig 时默认显示(true 分支)
TEST_F(UT_Common, UT_Common_isShowScreenSize_default_ReturnsTrue)
{
    // Arrange: 无 DConfig 环境(打桩后无配置覆写)
    // Act: 读取默认显示开关
    // Assert: 未配置时默认显示
    EXPECT_TRUE(Common::isShowScreenSize());
}
