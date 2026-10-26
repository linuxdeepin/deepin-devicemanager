// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_getdrivernamemodel.cpp
 * @brief GetDriverNameModel 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method              | level | factors                          | min | actual |
 * |---------------------|-------|----------------------------------|-----|--------|
 * | ctor                | low   | default                          | 1   | 1      |
 * | stopLoadingDrivers  | low   | set-stop-flag                    | 1   | 1      |
 * | traverseFolders     | low   | missing,flat,subdir,recursion,stop | 1 | 5      |
 * | startLoadDrivers    | low   | no-sub,dbus-ok,dbus-fallback,stop | 1  | 4      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet 真实源码，GetDriverNameModel.cpp）：
 *   - traverseFolders：m_Stop 置位/目录不存在/收集 *.deb *.ko（NoSymLinks）/
 *     recursion 才进入子目录；
 *   - startLoadDrivers：先复位 m_Stop；includeSub=true 先走 DBus
 *     （searchDriver 成功用 DBus 结果，失败回退递归遍历），false 直接遍历；
 *     填充循环逐行建 item（图标 DecorationRole、路径 UserRole、tooltip 文件名），
 *     循环内检查 m_Stop，最后 emit finishLoadDrivers；
 *   - stopLoadingDrivers 仅置 m_Stop。
 * 打桩策略（外部依赖全部 stub，不依赖真实设备环境）：
 *   - DBusAnythingInterface::searchDriver（非虚成员函数）→ 控制成功/失败与返回
 *     路径集，不依赖 com.deepin.anything 真实服务；
 *   - 目录数据用 QTemporaryDir + 真实 .deb/.ko 空文件，进程退出自动清理。
 */

#include "GetDriverNameModel.h"
#include "DBusAnythingInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStackedWidget>
#include <DLabel>

#include <QDir>
#include <QFileIconProvider>
#include <QTemporaryDir>
#include <QSignalSpy>

#include <gtest/gtest.h>

// ---- stub：DBusAnythingInterface::searchDriver（外部 DBus 依赖打桩） ----
static bool g_utSearchOk = false;
static QStringList g_utSearchResult;
static GetDriverNameModel *g_utStopTarget = nullptr;
static bool ut_searchDriver(void *self, const QString &path, QStringList &out)
{
    Q_UNUSED(self);
    Q_UNUSED(path);
    if (g_utSearchOk)
        out = g_utSearchResult;
    return g_utSearchOk;
}

// searchDriver 成功返回的同时置停止标志，模拟填充循环启动前收到停止请求
static bool ut_searchDriverStop(void *self, const QString &path, QStringList &out)
{
    Q_UNUSED(self);
    Q_UNUSED(path);
    out = g_utSearchResult;
    if (g_utStopTarget)
        g_utStopTarget->m_Stop = true;
    return true;
}

class UT_GetDriverNameModel : public UT_HEAD
{
public:
    void SetUp()
    {
        g_utSearchOk = false;
        g_utSearchResult.clear();
        g_utStopTarget = nullptr;
        m_GetDriverNameModel = new GetDriverNameModel;
    }
    void TearDown()
    {
        delete m_GetDriverNameModel;
    }

    // 构造临时目录：dir 下 rootCount 个 .deb/.ko 混合文件与 decoyCount 个干扰文件，
    // recursion 时额外创建 subdir 并放入 subCount 个 .deb 文件
    QTemporaryDir *mkDir(int rootCount, int decoyCount, int subCount = 0)
    {
        QTemporaryDir *dir = new QTemporaryDir;
        dir->setAutoRemove(true);
        for (int i = 0; i < rootCount; ++i) {
            const QString suffix = (i % 2 == 0) ? QStringLiteral(".deb") : QStringLiteral(".ko");
            QFile f(dir->filePath(QStringLiteral("drv%1%2").arg(i).arg(suffix)));
            f.open(QIODevice::WriteOnly);
        }
        for (int i = 0; i < decoyCount; ++i) {
            QFile f(dir->filePath(QStringLiteral("note%1.txt").arg(i)));
            f.open(QIODevice::WriteOnly);
        }
        if (subCount > 0) {
            QDir(dir->path()).mkdir(QStringLiteral("sub"));
            for (int i = 0; i < subCount; ++i) {
                QFile f(dir->filePath(QStringLiteral("sub/nested%1.deb").arg(i)));
                f.open(QIODevice::WriteOnly);
            }
        }
        return dir;
    }

    GetDriverNameModel *m_GetDriverNameModel;
};

// 构造：初始状态为空列表且未置停止标志
TEST_F(UT_GetDriverNameModel, GetDriverNameModel_DefaultState_ListsEmptyStopFalse)
{
    // Arrange：fixture 已构造模型（空列表、m_Stop 默认 false）
    ASSERT_NE(nullptr, m_GetDriverNameModel);
    m_GetDriverNameModel->setObjectName(QStringLiteral("dvm-ut-model"));

    // Act：读取构造后的默认状态
    const int pathCount = m_GetDriverNameModel->mp_driverPathList.size();
    const int nameCount = m_GetDriverNameModel->mp_driversList.size();
    const bool stopFlag = m_GetDriverNameModel->m_Stop;

    // Assert
    EXPECT_EQ(0, pathCount);
    EXPECT_EQ(0, nameCount);
    EXPECT_EQ(false, stopFlag);
}

// stopLoadingDrivers：调用后置位停止标志
TEST_F(UT_GetDriverNameModel, stopLoadingDrivers_Invoke_SetsStopFlagTrue)
{
    // Arrange：初始未停止，记录停止前标志与信号探针环境
    ASSERT_EQ(false, m_GetDriverNameModel->m_Stop);
    QTemporaryDir dir;
    dir.setAutoRemove(true);
    m_GetDriverNameModel->mp_driversList << dir.filePath(QStringLiteral("pre-a.deb"));
    ASSERT_EQ(1, m_GetDriverNameModel->mp_driversList.size());

    // Act
    m_GetDriverNameModel->stopLoadingDrivers();

    // Assert：停止标志置位，且不影响已有列表内容
    EXPECT_EQ(true, m_GetDriverNameModel->m_Stop);
    EXPECT_EQ(1, m_GetDriverNameModel->mp_driversList.size());
}

// traverseFolders：目录不存在 → 直接返回，两列表保持为空
TEST_F(UT_GetDriverNameModel, traverseFolders_NonexistentDir_ListsStayEmpty)
{
    // Arrange：构造一个必然不存在的路径
    const QString missing = QDir::temp().filePath(QStringLiteral("dvm-missing-%1").arg(
        reinterpret_cast<quintptr>(this)));

    // Act
    m_GetDriverNameModel->traverseFolders(missing);

    // Assert
    EXPECT_EQ(0, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_EQ(0, m_GetDriverNameModel->mp_driversList.size());
}

// traverseFolders（不递归）：平铺目录收集 .deb/.ko，忽略其他后缀
TEST_F(UT_GetDriverNameModel, traverseFolders_FlatDir_CollectsDebAndKoOnly)
{
    // Arrange：临时目录放 2 个驱动文件与 1 个干扰文件
    QScopedPointer<QTemporaryDir> dir(mkDir(2, 1));

    // Act
    m_GetDriverNameModel->traverseFolders(dir->path());

    // Assert：收集 2 项且文件名与绝对路径对应
    EXPECT_EQ(2, m_GetDriverNameModel->mp_driversList.size());
    ASSERT_EQ(2, m_GetDriverNameModel->mp_driverPathList.size());
    for (int i = 0; i < 2; ++i) {
        const QFileInfo info(m_GetDriverNameModel->mp_driverPathList[i]);
        EXPECT_EQ(m_GetDriverNameModel->mp_driversList[i], info.fileName());
        EXPECT_EQ(true, info.isAbsolute());
    }
}

// traverseFolders（不递归）：子目录中的驱动文件不收集
TEST_F(UT_GetDriverNameModel, traverseFolders_NoRecursion_SkipsSubdirFiles)
{
    // Arrange：根目录 1 个驱动 + 子目录 1 个驱动
    QScopedPointer<QTemporaryDir> dir(mkDir(1, 0, 1));

    // Act
    m_GetDriverNameModel->traverseFolders(dir->path(), false);

    // Assert：仅收集根目录 1 项
    EXPECT_EQ(1, m_GetDriverNameModel->mp_driversList.size());
    EXPECT_EQ(1, m_GetDriverNameModel->mp_driverPathList.size());
}

// traverseFolders（递归）：子目录中的驱动文件一并收集
TEST_F(UT_GetDriverNameModel, traverseFolders_Recursion_CollectsSubdirFiles)
{
    // Arrange：根目录 1 个 .deb + 子目录 1 个 .deb
    QScopedPointer<QTemporaryDir> dir(mkDir(1, 0, 1));

    // Act
    m_GetDriverNameModel->traverseFolders(dir->path(), true);

    // Assert：递归共收集 2 项
    EXPECT_EQ(2, m_GetDriverNameModel->mp_driversList.size());
    EXPECT_EQ(2, m_GetDriverNameModel->mp_driverPathList.size());
}

// traverseFolders：停止标志置位 → 跳过遍历，列表保持为空
TEST_F(UT_GetDriverNameModel, traverseFolders_StopFlagSet_SkipsTraversal)
{
    // Arrange：预置停止标志与含驱动文件的临时目录
    QScopedPointer<QTemporaryDir> dir(mkDir(2, 0));
    m_GetDriverNameModel->m_Stop = true;

    // Act
    m_GetDriverNameModel->traverseFolders(dir->path(), true);

    // Assert
    EXPECT_EQ(0, m_GetDriverNameModel->mp_driversList.size());
    EXPECT_EQ(0, m_GetDriverNameModel->mp_driverPathList.size());
}

// startLoadDrivers（不含子目录）：遍历目录填充 model 两列并发出完成信号
TEST_F(UT_GetDriverNameModel, startLoadDrivers_NoSubDir_BuildsModelRowsAndEmitsFinish)
{
    // Arrange：含 2 个驱动文件的临时目录 + 完成信号探针
    QScopedPointer<QTemporaryDir> dir(mkDir(2, 0));
    QStandardItemModel model;
    QSignalSpy spy(m_GetDriverNameModel, &GetDriverNameModel::finishLoadDrivers);
    ASSERT_EQ(0, spy.count());

    // Act：includeSub=false 走本地遍历分支
    m_GetDriverNameModel->startLoadDrivers(&model, false, dir->path());

    // Assert：2 行、第二列文件名、UserRole 为绝对路径、tooltip 同文件名、完成信号 1 次
    EXPECT_EQ(2, model.rowCount());
    ASSERT_NE(nullptr, model.item(0, 1));
    ASSERT_NE(nullptr, model.item(0, 0));
    EXPECT_EQ(m_GetDriverNameModel->mp_driversList[0], model.item(0, 1)->text());
    EXPECT_EQ(m_GetDriverNameModel->mp_driverPathList[0],
              model.item(0, 0)->data(Qt::UserRole).toString());
    EXPECT_EQ(model.item(0, 1)->text(), model.item(0, 1)->toolTip());
    EXPECT_EQ(1, spy.count());
}

// startLoadDrivers（含子目录）：DBus 查询成功 → 直接采用 DBus 结果
TEST_F(UT_GetDriverNameModel, startLoadDrivers_IncludeSub_DbusSuccess_UsesSearchResult)
{
    // Arrange：stub searchDriver 成功并返回 2 个路径 + 完成信号探针
    QTemporaryDir dir;
    dir.setAutoRemove(true);
    const QString p1 = dir.filePath(QStringLiteral("from-dbus-a.deb"));
    const QString p2 = dir.filePath(QStringLiteral("from-dbus-b.ko"));
    g_utSearchOk = true;
    g_utSearchResult << p1 << p2;
    Stub stub;
    stub.set(ADDR(DBusAnythingInterface, searchDriver), ut_searchDriver);
    QStandardItemModel model;
    QSignalSpy spy(m_GetDriverNameModel, &GetDriverNameModel::finishLoadDrivers);

    // Act：includeSub=true 走 DBus 分支
    m_GetDriverNameModel->startLoadDrivers(&model, true, dir.path());

    // Assert：2 行、文件名取自 DBus 路径、完成信号 1 次
    EXPECT_EQ(2, model.rowCount());
    EXPECT_EQ(QStringLiteral("from-dbus-a.deb"), model.item(0, 1)->text());
    EXPECT_EQ(2, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_EQ(1, spy.count());
}

// startLoadDrivers（含子目录）：DBus 查询失败 → 回退本地递归遍历
TEST_F(UT_GetDriverNameModel, startLoadDrivers_IncludeSub_DbusFail_FallsBackToTraverse)
{
    // Arrange：stub searchDriver 失败 + 根/子目录各 1 个驱动文件
    g_utSearchOk = false;
    Stub stub;
    stub.set(ADDR(DBusAnythingInterface, searchDriver), ut_searchDriver);
    QScopedPointer<QTemporaryDir> dir(mkDir(1, 0, 1));
    QStandardItemModel model;
    QSignalSpy spy(m_GetDriverNameModel, &GetDriverNameModel::finishLoadDrivers);

    // Act
    m_GetDriverNameModel->startLoadDrivers(&model, true, dir->path());

    // Assert：回退递归遍历收集 2 项并填入 model
    EXPECT_EQ(2, m_GetDriverNameModel->mp_driversList.size());
    EXPECT_EQ(2, model.rowCount());
    EXPECT_EQ(1, spy.count());
}

// startLoadDrivers（含子目录）：查询期间置停止 → 填充循环中止，不填行不发射
TEST_F(UT_GetDriverNameModel, startLoadDrivers_StopDuringSearch_AbortsFillNoFinishSignal)
{
    // Arrange：stub searchDriver 成功返回 1 项且在返回前置位停止标志
    QTemporaryDir dir;
    dir.setAutoRemove(true);
    g_utSearchOk = true;
    g_utSearchResult << dir.filePath(QStringLiteral("late-a.deb"));
    g_utStopTarget = m_GetDriverNameModel;
    Stub stub;
    stub.set(ADDR(DBusAnythingInterface, searchDriver), ut_searchDriverStop);
    QStandardItemModel model;
    QSignalSpy spy(m_GetDriverNameModel, &GetDriverNameModel::finishLoadDrivers);

    // Act：startLoadDrivers 复位 m_Stop 后进入 DBus 分支，桩在返回前置停止，
    // 填充循环在首个元素处即中止
    m_GetDriverNameModel->startLoadDrivers(&model, true, dir.path());

    // Assert：未填充任何行且未发出完成信号
    EXPECT_EQ(0, model.rowCount());
    EXPECT_EQ(1, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_EQ(0, spy.count());
}
