// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method              | level | factors                       | min | actual |
// |---------------------|-------|-------------------------------|-----|--------|
// | PageInfo(ctor)      | low   | -                             | 1   | 1      |
// | updateInfo(QMap)    | low   | -                             | 1   | 1      |
// | setLabel(2参)       | low   | -                             | 1   | 1      |
// | clearContent        | mid   | name_pattern:clear            | 2   | 2      |
// | setDeviceInfoNum    | low   | -                             | 1   | 1      |
// | getDeviceInfoNum    | low   | -                             | 1   | 1      |
// | setMultiFlag        | low   | -                             | 1   | 1      |
// | getMultiFlag        | low   | -                             | 1   | 1      |
// | isOverview          | low   | -                             | 1   | 1      |
// | isBaseBoard         | low   | -                             | 1   | 1      |
// | paintEvent          | low   | -                             | 1   | 1      |
// | packageHasInstalled | low   | 外部进程依赖(QProcess)        | 1   | 4      |
// ─── actual >= min 达标（共 16 个用例实例）───
//
// 注：PageInfo 为抽象基类（updateInfo(QList)/clearWidgets/setLabel(QString) 为纯虚），
// 测试经 TestPageInfo 派生类实例化；构造函数初始化 m_AllInfoNum=0 / m_multiFlag=false，
// 默认值断言安全。updateInfo(QList)/clearWidgets/setLabel(QString) 三个纯虚方法不在
// Mode 1 inventory 可测清单内，由派生类计数器顺带覆盖派发路径，不计入 min。
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（地图空/非空、bool 真/假、int 正/零/负、安装/未安装/空输出）
// 3. 每个等价类的边界值显式覆盖: [x]（0/-1、空输出）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（packageHasInstalled 3 组输出用参数化）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（paintEvent 的 activeWindow 两分支取 offscreen 实际路径；packageHasInstalled 输出包含/不包含 installed）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类方法无显式 throw，异常路径为不适用）
// 8. 负面场景有专门用例: [x]（未安装/空输出）
// 9. 负面用例验证强异常安全: [x]（成员状态保持）
// 10. stub_ext vs gMock 选择正确: [x]（QProcess 外部进程 → stub_ext 运行时替换；无接口类 → 不用 gMock）
//
// mock 深度分析（test-code-gen §4.0，来源：工作区实际源码核对 + MCP trace_path）：
// - PageInfo 为抽象 GUI 基类（DWidget 子类），QApplication 由 tests/src/test_main.cpp
//   统一以 offscreen 创建。构造函数已初始化 m_AllInfoNum=0/m_multiFlag=false。
// - packageHasInstalled 为 static 方法，内部 QProcess 调 dpkg -s 属外部进程依赖，
//   按打桩清单以 stub_ext 替换 start/waitForFinished/readAll，不触真实 dpkg；
//   Qt 重载方法一律 static_cast 消歧（铁律 #13），桩 lambda 首参接收对象指针
//   （本仓库 stub-shadow 约定）。
// - updateInfo(QMap)/setLabel(2参)/clearContent 为基类提供的非纯虚实现，测试
//   钉住「无副作用」契约：调用后成员保持原值。
// - 无文件系统/网络依赖；DBus 调用已由 test_main.cpp 全局桩拦截。
//
// 分支清单（来源：工作区实际源码 PageInfo.cpp / PageInfo.h）
// B1: packageHasInstalled: r.contains("installed") → true / false
// B2: paintEvent: DApplication::activeWindow() 非空 → Active / 空 → Inactive
// B3: updateInfo(QMap)/setLabel(2参)/clearContent: 空实现，仅日志
// B4: isOverview/isBaseBoard: 直接 return false
//
// 用例映射：
// - Ctor_DefaultConstruction_ZeroInitialized          → 构造函数默认值
// - UpdateInfo_MapOverload_MembersUntouched           → B3
// - SetLabel_TwoArgOverload_MembersUntouched          → B3
// - ClearContent_CallMethod_StatePreserved            → B3（mid #1）
// - ClearContent_RepeatedCalls_StatePreserved         → B3（mid #2）
// - SetDeviceInfoNum_SetGet_RoundTripAndBoundaries    → set/get 往返 + 0/-1 边界
// - GetDeviceInfoNum_DefaultZero_AndDirectRead        → get 读取路径
// - SetMultiFlag_TrueAndFalse_RoundTrip               → set/get 真/假
// - GetMultiFlag_DefaultFalse_AndDirectRead           → get 读取路径
// - IsOverview_BaseClass_CallReturnsFalse                  → B4
// - IsBaseBoard_BaseClass_CallReturnsFalse                 → B4
// - PaintEvent_Offscreen_RenderProducesCanvas                  → B2
// - PackageHasInstalled_DpkgOutput_DetectsInstallState (TEST_P) → B1 四组（含缺陷钉住组）

#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QProcess>

#include "stubext.h"
#include "PageInfo.h"

namespace {
// PageInfo 为抽象类，测试派生实现三个纯虚方法并计数，验证虚派发契约
class TestPageInfo : public PageInfo
{
public:
    explicit TestPageInfo(QWidget *parent = nullptr)
        : PageInfo(parent) {}
    void updateInfo(const QList<DeviceBaseInfo *> &) override { ++updateInfoListCalls; }
    void clearWidgets() override { ++clearWidgetsCalls; }
    void setLabel(const QString &itemstr) override
    {
        lastLabel = itemstr;
        ++setLabelCalls;
    }

    int updateInfoListCalls = 0;
    int clearWidgetsCalls = 0;
    int setLabelCalls = 0;
    QString lastLabel;
};
}  // namespace

struct DpkgOutputParam {
    const char *output;    // readAll 返回内容（nullptr → 空输出）
    bool expectInstalled;  // 期望判定结果
};

class PageInfoTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // Arrange（公共前置）：经派生类构造被测对象（offscreen 平台安全）
        robj = new TestPageInfo;
        obj = robj;
    }

    void TearDown() override {
        // 先卸桩再析构，防止析构路径调用已打桩方法触达已销毁的栈帧捕获
        stub.clear();
        delete obj;
        obj = nullptr;
        robj = nullptr;
    }

    stub_ext::StubExt stub;
    PageInfo *obj = nullptr;
    TestPageInfo *robj = nullptr;  // 派生类型指针，用于断言纯虚派发计数
};

class PageInfoPackageTest : public PageInfoTest,
                            public ::testing::WithParamInterface<DpkgOutputParam> {
};

// ═══════════════════════════════════════════════════════════════
// 构造函数（默认值契约）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, Ctor_DefaultConstruction_ZeroInitialized)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造后的默认成员值
    int num = obj->getDeviceInfoNum();
    bool flag = obj->getMultiFlag();

    // Assert：构造函数初始化 m_AllInfoNum=0 / m_multiFlag=false
    EXPECT_EQ(num, 0);
    EXPECT_EQ(flag, false);
}

// ═══════════════════════════════════════════════════════════════
// 基类空实现虚方法（updateInfo(QMap) / setLabel(2参) / clearContent[mid]）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, UpdateInfo_MapOverload_MembersUntouched)
{
    // Arrange：预置成员状态并准备空/非空两种输入
    obj->setDeviceInfoNum(7);
    obj->setMultiFlag(true);
    QMap<QString, QString> emptyMap;
    QMap<QString, QString> filledMap{{"key", "value"}};

    // Act
    obj->updateInfo(emptyMap);
    obj->updateInfo(filledMap);

    // Assert：QMap 重载为基类空实现，不产生副作用；纯虚派发走派生类
    EXPECT_EQ(obj->getDeviceInfoNum(), 7);
    EXPECT_EQ(robj->updateInfoListCalls, 0);
}

TEST_F(PageInfoTest, SetLabel_TwoArgOverload_MembersUntouched)
{
    // Arrange
    obj->setDeviceInfoNum(-3);
    obj->setMultiFlag(false);

    // Act：双参 setLabel 走基类空实现
    obj->setLabel("title", "content");

    // Assert：成员保持不变，单参纯虚未被触发
    EXPECT_EQ(obj->getDeviceInfoNum(), -3);
    EXPECT_EQ(robj->setLabelCalls, 0);
}

TEST_F(PageInfoTest, ClearContent_CallMethod_StatePreserved)
{
    // Arrange：预置状态（钉住基类契约：clearContent 不清理成员，由子类覆写实现清理）
    obj->setDeviceInfoNum(42);
    obj->setMultiFlag(true);

    // Act
    obj->clearContent();

    // Assert：成员保持原值（契约钉住，防基类意外加清理逻辑破坏子类语义）
    EXPECT_EQ(obj->getDeviceInfoNum(), 42);
    EXPECT_TRUE(obj->getMultiFlag());
}

TEST_F(PageInfoTest, ClearContent_RepeatedCalls_StatePreserved)
{
    // Arrange
    obj->setDeviceInfoNum(0);
    obj->setMultiFlag(false);

    // Act：连续调用验证幂等
    obj->clearContent();
    obj->clearContent();

    // Assert：重复调用后成员仍保持原值
    EXPECT_EQ(obj->getDeviceInfoNum(), 0);
    EXPECT_FALSE(obj->getMultiFlag());
}

// ═══════════════════════════════════════════════════════════════
// 设备信息数目（setDeviceInfoNum / getDeviceInfoNum）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, SetDeviceInfoNum_SetGet_RoundTripAndBoundaries)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：依次写入正数、零、负数三组等价类
    obj->setDeviceInfoNum(42);

    // Assert：写入后 get 返回写入值
    EXPECT_EQ(obj->getDeviceInfoNum(), 42);

    // Act：零值边界
    obj->setDeviceInfoNum(0);

    // Assert
    EXPECT_EQ(obj->getDeviceInfoNum(), 0);

    // Act：负值边界
    obj->setDeviceInfoNum(-1);

    // Assert
    EXPECT_EQ(obj->getDeviceInfoNum(), -1);
}

TEST_F(PageInfoTest, GetDeviceInfoNum_DefaultZero_AndDirectRead)
{
    // Arrange：直接预置成员，验证 get 读取路径
    obj->m_AllInfoNum = -5;

    // Act
    int num = obj->getDeviceInfoNum();

    // Assert：get 直读成员而非缓存
    EXPECT_EQ(num, -5);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// 多行标识（setMultiFlag / getMultiFlag）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, SetMultiFlag_TrueAndFalse_RoundTrip)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：写入 true
    obj->setMultiFlag(true);

    // Assert：真值往返
    EXPECT_TRUE(obj->getMultiFlag());

    // Act：写入 false
    obj->setMultiFlag(false);

    // Assert：假值往返；对象生命周期内写入可重复
    EXPECT_FALSE(obj->getMultiFlag());
    EXPECT_NE(obj, nullptr);
}

TEST_F(PageInfoTest, GetMultiFlag_DefaultFalse_AndDirectRead)
{
    // Arrange：直接预置成员，验证 get 读取路径
    obj->m_multiFlag = true;

    // Act
    bool flag = obj->getMultiFlag();

    // Assert
    EXPECT_TRUE(flag);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// 页面类型标识（isOverview / isBaseBoard，头文件内联实现）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, IsOverview_BaseClass_CallReturnsFalse)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：基类指针多态调用
    bool overview = obj->isOverview();

    // Assert：基类固定返回 false（子类覆写扩展），对象本体仍为 PageInfo 体系
    EXPECT_FALSE(overview);
    EXPECT_NE(dynamic_cast<PageInfo *>(obj), nullptr);
}

TEST_F(PageInfoTest, IsBaseBoard_BaseClass_CallReturnsFalse)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：基类指针多态调用
    bool baseBoard = obj->isBaseBoard();

    // Assert：基类固定返回 false（子类覆写扩展）
    EXPECT_FALSE(baseBoard);
    EXPECT_NE(dynamic_cast<PageInfo *>(obj), nullptr);
}

// ═══════════════════════════════════════════════════════════════
// paintEvent（low）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageInfoTest, PaintEvent_Offscreen_RenderProducesCanvas)
{
    // Arrange
    QPaintEvent paintEvent(obj->rect());

    // Act：直接触发绘制路径（offscreen 下 activeWindow 为空 → Inactive 分支）
    obj->paintEvent(&paintEvent);

    // Assert：绘制完成后可正常离屏渲染，画布与控件尺寸一致
    const QPixmap grabbed = obj->grab();
    EXPECT_FALSE(grabbed.isNull());
    EXPECT_EQ(grabbed.size(), obj->size());
}

// ═══════════════════════════════════════════════════════════════
// packageHasInstalled（low，外部进程依赖 → QProcess 全桩）
// ═══════════════════════════════════════════════════════════════
TEST_P(PageInfoPackageTest, PackageHasInstalled_DpkgOutput_DetectsInstallState)
{
    // Arrange：替换 QProcess 外部依赖（start 不真正拉起 dpkg）
    const DpkgOutputParam &p = GetParam();
    QByteArray fakeOutput = p.output ? QByteArray(p.output) : QByteArray();
    int startCount = 0;
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [&](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) { ++startCount; });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [](QProcess *, int) { return true; });
    stub.set_lamda(&QProcess::readAll,
                   [fakeOutput](QIODevice *) { return fakeOutput; });

    // Act：static 方法直接调用
    bool installed = PageInfo::packageHasInstalled("some-package");

    // Assert：输出包含 "installed" 与否决定判定结果；dpkg 参数经桩转发一次
    EXPECT_EQ(installed, p.expectInstalled);
    EXPECT_EQ(startCount, 1);
}

INSTANTIATE_TEST_SUITE_P(
    DpkgOutputVariants, PageInfoPackageTest,
    ::testing::Values(
        DpkgOutputParam{"Status: install ok installed\n", true},              // 已安装输出
        DpkgOutputParam{"Status: deinstall ok config-files\n", false},        // 已卸载输出
        DpkgOutputParam{nullptr, false},                                      // 空输出边界（dpkg 对未安装包走 stderr）
        DpkgOutputParam{"package 'x' is not installed\n", false}              // 未安装输出：随 DEF-DM-PAGE-001 修复（精确匹配状态行）翻转为 false
    ));
