// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// DriverBackupThread::run 主流程用例(任务06, cx=21):
// QProcess(apt download)/QTemporaryDir::path/DBusDriverInterface::aptUpdate+backupDebFd 全打桩。
// 不执行真实 apt、不访问真实 DBus 服务, 只操作测试私有临时目录。
// 注意: aptUpdateFail 用例必须在文件内最先定义(静态 updateFlag 首次置位语义)。

#include "DriverBackupThread.h"
#include "DBusDriverInterface.h"
#include "MacroDefinition.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>

#include <fcntl.h>
#include <cstdarg>
#include <sys/syscall.h>
#include <unistd.h>

#include <gtest/gtest.h>

// ---------------- stub 状态 ----------------
static QString ut_bt_tmpRoot;            // QTemporaryDir::path 桩返回的固定根目录
static bool ut_bt_startCalled = false;   // QProcess::start 是否被调用
static bool ut_bt_createDeb = false;     // QProcess::start 桩是否模拟产出 deb
static bool ut_bt_aptUpdateRet = true;   // aptUpdate 桩返回值
static bool ut_bt_aptUpdateCalled = false;
static QString ut_bt_backupDebName;      // backupDebFd 桩捕获的包名
static bool ut_bt_backupDebRet = true;
static DriverBackupThread *ut_bt_thread = nullptr;

static bool ut_bt_dnsError = false;        // readAllStandardOutput 桩返回域名解析错误文本
static bool ut_bt_emitAtStart = false;     // 在 QProcess::start 桩内触发 readyRead
static bool ut_bt_emitAtWait = false;      // 在 waitForFinished 桩内触发 readyRead
static bool ut_bt_emitAtEntry = false;     // 在 entryInfoList 桩内触发 readyRead
static void *ut_bt_lastProcess = nullptr;  // 最近一次进入 start 桩的 QProcess
static QByteArray ut_bt_procOut;           // readAllStandardOutput 桩返回值
static bool ut_bt_msleepFlipsStatus = false;
static void *ut_bt_owner = nullptr;        // 当前测试的线程对象(供 msleep 桩改状态)

static void ut_reset_bt_stubs()
{
    ut_bt_tmpRoot.clear();
    ut_bt_startCalled = false;
    ut_bt_createDeb = false;
    ut_bt_aptUpdateRet = true;
    ut_bt_aptUpdateCalled = false;
    ut_bt_backupDebName.clear();
    ut_bt_backupDebRet = true;
    ut_bt_thread = nullptr;
    ut_bt_dnsError = false;
    ut_bt_emitAtStart = false;
    ut_bt_emitAtWait = false;
    ut_bt_emitAtEntry = false;
    ut_bt_lastProcess = nullptr;
    ut_bt_procOut.clear();
    ut_bt_msleepFlipsStatus = false;
    ut_bt_owner = nullptr;
}

// ---------------- 桩函数 ----------------
static QString ut_stub_tmpdir_path(void *)
{
    return ut_bt_tmpRoot;
}

static void ut_bt_emit_readyRead(void *process)
{
    // run() 与 QProcess 同线程, readyReadStandardOutput 直连, 同步触发已连接的 lambda
    emit static_cast<QProcess *>(process)->readyReadStandardOutput(QProcess::QPrivateSignal{});
}

static void ut_stub_qprocess_start(void *self, const QString &, const QStringList &args, QIODevice::OpenMode)
{
    ut_bt_startCalled = true;
    ut_bt_lastProcess = self;
    if (ut_bt_emitAtStart)
        ut_bt_emit_readyRead(self);
    if (!ut_bt_createDeb)
        return;
    // 模拟 apt download 产出: <tmpRoot>/driver/<pkg>/<pkg>_1.0.0_amd64.deb
    const QString pkg = args.size() > 1 ? args.at(1).section('=', 0, 0) : QString();
    const QString dir = ut_bt_tmpRoot + "/driver/" + pkg;
    QFile f(dir + "/" + pkg + "_1.0.0_amd64.deb");
    f.open(QIODevice::WriteOnly);
}

static QByteArray ut_stub_qprocess_readAllStdout(void *)
{
    return ut_bt_procOut;
}

static bool ut_stub_qprocess_waitForFinished(void *self, int)
{
    if (ut_bt_emitAtWait)
        ut_bt_emit_readyRead(self);
    return true;
}

// 在 entryInfoList 触发 readyRead: 模拟下载中途 lambda 置停后, 文件遍历检测停止
static QList<QFileInfo> ut_stub_entryInfoList(void *, QDir::Filters, QDir::SortFlags)
{
    QList<QFileInfo> lst;
    if (ut_bt_emitAtEntry)
        ut_bt_emit_readyRead(ut_bt_lastProcess);
    const QString dir = ut_bt_tmpRoot + "/driver/ut-pkg";
    lst << QFileInfo(dir + "/ut-pkg_1.0.0_amd64.deb");
    return lst;
}

static void ut_stub_qthread_msleep(unsigned long)
{
    if (ut_bt_msleepFlipsStatus && ut_bt_owner)
        static_cast<DriverBackupThread *>(ut_bt_owner)->setStatus(DriverBackupThread::Success);
}

static bool ut_stub_aptUpdate(void *)
{
    ut_bt_aptUpdateCalled = true;
    return ut_bt_aptUpdateRet;
}

static bool ut_stub_backupDebFd(void *, const QDBusUnixFileDescriptor &, const QString &debname)
{
    ut_bt_backupDebName = debname;
    return ut_bt_backupDebRet;
}

class UT_DriverBackupThreadRun : public UT_HEAD
{
public:
    void SetUp() override
    {
        ut_reset_bt_stubs();
        m_dir.reset(new QTemporaryDir);
        ASSERT_TRUE(m_dir->isValid());
        ut_bt_tmpRoot = m_dir->path();
        m_thread = new DriverBackupThread;
        ut_bt_thread = m_thread;
        ut_bt_owner = m_thread;
    }
    void TearDown() override
    {
        delete m_thread;
        m_thread = nullptr;
        ut_bt_thread = nullptr;
    }

    DriverBackupThread *m_thread = nullptr;
    QScopedPointer<QTemporaryDir> m_dir;
};

// 必须最先执行: 依赖静态 updateFlag 首次为 false
TEST_F(UT_DriverBackupThreadRun, aptUpdateFail_emitsFalse)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    ut_bt_aptUpdateRet = false;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_TRUE(ut_bt_aptUpdateCalled);
    EXPECT_FALSE(ut_bt_startCalled);   // apt 失败后不应走到下载
}

TEST_F(UT_DriverBackupThreadRun, emptyInfo_emitsFalseEarly)
{
    DriverInfo info;   // 包名与版本均空
    m_thread->setBackupDriverInfo(&info);
    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_FALSE(ut_bt_startCalled);
}

TEST_F(UT_DriverBackupThreadRun, noDebDownloaded_emitsFalse)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    ut_bt_createDeb = false;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_TRUE(ut_bt_startCalled);    // 到达下载步骤
    EXPECT_TRUE(ut_bt_backupDebName.isEmpty());
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

TEST_F(UT_DriverBackupThreadRun, debDownloaded_success_emitsTrue)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    ut_bt_createDeb = true;
    ut_bt_backupDebRet = true;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);
    // 预置状态为 Success: 跳过 Waiting 自旋(msleep), 走备份成功分支
    m_thread->setStatus(DriverBackupThread::Success);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_TRUE(ut_bt_startCalled);
    EXPECT_EQ(QStringLiteral("ut-pkg"), ut_bt_backupDebName);
    EXPECT_EQ(1, spy.count());
    EXPECT_TRUE(spy.at(0).at(0).toBool());
    // 模拟下载的 deb 备份后应被清理
    EXPECT_FALSE(QFile::exists(ut_bt_tmpRoot + "/driver/ut-pkg/ut-pkg_1.0.0_amd64.deb"));
}

TEST_F(UT_DriverBackupThreadRun, debDownloaded_failed_emitsFalse)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    ut_bt_createDeb = true;
    ut_bt_backupDebRet = true;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);
    m_thread->setStatus(DriverBackupThread::Failed);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

TEST_F(UT_DriverBackupThreadRun, stoppedBeforeRun_noEmit)
{
    // m_isStop 默认 true(未 setBackupDriverInfo) -> 条件不满足分支
    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_EQ(0, spy.count());
}

TEST_F(UT_DriverBackupThreadRun, undoBackup_stopsFlag)
{
    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);
    m_thread->undoBackup();
    // 私有成员经 -fno-access-control 直读
    EXPECT_TRUE(m_thread->m_isStop);
}

// 下载过程中输出域名解析错误: readyRead lambda 置停并发失败信号,
// 随后 run 在 waitForFinished 后检测停止提前返回(覆盖 L103-106)
TEST_F(UT_DriverBackupThreadRun, dnsErrorInOutput_stopsAfterProcess)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_stub_qprocess_readAllStdout);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::msleep), ut_stub_qthread_msleep);
    ut_bt_emitAtStart = true;
    ut_bt_dnsError = true;
    ut_bt_procOut = "E: 无法解析域名 archive.utcu.com.cn\n";

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    // lambda 与 waitForFinished 后各发一次失败信号
    EXPECT_GE(spy.count(), 2);
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

// 文件遍历中被置停: entryInfoList 期间模拟 DNS 错误 lambda 触发, 遍历内检测停止后返回
TEST_F(UT_DriverBackupThreadRun, stoppedDuringFileIteration)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_stub_qprocess_readAllStdout);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    stub.set(static_cast<QFileInfoList (QDir::*)(QDir::Filters, QDir::SortFlags) const>(&QDir::entryInfoList),
             ut_stub_entryInfoList);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::msleep), ut_stub_qthread_msleep);
    ut_bt_createDeb = true;
    ut_bt_emitAtEntry = true;
    ut_bt_dnsError = true;
    ut_bt_procOut = "E: 无法解析域名 archive.utcu.com.cn\n";

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    // lambda 一次 + 遍历内停止一次
    ASSERT_EQ(2, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_FALSE(spy.at(1).at(0).toBool());
}

// Waiting 自旋: 状态保持 Waiting 进入循环, msleep 桩模拟前端置 Success 后退出
TEST_F(UT_DriverBackupThreadRun, waitingSpin_exitsWhenStatusSet)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_stub_qprocess_readAllStdout);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::msleep), ut_stub_qthread_msleep);
    ut_bt_createDeb = true;
    ut_bt_backupDebRet = true;
    ut_bt_msleepFlipsStatus = true;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);
    // m_status 保持默认 Waiting

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    EXPECT_EQ(QStringLiteral("ut-pkg"), ut_bt_backupDebName);
    ASSERT_EQ(1, spy.count());
    EXPECT_TRUE(spy.at(0).at(0).toBool());
}

// 目录 fd 打开失败(模拟权限异常): 仅对 O_DIRECTORY 的 open 返回 -1
// (即 run() 内 L136 的暂存目录 open), 其他 open(如 Qt 创建 deb 文件)经
// syscall 直连内核放行(不能用 dlsym 取 open64, 那会拿到已打桩的入口导致递归)
static int ut_stub_open_fail(const char *path, int flags, ...)
{
    if (flags & O_DIRECTORY)
        return -1;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        const int mode = va_arg(ap, int);
        va_end(ap);
        return (int)syscall(SYS_openat, AT_FDCWD, path, flags, mode);
    }
    return (int)syscall(SYS_openat, AT_FDCWD, path, flags);
}

TEST_F(UT_DriverBackupThreadRun, openDirFdFail_emitsFalse)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_stub_qprocess_readAllStdout);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::msleep), ut_stub_qthread_msleep);
    stub.set((void *)::open, (void *)ut_stub_open_fail);
    ut_bt_createDeb = true;

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);
    m_thread->setStatus(DriverBackupThread::Success);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    // open 失败路径下 backupDebFd 不应被调用
    EXPECT_TRUE(ut_bt_backupDebName.isEmpty());
    ASSERT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}

// 备份目录 mkpath 失败: 记录日志后继续走 "无 deb" 失败分支
static bool ut_stub_qdir_mkpath(void *, const QString &)
{
    return false;
}

TEST_F(UT_DriverBackupThreadRun, mkpathFail_continuesToFail)
{
    Stub stub;
    stub.set(ADDR(QTemporaryDir, path), ut_stub_tmpdir_path);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_stub_qprocess_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_stub_qprocess_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_stub_qprocess_readAllStdout);
    stub.set(ADDR(DBusDriverInterface, aptUpdate), ut_stub_aptUpdate);
    stub.set(ADDR(DBusDriverInterface, backupDebFd), ut_stub_backupDebFd);
    stub.set(ADDR(QDir, mkpath), ut_stub_qdir_mkpath);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::msleep), ut_stub_qthread_msleep);

    DriverInfo info;
    info.m_Packages = "ut-pkg";
    info.m_DebVersion = "1.0.0";
    m_thread->setBackupDriverInfo(&info);

    QSignalSpy spy(m_thread, &DriverBackupThread::backupProgressFinished);
    m_thread->run();
    ASSERT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
}
