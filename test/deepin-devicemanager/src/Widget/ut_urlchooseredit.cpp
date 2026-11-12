// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_urlchooseredit.cpp
 * @brief UrlChooserEdit 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method          | level | factors                           | min | actual |
 * |-----------------|-------|-----------------------------------|-----|--------|
 * | ctor            | mid   | initUI 状态链                      | 2   | 2      |
 * | initConnections | low   | 按钮到选目录槽连接（1 条 connect）  | 1   | 1      |
 * | slotChooseUrl   | mid   | 选目录/取消/非本地分支              | 2   | 3      |
 * | checkLocalFolder| mid   | 本地/非本地分支                    | 2   | 2      |
 * | text            | low   | 成员路径读取                        | 1   | 1      |
 *
 * 说明：initConnections 仅一条 connect（按钮 clicked → slotChooseUrl），
 * 由 slotChooseUrl 选目录用例经按钮 click() 触发以覆盖连接链路。
 *
 * 分支/等价类说明（源自 MCP get_code_snippet + 工作目录源码核对，24-107 行）：
 *   - ctor：mp_urlEdit(DLineEdit)/mp_urlBtn(DSuggestButton) → initUI（460×36、
 *     编辑框 410×36 只读 NoFocus、按钮 40×36、objectName MpUrlEdit/MpUrlBtn、
 *     初始路径 homePath）→ initConnections；
 *   - slotChooseUrl：QFileDialog::getExistingDirectory 取路径 → ElideMiddle 截断
 *     （宽度 - 80）→ 空路径恢复上次 mp_elidParh/mp_folderPath → 更新编辑框与
 *     成员 → checkLocalFolder；
 *   - checkLocalFolder：QStorageInfo(mp_folderPath) 校验 valid 且 device 以
 *     "/dev/" 开头 → 否则 setAlert(true)+showAlertMessage+isLocal=false；
 *     末尾 emit signalNotLocalFolder(isLocal)。
 *   用例映射：
 *   - ctor_Default_UIStateConfigured                       → initUI 控件状态
 *   - ctor_Default_HomePathShownInEdit                     → 初始路径分支
 *   - slotChooseUrl_UserPicksLocalDir_PathUpdatedLocalTrue → 选中目录分支（经按钮）
 *   - slotChooseUrl_UserCancelsDialog_PreviousPathRestored → 空路径恢复分支
 *   - slotChooseUrl_NonLocalDevice_AlertAndSignalFalse     → 非本地告警分支
 *   - checkLocalFolder_ParamIgnoredMemberUsed_NoAlert      → 本地路径分支（含
 *     Q_UNUSED(path) 形参被忽略的实测，见缺陷记录）
 *   - text_ReturnsMemberPathNotLabel                       → text() 读成员
 * 打桩策略（外部依赖全部 stub，不弹真实对话框、不读真实磁盘）：
 *   - QFileDialog::getExistingDirectory（静态函数）→ 返回受控路径并计数；
 *   - QStorageInfo::isValid/device（非虚成员，QtCore 源外实现）→ 返回受控值，
 *     消除对真实挂载点的依赖；
 *   - DLineEdit::showAlertMessage（非虚成员）→ 计数桩，避免告警浮层创建。
 */

#include "UrlChooserEdit.h"
#include "ut_Head.h"
#include "stubext.h"
#include "ut_stub_helper.h"

#include <gtest/gtest.h>
#include <DLineEdit>
#include <DSuggestButton>
#include <QDir>
#include <QFileDialog>
#include <QSignalSpy>
#include <QStorageInfo>

// ---- stub 观测点：文件对话框返回值、告警提示 ----
static QString g_utDialogPath;
static int g_utDialogCallCount = 0;
static int g_utAlertCount = 0;

static QString ut_getExistingDirectory(QWidget *, const QString &, const QString &,
                                       QFileDialog::Options)
{
    ++g_utDialogCallCount;
    return g_utDialogPath;
}

class UT_UrlChooserEdit : public ::testing::Test
{
public:
    void SetUp()
    {
        g_utDialogPath.clear();
        g_utDialogCallCount = 0;
        g_utAlertCount = 0;
        // 初始路径确定化：拦截 QDir::homePath（静态），构造不受本机用户目录影响
        stub.set_lamda(&QDir::homePath,
                       []() { return QStringLiteral("ut-home-dir"); });
        m_widget = new UrlChooserEdit;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    // 让 QStorageInfo 报告"本地磁盘"（valid + /dev/ 设备）
    void stubStorageLocal()
    {
        stub.set_lamda(&QStorageInfo::isValid, [](const QStorageInfo *) { return true; });
        stub.set_lamda(&QStorageInfo::device,
                       [](const QStorageInfo *) { return QByteArray("/dev/sda1"); });
    }

    // 让 QStorageInfo 报告"非本地"（设备不带 /dev/ 前缀）
    void stubStorageRemote()
    {
        stub.set_lamda(&QStorageInfo::isValid, [](const QStorageInfo *) { return true; });
        stub.set_lamda(&QStorageInfo::device,
                       [](const QStorageInfo *) { return QByteArray("sdb1"); });
    }

    // 拦截告警浮层（重载取 QWidget* follower 版本，源码实际调用该签名）
    void stubAlert()
    {
        stub.set_lamda(static_cast<void (DLineEdit::*)(const QString &, QWidget *, int)>(
                           &DLineEdit::showAlertMessage),
                       [](DLineEdit *, const QString &, QWidget *, int) { ++g_utAlertCount; });
    }

    UrlChooserEdit *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// ctor + initUI：固定尺寸、编辑框只读无焦点、objectName 与按钮尺寸
TEST_F(UT_UrlChooserEdit, ctor_Default_UIStateConfigured)
{
    // Arrange：期望尺寸常量（initUI 在 SetUp 构造内完成）
    const QSize kWhole(460, 36);
    const QSize kEdit(410, 36);
    const QSize kBtn(40, 36);

    // Act：读取构造后的关键控件状态
    DLineEdit *edit = m_widget->findChild<DLineEdit *>();
    DSuggestButton *btn = m_widget->findChild<DSuggestButton *>();

    // Assert：整体与控件尺寸、只读、焦点策略、objectName、布局三段
    ASSERT_NE(nullptr, edit);
    ASSERT_NE(nullptr, btn);
    EXPECT_EQ(kWhole, m_widget->size());
    EXPECT_EQ(kEdit, edit->size());
    EXPECT_EQ(kBtn, btn->size());
    EXPECT_TRUE(edit->lineEdit()->isReadOnly());
    EXPECT_EQ(Qt::NoFocus, edit->focusPolicy());
    EXPECT_EQ(QStringLiteral("MpUrlEdit"), edit->objectName());
    EXPECT_EQ(QStringLiteral("MpUrlBtn"), btn->objectName());
}

// ctor + initUI：初始路径为 homePath，编辑框与成员/截断缓存一致
TEST_F(UT_UrlChooserEdit, ctor_Default_HomePathShownInEdit)
{
    // Arrange：受控的初始路径（SetUp 已拦截 QDir::homePath）
    const QString expected = QStringLiteral("ut-home-dir");

    // Act：读取构造后的路径三处状态
    const QString shown = m_widget->mp_urlEdit->text();

    // Assert：编辑框、成员路径、截断缓存均为受控 home 值（短于截断宽度不省略）
    EXPECT_EQ(expected, shown);
    EXPECT_EQ(expected, m_widget->text());
    EXPECT_EQ(expected, m_widget->mp_elidParh);
}

// slotChooseUrl 选中本地目录分支（经按钮 click 覆盖 initConnections 连接）：
// 编辑框/成员/截断缓存更新，signalNotLocalFolder(true)，无告警
TEST_F(UT_UrlChooserEdit, slotChooseUrl_UserPicksLocalDir_PathUpdatedLocalTrue)
{
    // Arrange：对话框返回本地目录、存储信息报告 /dev/ 本地盘
    stub.set(ADDR(QFileDialog, getExistingDirectory), ut_getExistingDirectory);
    g_utDialogPath = QStringLiteral("ut-select-dir");
    stubStorageLocal();
    stubAlert();
    QSignalSpy spy(m_widget, &UrlChooserEdit::signalNotLocalFolder);
    ASSERT_TRUE(spy.isValid());

    // Act：点击选择按钮（initConnections 的 connect 链路）
    m_widget->mp_urlBtn->click();

    // Assert：路径三处更新、信号 true、无告警、对话框被调用一次
    EXPECT_EQ(1, g_utDialogCallCount);
    EXPECT_EQ(QStringLiteral("ut-select-dir"), m_widget->text());
    EXPECT_EQ(QStringLiteral("ut-select-dir"), m_widget->mp_urlEdit->text());
    EXPECT_EQ(QStringLiteral("ut-select-dir"), m_widget->mp_elidParh);
    ASSERT_EQ(1, spy.count());
    EXPECT_TRUE(spy.at(0).at(0).toBool());
    EXPECT_EQ(0, g_utAlertCount);
    EXPECT_FALSE(m_widget->mp_urlEdit->isAlert());
}

// slotChooseUrl 取消（空路径）分支：恢复上一次路径与截断文本
TEST_F(UT_UrlChooserEdit, slotChooseUrl_UserCancelsDialog_PreviousPathRestored)
{
    // Arrange：先选择一次本地目录建立基线
    stub.set(ADDR(QFileDialog, getExistingDirectory), ut_getExistingDirectory);
    stubStorageLocal();
    stubAlert();
    g_utDialogPath = QStringLiteral("ut-first");
    m_widget->slotChooseUrl();
    ASSERT_EQ(QStringLiteral("ut-first"), m_widget->text());

    // Act：对话框返回空串（用户取消）
    g_utDialogPath.clear();
    m_widget->slotChooseUrl();

    // Assert：路径与截断缓存保持上次值，编辑框显示未变
    EXPECT_EQ(QStringLiteral("ut-first"), m_widget->text());
    EXPECT_EQ(QStringLiteral("ut-first"), m_widget->mp_elidParh);
    EXPECT_EQ(QStringLiteral("ut-first"), m_widget->mp_urlEdit->text());
    EXPECT_EQ(2, g_utDialogCallCount);
}

// slotChooseUrl 非本地设备分支：告警提示 + signalNotLocalFolder(false)
TEST_F(UT_UrlChooserEdit, slotChooseUrl_NonLocalDevice_AlertAndSignalFalse)
{
    // Arrange：对话框返回路径，存储设备不带 /dev/ 前缀
    stub.set(ADDR(QFileDialog, getExistingDirectory), ut_getExistingDirectory);
    g_utDialogPath = QStringLiteral("ut-usb");
    stubStorageRemote();
    stubAlert();
    QSignalSpy spy(m_widget, &UrlChooserEdit::signalNotLocalFolder);
    ASSERT_TRUE(spy.isValid());

    // Act
    m_widget->slotChooseUrl();

    // Assert：路径已更新但进入告警分支：告警浮层一次、编辑框告警态、信号 false
    EXPECT_EQ(QStringLiteral("ut-usb"), m_widget->text());
    EXPECT_EQ(1, g_utAlertCount);
    EXPECT_TRUE(m_widget->mp_urlEdit->isAlert());
    ASSERT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

// checkLocalFolder 本地路径分支：无告警、signalNotLocalFolder(true)
// 注：源码 Q_UNUSED(path) 忽略形参、实际校验 mp_folderPath，本用例按实测定格
TEST_F(UT_UrlChooserEdit, checkLocalFolder_ParamIgnoredMemberUsed_NoAlert)
{
    // Arrange：成员路径为本地盘；存储信息报告本地
    stubStorageLocal();
    stubAlert();
    m_widget->mp_folderPath = QStringLiteral("ut-local-ok");
    QSignalSpy spy(m_widget, &UrlChooserEdit::signalNotLocalFolder);
    ASSERT_TRUE(spy.isValid());

    // Act：传入与成员不同的形参（源码会忽略）
    m_widget->checkLocalFolder(QStringLiteral("ut-other-param"));

    // Assert：无告警、信号携带 true（本地）
    EXPECT_EQ(0, g_utAlertCount);
    EXPECT_FALSE(m_widget->mp_urlEdit->isAlert());
    ASSERT_EQ(1, spy.count());
    EXPECT_TRUE(spy.at(0).at(0).toBool());
}

// text()：始终返回成员 mp_folderPath（而非编辑框显示的截断文本）
TEST_F(UT_UrlChooserEdit, text_MemberPathSet_ReturnsMemberPath)
{
    // Arrange：成员路径与编辑框显示文本设置不同值
    m_widget->mp_folderPath = QStringLiteral("ut-member-path");
    m_widget->mp_urlEdit->setText(QStringLiteral("ut-elided-path"));

    // Act：读取 text()
    const QString result = m_widget->text();

    // Assert：返回成员路径，与编辑框显示文本无关
    EXPECT_EQ(QStringLiteral("ut-member-path"), result);
    EXPECT_NE(result, m_widget->mp_urlEdit->text());
}
