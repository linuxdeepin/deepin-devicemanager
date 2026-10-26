// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// ut_CpuInfo.cpp —— CpuInfo 单元测试（整体重写，原文件仅 1 个环境依赖用例）
//
// 被测对象: deepin-deviceinfo/src/loadinfo/cpu/cpuinfo.cpp（CpuInfo，22 个方法）
// 源码基线: ut-t28 @ 6ccddaf4（分支清单行号均以该版本 cpuinfo.cpp 为准）
//
// 打桩策略（全部经 tests/src/stub.h 的 Stub RAII，作用域结束自动还原）：
//   1. 虚拟文件系统（VFS）: 静态 s_vfs 表模拟 /proc/cpuinfo 与 /sys/devices/system/cpu
//      下的全部文件与目录。QFile::open（虚函数，dlsym 取实现符号替换——成员指针编码为
//      vtable 偏移，ADDR 无法定位实现体；打在实现体上虚派发同样命中）、
//      QIODevice::readAll（非虚，ADDR）、QIODevice::close（虚函数，dlsym）全部改写为
//      查表读写；QDir::entryInfoList 与 QDir::exists(QString) 同理按 VFS 键前缀枚举。
//      由此 readProcCpuinfo 硬编码的 /proc/cpuinfo 与全部 sysfs 路径均被注入，
//      测试不依赖真实设备环境。
//   2. uname 系统调用：替换 glibc uname 符号，注入 machine 字段与返回值。
//
// 分支清单（get_code_snippet @ 6ccddaf4 实源）与用例映射：
//   loadCpuInfo          B1 !readProcCpuinfo→false        → LoadCpuInfo_ProcCpuinfoMissing_ReturnsFalse
//                        B2 重编号循环 0..N 次             → LoadCpuInfo_FullTopology_RenumbersCores
//                        B3 重编号含哨兵(-1, 无核)          → LoadCpuInfo_ProcOnly_SentinelOnly
//                        B4 全空仍返回 true                → LoadCpuInfo_EmptyProc_TrueEmptyState
//   readCpuArchitecture  B1 uname 失败→保持原值            → ReadCpuArchitecture_UnameFailed_KeepsArch
//                        B2 成功→m_Arch=machine           → Arch_AfterReadCpuArchitecture_MatchesUname
//   readProcCpuinfo      B1 open 失败→false               → ReadProcCpuinfo_OpenFailed_ReturnsFalse
//                        B2 段落遍历/空段 continue         → ReadProcCpuinfo_ParsesStanzas_UpdatesLogical
//                        B3 空内容→true                   → ReadProcCpuinfo_EmptyContent_ReturnsTrue
//                        B4 全空段落→true                 → ReadProcCpuinfo_OnlyEmptyStanzas_ReturnsTrue
//   parseInfo            B1 空串→false                    → ParseInfo_Empty_ReturnsFalse
//                        B2/B3 行空或非两段→continue       → ParseInfo_NoProcessor_ReturnsFalse
//                        B4/B5 core/package 别名映射       → ParseInfo_CoreAndLogicalExisted_UpdatesFields
//                        B6 其余键 toLower 入表            → ParseInfo_CoreAndLogicalExisted_UpdatesFields
//                        B7 processor 行取 logical_id      → 全部 ParseInfo_* 用例
//                        B8 logical_id<0→false            → ParseInfo_NoProcessor_ReturnsFalse
//                        B10 物理 id 不在表→false          → ParseInfo_PhysicalMissing_ReturnsFalse
//                        B11/B12 核不存在且物理有该逻辑     → ParseInfo_CoreMissing_UpdatesExistingLogical
//                        B13/B14/B15 核存在+核无该逻辑+物理有→ ParseInfo_LogicalMissingInCore_UpdatesOtherCoreLogical
//                        B16 核与逻辑均存在→setProcCpuinfo  → ParseInfo_CoreAndLogicalExisted_UpdatesFields
//                        B17 无物理/核信息→logicalCpu 兜底  → ParseInfo_NoPhysicalCore_FallsBackToSentinel
//   readSysCpu           B1 目录项非 cpuN→continue         → ReadSysCpu_MultiCpu_BuildsTopology
//                        B2 命中→readSysCpuN(N, path)     → ReadSysCpu_MultiCpu_BuildsTopology
//   readSysCpuN          B1 physical_id<0→return           → ReadSysCpuN_MissingPhysical_EarlyReturn
//                        B2 新物理入表                     → ReadSysCpuN_MissingSiblings_PhysicalOnly
//                        B3 tsl<0→return                  → ReadSysCpuN_MissingSiblings_PhysicalOnly
//                        B4 核已存在→复用                  → ReadSysCpuN_SameCoreReused_TwoLogicals
//                        B5 exists(cache)→readCpuCache     → ReadSysCpuN_CacheAndFreq_SetsAllFields
//                        B6 exists(cpufreq)→readCpuFreq    → ReadSysCpuN_CacheAndFreq_SetsAllFields
//                        B7 addLogicalCpu                 → ReadSysCpuN_CacheAndFreq_SetsAllFields
//   readPhysicalID       B1 open 失败→-1                  → ReadPhysicalID_MissingFile_Negative1
//                        B2 sw_64 且 toInt==-1→0           → ReadPhysicalID_Sw64Negative1_ReturnsZero
//                        B3 常规 toInt 透传                → ReadPhysicalID_X86Numeric_ReturnsValue
//   readThreadSiblings.. B1 open 失败→-1                  → ReadThreadSiblingsListPath_MissingFile_Negative1
//                        B2 空拆分结果→0                   → ReadThreadSiblingsListPath_EmptyContent_ReturnsZero
//                        B3 首段 toInt                    → ReadThreadSiblingsListPath_Range_ReturnsFirst
//   readCpuCache         B1 非 index 目录→continue         → ReadCpuCache_IndexApplied_NonIndexSkipped
//                        B2 index 目录→readCpuCacheIndex   → ReadCpuCache_IndexApplied_NonIndexSkipped
//   readCpuCacheIndex    B1 level 读不到→-1(无动作)         → ReadCpuCacheIndex_LevelMissing_NothingSet
//                        B2 level==2→L2                   → TEST_P Levels/ReadCpuCacheIndex_Level_SetsExpectedCache
//                        B3 level==3→L3                   → TEST_P Levels/ReadCpuCacheIndex_Level_SetsExpectedCache
//                        B4 level==4→L4                   → TEST_P Levels/ReadCpuCacheIndex_Level_SetsExpectedCache
//                        B5 level==1 含 Data→L1d           → TEST_P Levels/ReadCpuCacheIndex_Level_SetsExpectedCache
//                        B6 level==1 否则→L1i              → TEST_P Levels/ReadCpuCacheIndex_Level_SetsExpectedCache
//                        B7 level==5 无动作                → ReadCpuCacheIndex_Level5_Ignored
//   readCpuFreq          B1 文件缺失→保持原值              → ReadCpuFreq_FilesMissing_KeepsValues
//                        B2 频率 /1000 取整加 MHz          → ReadCpuFreq_AllPresent_SetsMhz
//   physicalNum          B1 无哨兵→size                   → PhysicalNum_Normal_ReturnsSize
//                        B2 有哨兵→size-1                 → PhysicalNum_WithSentinel_ExcludesSentinel
//   coreNum              B1 id<0→跳过                     → CoreNum_EmptyOrSentinel_ReturnsZero
//                        B2 累加各物理 coreNum             → CoreNum_MultiPhysical_SumsAll
//   logicalNum           B1 id<0→跳过                     → LogicalNum_WithSentinel_ExcludesSentinel
//                        B2 累加各物理 logicalNum          → LogicalNum_MultiPhysical_SumsAll
//   logicalCpu           B1 physical_id<0→continue         → LogicalCpu_SentinelSkipped_FallsBackToSentinel
//                        B2 命中→返回引用                  → LogicalCpu_Found_ReturnsReference
//                        B3 兜底哨兵(插入 -1 键)           → LogicalCpu_NotFound_InsertsSentinel
//   setProcCpuinfo       B1 mips64/loongarch64 特判        → SetProcCpuinfo_Mips64_SetsCpuMhzAndFeatures
//                        B2 flags 空→features 兜底         → SetProcCpuinfo_Mips64_SetsCpuMhzAndFeatures
//                        B3 x86 常规 7 字段                → SetProcCpuinfo_X86_SetsFields
//
// 最小清单：[x]等价类 [x]边界值 [x]异常路径 [x]负面场景 [x]分支映射 [x]TEST_P(5 组同构)
//
// 用例计数声明（self-check-structural §2 校验 actual ≥ min）：
// | method                     | level | factors                                              | min | actual |
// | CpuInfo(ctor)              | low   | []                                                   | 1   | 1 |
// | ~CpuInfo(dtor)             | low   | [destructor]                                         | 1   | 1 |
// | arch                       | mid   | [in_degree:2]                                        | 2   | 2 |
// | coreNum                    | mid   | [in_degree:3]                                        | 2   | 2 |
// | diagPrintInfo              | low   | []                                                   | 1   | 1 |
// | loadCpuInfo                | high  | [transitive_loop_depth:4, in_degree:2]               | 4   | 4 |
// | logicalCpu                 | low   | []                                                   | 1   | 3 |
// | logicalCpus                | mid   | [in_degree:2]                                        | 2   | 2 |
// | logicalNum                 | mid   | [in_degree:3]                                        | 2   | 2 |
// | parseInfo                  | high  | [complexity:17, cognitive:38, lines:65]              | 3   | 7 |
// | physicalNum                | mid   | [in_degree:3]                                        | 2   | 2 |
// | readCoreID                 | high  | [transitive_loop_depth:3]                            | 4   | 4 |
// | readCpuArchitecture        | low   | []                                                   | 1   | 2 |
// | readCpuCache               | low   | []                                                   | 1   | 1 |
// | readCpuCacheIndex          | high  | [complexity:9, cognitive:19, transitive_loop_depth:3]| 4   | 7 |
// | readCpuFreq                | high  | [transitive_loop_depth:3]                            | 4   | 4 |
// | readPhysicalID             | high  | [transitive_loop_depth:3]                            | 4   | 6 |
// | readProcCpuinfo            | high  | [transitive_loop_depth:3]                            | 4   | 4 |
// | readSysCpu                 | low   | []                                                   | 1   | 2 |
// | readSysCpuN                | high  | [complexity:6, transitive_loop_depth:3]              | 4   | 5 |
// | readThreadSiblingsListPath | high  | [transitive_loop_depth:3]                            | 4   | 5 |
// | setProcCpuinfo             | low   | []                                                   | 1   | 3 |
// ============================================================================

#include <gtest/gtest.h>
#include "../stub.h"
#include "cpu/cpuinfo.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>

#include <cstring>
#include <dlfcn.h>
#include <sys/utsname.h>

// ---------------------------------------------------------------------------
// 虚拟文件系统（VFS）：被测代码读到的所有路径与内容都登记在 s_vfs，桩按路径查表
// ---------------------------------------------------------------------------
static QMap<QString, QByteArray> s_vfs;
static QString s_arch = "x86_64";
static int s_unameRet = 0;

static void vfsSetFile(const QString &path, const QByteArray &content)
{
    s_vfs.insert(path, content);
}

static void vfsSetDir(const QString &path)
{
    s_vfs.insert(path + "/", QByteArray());
}

static void vfsProcCpuinfo(const QByteArray &content)
{
    vfsSetFile("/proc/cpuinfo", content);
}

static void vfsCpuTopology(int n, int physId, int coreId)
{
    const QString base = QString("/sys/devices/system/cpu/cpu%1/topology").arg(n);
    vfsSetFile(base + "/physical_package_id", QByteArray::number(physId));
    vfsSetFile(base + "/thread_siblings_list", QByteArray::number(coreId));
}

static void vfsCpuCacheIndex(int n, int index, int level, const char *type,
                             const char *size, const char *shared)
{
    const QString base = QString("/sys/devices/system/cpu/cpu%1/cache").arg(n);
    vfsSetDir(base);
    const QString idx = QString("%1/index%2").arg(base).arg(index);
    vfsSetFile(idx + "/level", QByteArray::number(level));
    vfsSetFile(idx + "/type", type);
    vfsSetFile(idx + "/size", size);
    vfsSetFile(idx + "/shared_cpu_list", shared);
}

static void vfsCpuFreq(int n, const char *minF, const char *curF, const char *maxF)
{
    const QString base = QString("/sys/devices/system/cpu/cpu%1/cpufreq").arg(n);
    vfsSetDir(base);
    if (minF)
        vfsSetFile(base + "/cpuinfo_min_freq", minF);
    if (curF)
        vfsSetFile(base + "/scaling_cur_freq", curF);
    if (maxF)
        vfsSetFile(base + "/cpuinfo_max_freq", maxF);
}

// --- 桩函数 ---
static bool ut_qfile_open(void *obj, QIODevice::OpenMode mode)
{
    Q_UNUSED(mode);
    QFile *f = static_cast<QFile *>(obj);
    return s_vfs.contains(f->fileName());
}

static QByteArray ut_qiodevice_readAll(void *obj)
{
    QFile *f = static_cast<QFile *>(obj);
    return s_vfs.value(f->fileName());
}

static void ut_qiodevice_close(void *)
{
}

static QFileInfoList ut_qdir_entryInfoList(void *obj, QDir::Filters filters, QDir::SortFlags sort)
{
    Q_UNUSED(filters);
    Q_UNUSED(sort);
    QDir *d = static_cast<QDir *>(obj);
    const QString dirPath = d->absolutePath();
    const QString base = dirPath + "/";
    QFileInfoList out;
    QSet<QString> seen;
    for (auto it = s_vfs.constBegin(); it != s_vfs.constEnd(); ++it) {
        const QString &key = it.key();
        if (!key.startsWith(base))
            continue;
        QString rest = key.mid(base.size());
        const int slash = rest.indexOf('/');
        if (slash >= 0)
            rest = rest.left(slash);   // 子目录名
        if (rest.isEmpty() || seen.contains(rest))
            continue;
        seen.insert(rest);
        out.append(QFileInfo(dirPath + "/" + rest));
    }
    return out;
}

static bool ut_qdir_exists_name(void *obj, const QString &name)
{
    QDir *d = static_cast<QDir *>(obj);
    const QString target = QDir::cleanPath(d->absolutePath() + "/" + name);
    return s_vfs.contains(target) || s_vfs.contains(target + "/");
}

static int ut_uname(struct utsname *buf)
{
    memset(buf, 0, sizeof(struct utsname));
    const QByteArray machine = s_arch.toLocal8Bit();
    memcpy(buf->machine, machine.constData(),
           static_cast<size_t>(qMin(machine.size(), static_cast<int>(sizeof(buf->machine)) - 1)));
    return s_unameRet;
}

// --- 符号获取：QFile::open / QIODevice::close 为虚函数，须 dlsym 定位实现体 ---
static void *qtSym(const char *name)
{
    static void *handle = dlopen("libQt6Core.so.6", RTLD_NOW | RTLD_GLOBAL);
    return handle ? dlsym(handle, name) : nullptr;
}

static void *qtFileOpenSym()
{
    // Qt6: QIODeviceBase::OpenModeFlag；Qt5 兜底: QIODevice::OpenModeFlag
    static void *p = qtSym("_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    if (!p)
        p = qtSym("_ZN5QFile4openE6QFlagsIN9QIODevice10OpenModeFlagEE");
    return p;
}

static void *qtIoCloseSym()
{
    static void *p = qtSym("_ZN9QIODevice5closeEv");
    return p;
}

static void installVfsStubs(Stub &stub)
{
    void *openSym = qtFileOpenSym();
    void *closeSym = qtIoCloseSym();
    if (!openSym || !closeSym) {
        ADD_FAILURE() << "Qt symbol lookup failed: open=" << openSym << " close=" << closeSym;
        return;
    }
    stub.set(openSym, (void *)&ut_qfile_open);
    stub.set(ADDR(QIODevice, readAll), ut_qiodevice_readAll);
    stub.set(closeSym, (void *)&ut_qiodevice_close);
    stub.set((QFileInfoList (QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList),
             ut_qdir_entryInfoList);
    stub.set((bool (QDir::*)(const QString &) const)ADDR(QDir, exists), ut_qdir_exists_name);
}

static void installUnameStub(Stub &stub)
{
    stub.set((int (*)(struct utsname *))&uname, &ut_uname);
}

// 直接向 CpuInfo 私有表注入 物理0->核X->逻辑Y 链路（-fno-access-control 下可访问私有成员）
static void seedLogicalCpu(CpuInfo &cpu, int physId, int coreId, int logicalId)
{
    PhysicalCpu physical(physId);
    CoreCpu core(coreId);
    LogicalCpu logical;
    logical.setLogicalID(logicalId);
    core.addLogicalCpu(logicalId, logical);
    physical.addCoreCpu(coreId, core);
    cpu.m_MapPhysicalCpu.insert(physId, physical);
}

// ---------------------------------------------------------------------------
// 测试夹具
// ---------------------------------------------------------------------------
class CpuInfo_UT : public ::testing::Test
{
public:
    void SetUp() override
    {
        s_vfs.clear();
        s_arch = "x86_64";
        s_unameRet = 0;
    }
    void TearDown() override
    {
    }
};

// ===== 生命周期 / 基础查询 =====

TEST_F(CpuInfo_UT, DefaultConstructor_Counters_AllZero)
{
    // Arrange
    CpuInfo cpu;
    // Act
    const int phys = cpu.physicalNum();
    const int cores = cpu.coreNum();
    const int logicals = cpu.logicalNum();
    // Assert — ctor 初始化 m_Arch 为 "unknow"（源码字面拼写）
    EXPECT_EQ(cpu.arch(), QString("unknow"));
    EXPECT_EQ(phys, 0);
    EXPECT_EQ(cores, 0);
    EXPECT_EQ(logicals, 0);
}

TEST_F(CpuInfo_UT, ObjectLifecycle_ConstructDestruct_Clean)
{
    // Arrange — 空对象先取一次基线
    CpuInfo cpu;
    const int emptyCount = cpu.physicalNum();
    // Act — 注入数据，作用域结束时析构
    seedLogicalCpu(cpu, 0, 3, 5);
    const int filledCount = cpu.logicalNum();
    // Assert — 析构无崩溃，由 gtest 正常收尾即视为通过
    EXPECT_EQ(emptyCount, 0);
    EXPECT_EQ(filledCount, 1);
}

// ===== arch =====

TEST_F(CpuInfo_UT, Arch_Default_IsUnknow)
{
    // Arrange
    CpuInfo cpu;
    // Act
    const QString &value = cpu.arch();
    // Assert
    EXPECT_EQ(value, QString("unknow"));
    EXPECT_FALSE(value.isEmpty());
}

TEST_F(CpuInfo_UT, Arch_AfterReadCpuArchitecture_MatchesUname)
{
    // Arrange
    Stub stub;
    installUnameStub(stub);
    s_arch = "loongarch64";
    CpuInfo cpu;
    // Act
    cpu.readCpuArchitecture();
    // Assert
    EXPECT_EQ(cpu.arch(), QString("loongarch64"));
    EXPECT_NE(cpu.arch(), QString("unknow"));
}

// ===== readCpuArchitecture =====

TEST_F(CpuInfo_UT, ReadCpuArchitecture_UnameFailed_KeepsArch)
{
    // Arrange
    Stub stub;
    installUnameStub(stub);
    s_unameRet = -1;
    CpuInfo cpu;
    cpu.m_Arch = "arm64";
    // Act
    cpu.readCpuArchitecture();
    // Assert
    EXPECT_EQ(cpu.arch(), QString("arm64"));
    EXPECT_NE(cpu.arch(), QString("x86_64"));
}

// ===== physicalNum / coreNum / logicalNum =====

TEST_F(CpuInfo_UT, PhysicalNum_Normal_ReturnsSize)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 0, 0);
    seedLogicalCpu(cpu, 1, 0, 1);
    // Act
    const int num = cpu.physicalNum();
    // Assert
    EXPECT_EQ(num, 2);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 2);
}

TEST_F(CpuInfo_UT, PhysicalNum_WithSentinel_ExcludesSentinel)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 0, 0);
    cpu.m_MapPhysicalCpu.insert(-1, PhysicalCpu(-1));
    // Act
    const int num = cpu.physicalNum();
    // Assert
    EXPECT_EQ(num, 1);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 2);
}

TEST_F(CpuInfo_UT, CoreNum_MultiPhysical_SumsAll)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    seedLogicalCpu(cpu, 1, 7, 9);
    // Act
    const int num = cpu.coreNum();
    // Assert
    EXPECT_EQ(num, 2);
    EXPECT_EQ(cpu.physicalNum(), 2);
}

TEST_F(CpuInfo_UT, CoreNum_EmptyOrSentinel_ReturnsZero)
{
    // Arrange
    CpuInfo cpu;
    cpu.m_MapPhysicalCpu.insert(-1, PhysicalCpu(-1));
    // Act
    const int num = cpu.coreNum();
    // Assert
    EXPECT_EQ(num, 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
}

TEST_F(CpuInfo_UT, LogicalNum_MultiPhysical_SumsAll)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    seedLogicalCpu(cpu, 1, 7, 9);
    CoreCpu extra(8);
    LogicalCpu second;
    second.setLogicalID(10);
    extra.addLogicalCpu(10, second);
    cpu.m_MapPhysicalCpu[1].addCoreCpu(8, extra);
    // Act
    const int num = cpu.logicalNum();
    // Assert
    EXPECT_EQ(num, 3);
    EXPECT_EQ(cpu.coreNum(), 3);
}

TEST_F(CpuInfo_UT, LogicalNum_WithSentinel_ExcludesSentinel)
{
    // Arrange
    CpuInfo cpu;
    cpu.m_MapPhysicalCpu.insert(-1, PhysicalCpu(-1));
    // Act
    const int num = cpu.logicalNum();
    // Assert
    EXPECT_EQ(num, 0);
    EXPECT_EQ(cpu.physicalNum(), 0);
}

// ===== logicalCpus =====

TEST_F(CpuInfo_UT, LogicalCpus_Empty_KeepsInfoEmpty)
{
    // Arrange
    CpuInfo cpu;
    QString info;
    // Act
    cpu.logicalCpus(info);
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(info.length(), 0);
}

TEST_F(CpuInfo_UT, LogicalCpus_WithPhysical_AppendsInfo)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 0, 0);
    cpu.m_MapPhysicalCpu[0].coreCpu(0).logicalCpu(0).setVendor("GenuineIntel");
    cpu.m_MapPhysicalCpu[0].coreCpu(0).logicalCpu(0).setCoreID(0);
    QString info;
    // Act
    cpu.logicalCpus(info);
    // Assert
    EXPECT_FALSE(info.isEmpty());
    EXPECT_NE(info.indexOf("vendor_id : GenuineIntel"), -1);
}

// ===== logicalCpu =====

TEST_F(CpuInfo_UT, LogicalCpu_Found_ReturnsReference)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    // Act
    LogicalCpu &logical = cpu.logicalCpu(5);
    // Assert
    EXPECT_EQ(logical.logicalID(), 5);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
}

TEST_F(CpuInfo_UT, LogicalCpu_NotFound_InsertsSentinel)
{
    // Arrange
    CpuInfo cpu;
    // Act
    LogicalCpu &logical = cpu.logicalCpu(7);
    // Assert
    EXPECT_EQ(logical.logicalID(), -1);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
    EXPECT_EQ(cpu.physicalNum(), 0);
}

TEST_F(CpuInfo_UT, LogicalCpu_SentinelSkipped_FallsBackToSentinel)
{
    // Arrange
    CpuInfo cpu;
    cpu.m_MapPhysicalCpu.insert(-1, PhysicalCpu(-1));
    // Act
    LogicalCpu &logical = cpu.logicalCpu(7);
    // Assert
    EXPECT_EQ(logical.logicalID(), -1);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
}

// ===== loadCpuInfo =====

TEST_F(CpuInfo_UT, LoadCpuInfo_FullTopology_RenumbersCores)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    installUnameStub(stub);
    vfsCpuTopology(0, 0, 0);   // 逻辑0 → 物理0 核心0
    vfsCpuTopology(1, 0, 2);   // 逻辑1 → 物理0 核心2（重编号后应为核心1）
    vfsCpuFreq(1, "800000", "2400000", "3600000");
    vfsCpuCacheIndex(0, 0, 2, "Unified", "512K", "0");
    vfsCpuFreq(0, "800000", "2400000", "3600000");
    vfsProcCpuinfo(QByteArray("processor : 0\nvendor_id : GENUINE\nmodel name : MZERO\n"
                              "flags : fpu vme\n\nprocessor : 1\nvendor_id : GENUINE\n"
                              "model name : MONE\nflags : fpu\n\n"));
    CpuInfo cpu;
    // Act
    const bool ok = cpu.loadCpuInfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.physicalNum(), 1);
    EXPECT_EQ(cpu.coreNum(), 2);
    EXPECT_EQ(cpu.logicalNum(), 2);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreNums(), QList<int>() << 0 << 1);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalCpu(0).modelName(), QString("MZERO"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(1).logicalCpu(1).modelName(), QString("MONE"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalCpu(0).vendor(), QString("GENUINE"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalCpu(0).minFreq(), QString("800MHz"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(1).logicalCpu(1).maxFreq(), QString("3600MHz"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalCpu(0).l2Cache(), QString("512K"));
    EXPECT_EQ(cpu.arch(), QString("x86_64"));
    QString info;
    cpu.logicalCpus(info);
    EXPECT_FALSE(info.isEmpty());
}

TEST_F(CpuInfo_UT, LoadCpuInfo_ProcCpuinfoMissing_ReturnsFalse)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    installUnameStub(stub);
    vfsCpuTopology(0, 0, 0);   // sysfs 正常，但 /proc/cpuinfo 未注册
    CpuInfo cpu;
    // Act
    const bool ok = cpu.loadCpuInfo();
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cpu.physicalNum(), 1);   // sysfs 数据保留，未因 /proc 失败被清除
    EXPECT_EQ(cpu.coreNum(), 1);
    EXPECT_EQ(cpu.logicalNum(), 1);
}

TEST_F(CpuInfo_UT, LoadCpuInfo_ProcOnly_SentinelOnly)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    installUnameStub(stub);
    vfsProcCpuinfo(QByteArray("processor : 0\nmodel name : SOLO\n"));   // 无 sysfs
    CpuInfo cpu;
    // Act
    const bool ok = cpu.loadCpuInfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.physicalNum(), 0);   // 哨兵(-1)被排除
    EXPECT_EQ(cpu.coreNum(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
}

TEST_F(CpuInfo_UT, LoadCpuInfo_EmptyProc_TrueEmptyState)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    installUnameStub(stub);
    vfsProcCpuinfo(QByteArray(""));   // 空 /proc/cpuinfo 且无 sysfs
    CpuInfo cpu;
    // Act
    const bool ok = cpu.loadCpuInfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.physicalNum(), 0);
    EXPECT_EQ(cpu.coreNum(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
}

// ===== readProcCpuinfo =====

TEST_F(CpuInfo_UT, ReadProcCpuinfo_OpenFailed_ReturnsFalse)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);   // VFS 为空 → /proc/cpuinfo 打不开
    CpuInfo cpu;
    // Act
    const bool ok = cpu.readProcCpuinfo();
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
}

TEST_F(CpuInfo_UT, ReadProcCpuinfo_ParsesStanzas_UpdatesLogical)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    vfsProcCpuinfo(QByteArray("processor : 5\npackage : 0\ncore : 3\n"
                              "model name : STANZA1\n\n"));
    // Act
    const bool ok = cpu.readProcCpuinfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(3).logicalCpu(5).modelName(), QString("STANZA1"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
}

TEST_F(CpuInfo_UT, ReadProcCpuinfo_EmptyContent_ReturnsTrue)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsProcCpuinfo(QByteArray(""));
    CpuInfo cpu;
    // Act
    const bool ok = cpu.readProcCpuinfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
}

TEST_F(CpuInfo_UT, ReadProcCpuinfo_OnlyEmptyStanzas_ReturnsTrue)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsProcCpuinfo(QByteArray("\n\n\n"));
    CpuInfo cpu;
    // Act
    const bool ok = cpu.readProcCpuinfo();
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
    EXPECT_EQ(cpu.physicalNum(), 0);
}

// ===== parseInfo =====

TEST_F(CpuInfo_UT, ParseInfo_Empty_ReturnsFalse)
{
    // Arrange
    CpuInfo cpu;
    // Act
    const bool ok = cpu.parseInfo(QString());
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
}

TEST_F(CpuInfo_UT, ParseInfo_NoProcessor_ReturnsFalse)
{
    // Arrange
    CpuInfo cpu;
    const QString stanza = "garbage line\nprocessor : 5 : extra\nmodel name : Intel\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
}

TEST_F(CpuInfo_UT, ParseInfo_PhysicalMissing_ReturnsFalse)
{
    // Arrange
    CpuInfo cpu;   // 表为空，物理 9 不存在
    const QString stanza = "processor : 5\npackage : 9\ncore : 0\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
}

TEST_F(CpuInfo_UT, ParseInfo_NoPhysicalCore_FallsBackToSentinel)
{
    // Arrange
    CpuInfo cpu;
    const QString stanza = "processor : 0\nmodel name : SOLO\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);   // 兜底哨兵 -1
    EXPECT_EQ(cpu.physicalNum(), 0);
    EXPECT_EQ(cpu.coreNum(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
}

TEST_F(CpuInfo_UT, ParseInfo_CoreMissing_UpdatesExistingLogical)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 1, 5);
    const QString stanza = "processor : 5\npackage : 0\ncore : 9\nmodel name : REMAPPED\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(1).logicalCpu(5).modelName(),
              QString("REMAPPED"));
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 1);
}

TEST_F(CpuInfo_UT, ParseInfo_LogicalMissingInCore_UpdatesOtherCoreLogical)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 1, 5);                      // 逻辑5 现存于核1
    cpu.m_MapPhysicalCpu[0].addCoreCpu(3, CoreCpu(3)); // 核3 存在但无逻辑
    const QString stanza = "processor : 5\npackage : 0\ncore : 3\nmodel name : REMAPPED\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(cpu.m_MapPhysicalCpu.value(0).coreCpu(1).logicalCpu(5).modelName(),
              QString("REMAPPED"));
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.value(0).coreIsExisted(3));
}

TEST_F(CpuInfo_UT, ParseInfo_CoreAndLogicalExisted_UpdatesFields)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    const QString stanza = "processor : 5\npackage : 0\ncore : 3\nvendor_id : GENUINE\n"
                           "model name : INTEL-I7\nmodel : 142\nflags : fpu vme\n"
                           "stepping : 9\ncpu family : 6\nbogomips : 5000.00\n";
    // Act
    const bool ok = cpu.parseInfo(stanza);
    // Assert
    EXPECT_TRUE(ok);
    LogicalCpu &logical = cpu.m_MapPhysicalCpu[0].coreCpu(3).logicalCpu(5);
    EXPECT_EQ(logical.modelName(), QString("INTEL-I7"));
    EXPECT_EQ(logical.vendor(), QString("GENUINE"));
    EXPECT_EQ(logical.model(), QString("142"));
    EXPECT_EQ(logical.flags(), QString("fpu vme"));
    EXPECT_EQ(logical.stepping(), QString("9"));
    EXPECT_EQ(logical.cpuFamliy(), QString("6"));
    EXPECT_EQ(logical.bogomips(), QString("5000.00"));
}

// ===== readSysCpu =====

TEST_F(CpuInfo_UT, ReadSysCpu_MultiCpu_BuildsTopology)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuTopology(0, 0, 0);
    vfsCpuTopology(1, 0, 2);
    vfsSetFile("/sys/devices/system/cpu/kernel_max", "31");   // 文件项，名称不匹配 → 跳过
    vfsSetDir("/sys/devices/system/cpu/cpufreq");             // 无数字目录 → 跳过
    CpuInfo cpu;
    // Act
    cpu.readSysCpu();
    // Assert
    EXPECT_EQ(cpu.physicalNum(), 1);
    EXPECT_EQ(cpu.coreNum(), 2);
    EXPECT_EQ(cpu.logicalNum(), 2);
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalIsExisted(0));
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.value(0).coreCpu(2).logicalIsExisted(1));
    EXPECT_FALSE(cpu.m_MapPhysicalCpu.value(0).coreIsExisted(1));
}

TEST_F(CpuInfo_UT, ReadSysCpu_EmptySysfs_NothingBuilt)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);   // VFS 为空
    CpuInfo cpu;
    // Act
    cpu.readSysCpu();
    // Assert
    EXPECT_EQ(cpu.physicalNum(), 0);
    EXPECT_EQ(cpu.coreNum(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
}

// ===== readSysCpuN =====

TEST_F(CpuInfo_UT, ReadSysCpuN_MissingPhysical_EarlyReturn)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);   // physical_package_id 不存在
    CpuInfo cpu;
    // Act
    cpu.readSysCpuN(0, "/sys/devices/system/cpu/cpu0");
    // Assert
    EXPECT_EQ(cpu.m_MapPhysicalCpu.size(), 0);
    EXPECT_EQ(cpu.physicalNum(), 0);
}

TEST_F(CpuInfo_UT, ReadSysCpuN_MissingSiblings_PhysicalOnly)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "0");
    CpuInfo cpu;
    // Act
    cpu.readSysCpuN(0, "/sys/devices/system/cpu/cpu0");
    // Assert
    EXPECT_EQ(cpu.physicalNum(), 1);
    EXPECT_EQ(cpu.coreNum(), 0);
    EXPECT_EQ(cpu.logicalNum(), 0);
}

TEST_F(CpuInfo_UT, ReadSysCpuN_CacheAndFreq_SetsAllFields)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    installUnameStub(stub);
    s_arch = "x86_64";
    vfsCpuTopology(0, 0, 0);
    vfsCpuCacheIndex(0, 0, 1, "Data", "32K", "0");
    vfsCpuCacheIndex(0, 1, 1, "Instruction", "32K", "0");
    vfsCpuCacheIndex(0, 2, 2, "Unified", "512K", "0");
    vfsCpuCacheIndex(0, 3, 3, "Unified", "8192K", "0");
    vfsCpuFreq(0, "800000", "2400000", "3600000");
    CpuInfo cpu;
    cpu.readCpuArchitecture();   // 先确定 m_Arch，readSysCpuN 会透传给逻辑核
    // Act
    cpu.readSysCpuN(3, "/sys/devices/system/cpu/cpu0");
    // Assert
    LogicalCpu &logical = cpu.m_MapPhysicalCpu[0].coreCpu(0).logicalCpu(3);
    EXPECT_EQ(logical.logicalID(), 3);
    EXPECT_EQ(logical.coreID(), 0);
    EXPECT_EQ(logical.physicalID(), 0);
    EXPECT_EQ(logical.arch(), QString("x86_64"));
    EXPECT_EQ(logical.l1dCache(), QString("32K"));
    EXPECT_EQ(logical.l1dSharedCpuList(), QString("0"));
    EXPECT_EQ(logical.l1iCache(), QString("32K"));
    EXPECT_EQ(logical.l1iSharedCpuList(), QString("0"));
    EXPECT_EQ(logical.l2Cache(), QString("512K"));
    EXPECT_EQ(logical.l3Cache(), QString("8192K"));
    EXPECT_EQ(logical.minFreq(), QString("800MHz"));
    EXPECT_EQ(logical.curFreq(), QString("2400MHz"));
    EXPECT_EQ(logical.maxFreq(), QString("3600MHz"));
}

TEST_F(CpuInfo_UT, ReadSysCpuN_NoCacheNoFreq_FieldsEmpty)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuTopology(0, 0, 0);   // 无 cache / cpufreq 目录
    CpuInfo cpu;
    // Act
    cpu.readSysCpuN(2, "/sys/devices/system/cpu/cpu0");
    // Assert
    LogicalCpu &logical = cpu.m_MapPhysicalCpu[0].coreCpu(0).logicalCpu(2);
    EXPECT_EQ(logical.logicalID(), 2);
    EXPECT_TRUE(logical.l1dCache().isEmpty());
    EXPECT_TRUE(logical.l2Cache().isEmpty());
    EXPECT_TRUE(logical.l3Cache().isEmpty());
    EXPECT_TRUE(logical.minFreq().isEmpty());
    EXPECT_TRUE(logical.curFreq().isEmpty());
    EXPECT_TRUE(logical.maxFreq().isEmpty());
}

TEST_F(CpuInfo_UT, ReadSysCpuN_SameCoreReused_TwoLogicals)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuTopology(0, 0, 0);
    CpuInfo cpu;
    // Act — 同一核路径读两次（超线程两逻辑同核）
    cpu.readSysCpuN(0, "/sys/devices/system/cpu/cpu0");
    cpu.readSysCpuN(1, "/sys/devices/system/cpu/cpu0");
    // Assert
    EXPECT_EQ(cpu.physicalNum(), 1);
    EXPECT_EQ(cpu.coreNum(), 1);
    EXPECT_EQ(cpu.logicalNum(), 2);
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalIsExisted(0));
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.value(0).coreCpu(0).logicalIsExisted(1));
}

// ===== readPhysicalID =====

TEST_F(CpuInfo_UT, ReadPhysicalID_MissingFile_Negative1)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);   // 文件未注册
    CpuInfo cpu;
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert
    EXPECT_EQ(value, -1);
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.isEmpty());
}

TEST_F(CpuInfo_UT, ReadPhysicalID_X86Numeric_ReturnsValue)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "4");
    CpuInfo cpu;
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert
    EXPECT_EQ(value, 4);
    EXPECT_NE(value, -1);   // 文件已读到，非 open 失败路径
}

TEST_F(CpuInfo_UT, ReadPhysicalID_X86Overflow_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "99999999999");
    CpuInfo cpu;
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert
    EXPECT_EQ(value, 0);   // toInt 溢出失败 → 0
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadPhysicalID_Sw64NonNumeric_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "n/a");
    CpuInfo cpu;
    cpu.m_Arch = "sw_64";
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert
    EXPECT_EQ(value, 0);   // 非数字 toInt → 0（透传，与 x86 溢出路径一致）
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadPhysicalID_Sw64Negative1_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "-1");
    CpuInfo cpu;
    cpu.m_Arch = "sw_64";
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert — sw_64 特判：字面 -1 按 0 返回（B2）
    EXPECT_EQ(value, 0);
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadPhysicalID_Sw64Numeric_Passthrough)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/physical_package_id", "12");
    CpuInfo cpu;
    cpu.m_Arch = "sw_64";
    // Act
    const int value = cpu.readPhysicalID("/sys/devices/system/cpu/cpu0/topology/physical_package_id");
    // Assert
    EXPECT_EQ(value, 12);
    EXPECT_NE(value, -1);
}

// ===== readCoreID =====

TEST_F(CpuInfo_UT, ReadCoreID_MissingFile_Negative1)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    CpuInfo cpu;
    // Act
    const int value = cpu.readCoreID("/sys/devices/system/cpu/cpu0/topology/core_id");
    // Assert
    EXPECT_EQ(value, -1);
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.isEmpty());
}

TEST_F(CpuInfo_UT, ReadCoreID_Numeric_ReturnsValue)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/core_id", "3");
    CpuInfo cpu;
    // Act
    const int value = cpu.readCoreID("/sys/devices/system/cpu/cpu0/topology/core_id");
    // Assert
    EXPECT_EQ(value, 3);
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadCoreID_EmptyContent_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/core_id", QByteArray(""));
    CpuInfo cpu;
    // Act
    const int value = cpu.readCoreID("/sys/devices/system/cpu/cpu0/topology/core_id");
    // Assert
    EXPECT_EQ(value, 0);
    EXPECT_NE(value, -1);   // 空内容仍走读取路径
}

TEST_F(CpuInfo_UT, ReadCoreID_NonNumeric_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/core_id", "abc");
    CpuInfo cpu;
    // Act
    const int value = cpu.readCoreID("/sys/devices/system/cpu/cpu0/topology/core_id");
    // Assert
    EXPECT_EQ(value, 0);
    EXPECT_NE(value, -1);
}

// ===== readThreadSiblingsListPath =====

TEST_F(CpuInfo_UT, ReadThreadSiblingsListPath_MissingFile_Negative1)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    CpuInfo cpu;
    // Act
    const int value = cpu.readThreadSiblingsListPath("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list");
    // Assert
    EXPECT_EQ(value, -1);
    EXPECT_TRUE(cpu.m_MapPhysicalCpu.isEmpty());
}

TEST_F(CpuInfo_UT, ReadThreadSiblingsListPath_Range_ReturnsFirst)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list", "0-1");
    CpuInfo cpu;
    // Act
    const int value = cpu.readThreadSiblingsListPath("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list");
    // Assert
    EXPECT_EQ(value, 0);
    EXPECT_NE(value, -1);   // 文件已读到，非 open 失败路径
}

TEST_F(CpuInfo_UT, ReadThreadSiblingsListPath_Single_ReturnsValue)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list", "2");
    CpuInfo cpu;
    // Act
    const int value = cpu.readThreadSiblingsListPath("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list");
    // Assert
    EXPECT_EQ(value, 2);
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadThreadSiblingsListPath_EmptyContent_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list", QByteArray(""));
    CpuInfo cpu;
    // Act
    const int value = cpu.readThreadSiblingsListPath("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list");
    // Assert
    EXPECT_EQ(value, 0);   // 空内容拆分出空段 → toInt 为 0
    EXPECT_NE(value, -1);
}

TEST_F(CpuInfo_UT, ReadThreadSiblingsListPath_NonNumeric_ReturnsZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetFile("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list", "abc");
    CpuInfo cpu;
    // Act
    const int value = cpu.readThreadSiblingsListPath("/sys/devices/system/cpu/cpu0/topology/thread_siblings_list");
    // Assert
    EXPECT_EQ(value, 0);
    EXPECT_NE(value, -1);
}

// ===== readCpuCache / readCpuCacheIndex =====

TEST_F(CpuInfo_UT, ReadCpuCache_IndexApplied_NonIndexSkipped)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuCacheIndex(0, 0, 2, "Unified", "512K", "0");
    vfsSetFile("/sys/devices/system/cpu/cpu0/cache/depth", "3");   // 非 index 项 → 跳过
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuCache("/sys/devices/system/cpu/cpu0/cache", lcpu);
    // Assert
    EXPECT_EQ(lcpu.l2Cache(), QString("512K"));
    EXPECT_TRUE(lcpu.l1dCache().isEmpty());
    EXPECT_TRUE(lcpu.l3Cache().isEmpty());
}

namespace {
enum CacheField { FIELD_L1D, FIELD_L1I, FIELD_L2, FIELD_L3, FIELD_L4 };

struct CacheCase {
    int level;
    const char *type;
    CacheField field;
};
}  // namespace

class CpuInfoCacheParam_UT : public CpuInfo_UT,
                             public ::testing::WithParamInterface<CacheCase>
{
};

TEST_P(CpuInfoCacheParam_UT, ReadCpuCacheIndex_Level_SetsExpectedCache)
{
    // Arrange
    const CacheCase param = GetParam();
    Stub stub;
    installVfsStubs(stub);
    vfsCpuCacheIndex(0, 0, param.level, param.type, "32K", "0");
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuCacheIndex("/sys/devices/system/cpu/cpu0/cache/index0", lcpu);
    // Assert
    const QString cache = "32K";
    const QString list = "0";
    switch (param.field) {
    case FIELD_L1D:
        EXPECT_EQ(lcpu.l1dCache(), cache);
        EXPECT_EQ(lcpu.l1dSharedCpuList(), list);
        EXPECT_TRUE(lcpu.l1iCache().isEmpty());
        EXPECT_TRUE(lcpu.l2Cache().isEmpty());
        break;
    case FIELD_L1I:
        EXPECT_EQ(lcpu.l1iCache(), cache);
        EXPECT_EQ(lcpu.l1iSharedCpuList(), list);
        EXPECT_TRUE(lcpu.l1dCache().isEmpty());
        EXPECT_TRUE(lcpu.l2Cache().isEmpty());
        break;
    case FIELD_L2:
        EXPECT_EQ(lcpu.l2Cache(), cache);
        EXPECT_EQ(lcpu.l2SharedCpuList(), list);
        EXPECT_TRUE(lcpu.l1dCache().isEmpty());
        EXPECT_TRUE(lcpu.l3Cache().isEmpty());
        break;
    case FIELD_L3:
        EXPECT_EQ(lcpu.l3Cache(), cache);
        EXPECT_EQ(lcpu.l3SharedCpuList(), list);
        EXPECT_TRUE(lcpu.l2Cache().isEmpty());
        break;
    case FIELD_L4:
        EXPECT_EQ(lcpu.l4Cache(), cache);
        EXPECT_EQ(lcpu.l4SharedCpuList(), list);
        EXPECT_TRUE(lcpu.l3Cache().isEmpty());
        break;
    }
}

INSTANTIATE_TEST_SUITE_P(Levels, CpuInfoCacheParam_UT,
                         ::testing::Values(CacheCase{1, "Data", FIELD_L1D},
                                           CacheCase{1, "Instruction", FIELD_L1I},
                                           CacheCase{2, "Unified", FIELD_L2},
                                           CacheCase{3, "Unified", FIELD_L3},
                                           CacheCase{4, "Unified", FIELD_L4}));

TEST_F(CpuInfo_UT, ReadCpuCacheIndex_LevelMissing_NothingSet)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsSetDir("/sys/devices/system/cpu/cpu0/cache");   // 目录存在但无 index 文件
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuCacheIndex("/sys/devices/system/cpu/cpu0/cache/index0", lcpu);
    // Assert
    EXPECT_EQ(lcpu.l1dCache(), QString());
    EXPECT_EQ(lcpu.l1iCache(), QString());
    EXPECT_EQ(lcpu.l2Cache(), QString());
    EXPECT_EQ(lcpu.l3Cache(), QString());
    EXPECT_EQ(lcpu.l4Cache(), QString());
}

TEST_F(CpuInfo_UT, ReadCpuCacheIndex_Level5_Ignored)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuCacheIndex(0, 0, 5, "Unified", "1M", "0");   // level 5 超出处理范围
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuCacheIndex("/sys/devices/system/cpu/cpu0/cache/index0", lcpu);
    // Assert
    EXPECT_EQ(lcpu.l1dCache(), QString());
    EXPECT_EQ(lcpu.l2Cache(), QString());
    EXPECT_EQ(lcpu.l3Cache(), QString());
    EXPECT_EQ(lcpu.l4Cache(), QString());
}

// ===== readCpuFreq =====

TEST_F(CpuInfo_UT, ReadCpuFreq_AllPresent_SetsMhz)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuFreq(0, "800000", "2400000", "3600000");
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuFreq("/sys/devices/system/cpu/cpu0/cpufreq", lcpu);
    // Assert
    EXPECT_EQ(lcpu.minFreq(), QString("800MHz"));
    EXPECT_EQ(lcpu.curFreq(), QString("2400MHz"));
    EXPECT_EQ(lcpu.maxFreq(), QString("3600MHz"));
}

TEST_F(CpuInfo_UT, ReadCpuFreq_FilesMissing_KeepsValues)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);   // 频率文件未注册
    CpuInfo cpu;
    LogicalCpu lcpu;
    lcpu.setMinFreq("KEEP1");
    lcpu.setCurFreq("KEEP2");
    lcpu.setMaxFreq("KEEP3");
    // Act
    cpu.readCpuFreq("/sys/devices/system/cpu/cpu9/cpufreq", lcpu);
    // Assert
    EXPECT_EQ(lcpu.minFreq(), QString("KEEP1"));
    EXPECT_EQ(lcpu.curFreq(), QString("KEEP2"));
    EXPECT_EQ(lcpu.maxFreq(), QString("KEEP3"));
}

TEST_F(CpuInfo_UT, ReadCpuFreq_ZeroKHz_RoundsToZero)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuFreq(0, "999", "1", "0");   // 不足 1000KHz → 整除为 0
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuFreq("/sys/devices/system/cpu/cpu0/cpufreq", lcpu);
    // Assert
    EXPECT_EQ(lcpu.minFreq(), QString("0MHz"));
    EXPECT_EQ(lcpu.curFreq(), QString("0MHz"));
    EXPECT_EQ(lcpu.maxFreq(), QString("0MHz"));
}

TEST_F(CpuInfo_UT, ReadCpuFreq_EmptyContent_ZeroMhz)
{
    // Arrange
    Stub stub;
    installVfsStubs(stub);
    vfsCpuFreq(0, "", "", "");
    CpuInfo cpu;
    LogicalCpu lcpu;
    // Act
    cpu.readCpuFreq("/sys/devices/system/cpu/cpu0/cpufreq", lcpu);
    // Assert
    EXPECT_EQ(lcpu.minFreq(), QString("0MHz"));
    EXPECT_EQ(lcpu.curFreq(), QString("0MHz"));
    EXPECT_EQ(lcpu.maxFreq(), QString("0MHz"));
}

// ===== setProcCpuinfo =====

TEST_F(CpuInfo_UT, SetProcCpuinfo_X86_SetsFields)
{
    // Arrange
    CpuInfo cpu;
    LogicalCpu lcpu;
    QMap<QString, QString> info;
    info.insert("flags", "fpu vme");
    info.insert("model", "142");
    info.insert("model name", "INTEL-I7");
    info.insert("vendor_id", "GENUINE");
    info.insert("stepping", "9");
    info.insert("cpu family", "6");
    info.insert("bogomips", "5000.00");
    // Act — m_Arch 非特判架构 → 常规分支
    cpu.setProcCpuinfo(lcpu, info);
    // Assert
    EXPECT_EQ(lcpu.flags(), QString("fpu vme"));
    EXPECT_EQ(lcpu.model(), QString("142"));
    EXPECT_EQ(lcpu.modelName(), QString("INTEL-I7"));
    EXPECT_EQ(lcpu.vendor(), QString("GENUINE"));
    EXPECT_EQ(lcpu.stepping(), QString("9"));
    EXPECT_EQ(lcpu.cpuFamliy(), QString("6"));
    EXPECT_EQ(lcpu.bogomips(), QString("5000.00"));
    EXPECT_TRUE(lcpu.curFreq().isEmpty());
}

TEST_F(CpuInfo_UT, SetProcCpuinfo_Mips64_SetsCpuMhzAndFeatures)
{
    // Arrange
    CpuInfo cpu;
    cpu.m_Arch = "mips64";
    LogicalCpu lcpu;
    QMap<QString, QString> info;
    info.insert("cpu mhz", "1200");
    info.insert("cpu model", "loongson-3a");
    info.insert("features", "mips-feature");
    // Act
    cpu.setProcCpuinfo(lcpu, info);
    // Assert
    EXPECT_EQ(lcpu.curFreq(), QString("1200"));
    EXPECT_EQ(lcpu.model(), QString("loongson-3a"));
    EXPECT_EQ(lcpu.flags(), QString("mips-feature"));   // flags 为空 → features 兜底
}

TEST_F(CpuInfo_UT, SetProcCpuinfo_Loongarch64_FeaturesFallback)
{
    // Arrange
    CpuInfo cpu;
    cpu.m_Arch = "loongarch64";
    LogicalCpu lcpu;
    lcpu.setFlags("pre-existing-flags");
    QMap<QString, QString> info;
    info.insert("cpu mhz", "1800");
    info.insert("cpu model", "3a5000");
    info.insert("features", "lsx lasx");
    // Act
    cpu.setProcCpuinfo(lcpu, info);
    // Assert — 源码先无条件 setFlags(mapInfo["flags"])（缺键→空），
    // 预置 flags 被清空后走 isEmpty→features 兜底，最终为 features
    EXPECT_EQ(lcpu.curFreq(), QString("1800"));
    EXPECT_EQ(lcpu.model(), QString("3a5000"));
    EXPECT_EQ(lcpu.flags(), QString("lsx lasx"));
}

// ===== diagPrintInfo =====

TEST_F(CpuInfo_UT, DiagPrintInfo_PopulatedMap_NoCrash)
{
    // Arrange
    CpuInfo cpu;
    seedLogicalCpu(cpu, 0, 3, 5);
    const int physBefore = cpu.physicalNum();
    const int logicalBefore = cpu.logicalNum();
    // Act
    EXPECT_NO_THROW(cpu.diagPrintInfo());
    // Assert
    EXPECT_EQ(cpu.physicalNum(), physBefore);
    EXPECT_EQ(cpu.logicalNum(), logicalBefore);
}
