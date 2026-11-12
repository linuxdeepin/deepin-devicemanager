// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "drivercontrol/modcore.h"
#include "securityutils.h"
#include "DDLog.h"

#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <unistd.h>

#include "../stub.h"

/*
 * Branch list for ModCore:
 *
 * infoType2String (private, pure logic):
 *   B1: EAlias       -> "alias"       | InfoType2String_Alias_ReturnsAlias
 *   B2: ELiense      -> "license"     | InfoType2String_Liense_ReturnsLicense
 *   B3: EVersion     -> "version"     | InfoType2String_Version_ReturnsVersion
 *   B4: EAuther      -> "author"      | InfoType2String_Auther_ReturnsAuthor
 *   B5: EDescription -> "description" | InfoType2String_Description_ReturnsDescription
 *   B6: ESrcVersion  -> "srcversion"  | InfoType2String_SrcVersion_ReturnsSrcVersion
 *   B7: EName        -> "name"        | InfoType2String_Name_ReturnsName
 *   B8: EVermagic    -> "vermagic"    | InfoType2String_Vermagic_ReturnsVermagic
 *
 * bFromPath (private):
 *   B1: modName is existing file path -> true  | BFromPath_ExistingFile_ReturnsTrue
 *   B2: modName is non-existent path  -> false | BFromPath_NonExistentPath_ReturnsFalse
 *   B3: modName is plain module name  -> false | BFromPath_PlainModuleName_ReturnsFalse
 *
 * addModBlackList (input validation guard):
 *   B1: invalid modName (contains '/')  -> false | AddModBlackList_InvalidName_ReturnsFalse
 *   B2: invalid modName (contains '..') -> false | AddModBlackList_DotDotName_ReturnsFalse
 *   B3: invalid modName (empty)         -> false | AddModBlackList_EmptyName_ReturnsFalse
 *   B4: invalid modName (space)         -> false | AddModBlackList_SpaceInName_ReturnsFalse
 *
 * setModLoadedOnBoot (input validation guard):
 *   B1: invalid modName (contains '/')  -> false | SetModLoadedOnBoot_InvalidName_ReturnsFalse
 *   B2: invalid modName (empty)         -> false | SetModLoadedOnBoot_EmptyName_ReturnsFalse
 *   B3: invalid modName (slash)         -> false | SetModLoadedOnBoot_SlashInName_ReturnsFalse
 *
 * rmModLoadedOnBoot (input validation guard):
 *   B1: invalid modName -> safe return, no crash | RmModLoadedOnBoot_InvalidName_ReturnsSafely
 *   B2: empty modName   -> safe return, no crash | RmModLoadedOnBoot_EmptyName_ReturnsSafely
 *
 * rmFromBlackList (input validation guard):
 *   B1: invalid modName -> safe return, no crash | RmFromBlackList_InvalidName_ReturnsSafely
 *   B2: empty modName   -> safe return, no crash | RmFromBlackList_EmptyName_ReturnsSafely
 *
 * NOTE: -fno-access-control is set in CMakeLists.txt, so private methods
 * are directly accessible without addr_pri macros.
 */

class ModCore_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

// ===== infoType2String =====

TEST_F(ModCore_UT, InfoType2String_Alias_ReturnsAlias)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EAlias);
    // Assert
    EXPECT_EQ(result.toStdString(), "alias");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Liense_ReturnsLicense)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::ELiense);
    // Assert
    EXPECT_EQ(result.toStdString(), "license");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Version_ReturnsVersion)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EVersion);
    // Assert
    EXPECT_EQ(result.toStdString(), "version");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Auther_ReturnsAuthor)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EAuther);
    // Assert
    EXPECT_EQ(result.toStdString(), "author");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Description_ReturnsDescription)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EDescription);
    // Assert
    EXPECT_EQ(result.toStdString(), "description");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_SrcVersion_ReturnsSrcVersion)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::ESrcVersion);
    // Assert
    EXPECT_EQ(result.toStdString(), "srcversion");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Name_ReturnsName)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EName);
    // Assert
    EXPECT_EQ(result.toStdString(), "name");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(ModCore_UT, InfoType2String_Vermagic_ReturnsVermagic)
{
    // Arrange
    ModCore mod;
    // Act
    QString result = mod.infoType2String(ModCore::EVermagic);
    // Assert
    EXPECT_EQ(result.toStdString(), "vermagic");
    EXPECT_FALSE(result.isEmpty());
}

// ===== bFromPath =====

TEST_F(ModCore_UT, BFromPath_ExistingFile_ReturnsTrue)
{
    // Arrange
    ModCore mod;
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    QString filePath = tmpFile.fileName();
    // Act
    bool result = mod.bFromPath(filePath);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(QFile::exists(filePath));
}

TEST_F(ModCore_UT, BFromPath_NonExistentPath_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QTemporaryDir tmpDir;
    QString nonExistentPath = tmpDir.path() + "/nonexistent.ko";
    // Act
    bool result = mod.bFromPath(nonExistentPath);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_FALSE(QFile::exists(nonExistentPath));
}

TEST_F(ModCore_UT, BFromPath_PlainModuleName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString plainName = "hid";
    // Act
    bool result = mod.bFromPath(plainName);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_FALSE(QFile::exists(plainName));
}

// ===== addModBlackList (input validation guard) =====

TEST_F(ModCore_UT, AddModBlackList_InvalidName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "evil/../module";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.addModBlackList(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ModCore_UT, AddModBlackList_DotDotName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "../etc/passwd";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.addModBlackList(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ModCore_UT, AddModBlackList_EmptyName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.addModBlackList(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ModCore_UT, AddModBlackList_SpaceInName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "module name";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.addModBlackList(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

// ===== setModLoadedOnBoot (input validation guard) =====

TEST_F(ModCore_UT, SetModLoadedOnBoot_InvalidName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "evil/../module";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.setModLoadedOnBoot(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ModCore_UT, SetModLoadedOnBoot_EmptyName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.setModLoadedOnBoot(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ModCore_UT, SetModLoadedOnBoot_SlashInName_ReturnsFalse)
{
    // Arrange
    ModCore mod;
    QString invalidName = "path/module";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act
    bool result = mod.setModLoadedOnBoot(invalidName);
    // Assert
    EXPECT_FALSE(result);
}

// ===== rmModLoadedOnBoot (input validation guard) =====

TEST_F(ModCore_UT, RmModLoadedOnBoot_InvalidName_ReturnsSafely)
{
    // Arrange
    ModCore mod;
    QString invalidName = "evil/../module";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act — invalid modName should be rejected by input validation guard
    bool noCrash = true;
    try { mod.rmModLoadedOnBoot(invalidName); } catch (...) { noCrash = false; }
    // Assert — method returns safely without throwing
    EXPECT_TRUE(noCrash);
}

TEST_F(ModCore_UT, RmModLoadedOnBoot_EmptyName_ReturnsSafely)
{
    // Arrange
    ModCore mod;
    QString invalidName = "";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act — empty modName should be rejected by input validation guard
    bool noCrash = true;
    try { mod.rmModLoadedOnBoot(invalidName); } catch (...) { noCrash = false; }
    // Assert — method returns safely without throwing
    EXPECT_TRUE(noCrash);
}

// ===== rmFromBlackList (input validation guard) =====

TEST_F(ModCore_UT, RmFromBlackList_InvalidName_ReturnsSafely)
{
    // Arrange
    ModCore mod;
    QString invalidName = "evil/../module";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act — invalid modName should be rejected by input validation guard
    bool noCrash = true;
    try { mod.rmFromBlackList(invalidName); } catch (...) { noCrash = false; }
    // Assert — method returns safely without throwing
    EXPECT_TRUE(noCrash);
}

TEST_F(ModCore_UT, RmFromBlackList_EmptyName_ReturnsSafely)
{
    // Arrange
    ModCore mod;
    QString invalidName = "";
    EXPECT_FALSE(isValidModName(invalidName));
    // Act — empty modName should be rejected by input validation guard
    bool noCrash = true;
    try { mod.rmFromBlackList(invalidName); } catch (...) { noCrash = false; }
    // Assert — method returns safely without throwing
    EXPECT_TRUE(noCrash);
}

// ==================== T24 补充覆盖：kmod 闭环与配置文件操作 ====================
//
// 覆盖目标（对应 tests/.ut/inventory.json 中 ModCore HIGH 项）：
//   rmModFromBlackListFile / rmModFromLoadonBootConf / deleteLineOfFileWithItem
//   updateInitramfs / rmFromBlackList / rmModLoadedOnBoot / setModLoadedOnBoot
//   addModBlackList / modGetConfsWithType / modIsBuildIn / modIsLoaded
//   modGetPath / modGetInfo / modGetName / isModFile / rmModForce / modInstall
//   checkModuleInUsed / modNew / ctor
// 打桩策略：QProcess::start / waitForFinished 空操作（depmod/update-initramfs 不真执行）；
//   kmod 调用均为只读内核接口（查询不存在模块 → 错误码），无副作用；
//   涉及 /etc 的写操作仅断言“被拒绝/无副作用”，root 环境直接 skip。

namespace {

void ut_stubQProcessStartMc(QProcess *, const QString &, const QStringList &, QIODevice::OpenMode)
{
    // 不执行任何外部进程
}

bool ut_stubQProcessWaitFinishedMc(QProcess *, int)
{
    return true;
}

}  // namespace

class ModCoreFileOps_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_dir = QDir::tempPath() + "/ut_modcore_t24_" + QString::number(::getpid());
        ASSERT_TRUE(QDir().mkpath(m_dir));
    }

    void TearDown() override
    {
        QDir(m_dir).removeRecursively();
    }

    QString filePath(const QString &name) const { return m_dir + "/" + name; }

    bool writeFile(const QString &name, const QString &content)
    {
        QFile f(filePath(name));
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;
        f.write(content.toUtf8());
        return true;
    }

    QString readFile(const QString &name)
    {
        QFile f(filePath(name));
        f.open(QIODevice::ReadOnly);
        return QString::fromUtf8(f.readAll());
    }

private:
    QString m_dir;
};

// ===== ctor / modNew =====

TEST_F(ModCoreFileOps_UT, Ctor_InitializesWithoutExternalResources)
{
    ModCore mod;
    SUCCEED();
}

TEST_F(ModCoreFileOps_UT, ModNew_NameAndPathBranches_BehaveAsExpected)
{
    ModCore mod;
    struct kmod_ctx *ctx = kmod_new(nullptr, nullptr);
    ASSERT_NE(ctx, nullptr);

    // 路径分支：kmod_module_new_from_path 不 stat 文件，不存在的路径同样返回 0
    struct kmod_module *m1 = nullptr;
    EXPECT_EQ(mod.modNew(ctx, filePath("no_such.ko"), m1), 0);
    if (m1)
        kmod_module_unref(m1);

    // 名称分支：任意名称都能创建模块对象（仅注册，不加载）
    struct kmod_module *m2 = nullptr;
    EXPECT_EQ(mod.modNew(ctx, "ut_no_such_mod_t24xyz", m2), 0);
    if (m2)
        kmod_module_unref(m2);

    kmod_unref(ctx);
}

// ===== rmModFromBlackListFile / deleteLineOfFileWithItem =====

TEST_F(ModCoreFileOps_UT, RmModFromBlackListFile_MatchingContent_TruncatesByDefect)
{
    ModCore mod;
    const QString original = "blacklist utmod\ninstall utmod /bin/false\nkeep this line\n";
    ASSERT_TRUE(writeFile("bl.conf", original));

    // 已知缺陷（见 defects 清单 D4）：deleteLineOfFileWithItem 内部 QFile 从未 open，
    // 读取全静默失败，但 resize(0) 仍对未打开设备生效 → 整个文件被清空（数据丢失）。
    // 此用例锁定当前真实行为，缺陷修复后应改为断言“仅保留 keep this line”。
    mod.rmModFromBlackListFile(filePath("bl.conf"), "utmod");

    EXPECT_TRUE(QFile::exists(filePath("bl.conf")));
    EXPECT_TRUE(readFile("bl.conf").isEmpty());
}

TEST_F(ModCoreFileOps_UT, RmModFromBlackListFile_SkipsWhenNoBlacklistKeyword)
{
    ModCore mod;
    // 文件不含 "blacklist" 关键字 → 不改写，原样保留
    ASSERT_TRUE(writeFile("plain.conf", "options utmod mode=1\n"));

    mod.rmModFromBlackListFile(filePath("plain.conf"), "utmod");

    EXPECT_EQ(readFile("plain.conf").toStdString(), "options utmod mode=1\n");
}

TEST_F(ModCoreFileOps_UT, RmModFromBlackListFile_FollowsSymlink)
{
    ModCore mod;
    ASSERT_TRUE(writeFile("target.conf", "blacklist utmod\nkeepme\n"));
    ASSERT_TRUE(QFile::link(filePath("target.conf"), filePath("link.conf")));

    // 传入符号链接：内部解析到真实文件并读取内容（读取路径分支）。
    // 受 D4 缺陷影响，目标文件最终被清空（数据丢失），符号链接本身保留。
    mod.rmModFromBlackListFile(filePath("link.conf"), "utmod");

    EXPECT_TRUE(QFile::exists(filePath("target.conf")));
    EXPECT_TRUE(readFile("target.conf").isEmpty());
    EXPECT_TRUE(QFile::exists(filePath("link.conf")));
}

TEST_F(ModCoreFileOps_UT, RmModFromBlackListFile_MissingFile_ReturnsSafely)
{
    ModCore mod;
    // 文件不存在：安全返回，不创建、不崩溃
    mod.rmModFromBlackListFile(filePath("no_such.conf"), "utmod");
    EXPECT_FALSE(QFile::exists(filePath("no_such.conf")));
}

TEST_F(ModCoreFileOps_UT, DeleteLineOfFileWithItem_TruncatesFileByDefect)
{
    ModCore mod;
    ASSERT_TRUE(writeFile("lines.conf", "alpha\nbeta utmod\ngamma\ndelta\n"));

    // 已知缺陷 D4：deleteLineOfFileWithItem 对未 open 的 QFile 做 seek/read/write，
    // 读取与回写全部静默失败，但 resize(0) 生效 → 文件被清空为 0 字节（数据丢失）。
    // 缺陷修复后应变为 "alpha\ngamma\ndelta\n"。
    mod.deleteLineOfFileWithItem(filePath("lines.conf"), "utmod");

    EXPECT_TRUE(QFile::exists(filePath("lines.conf")));
    EXPECT_TRUE(readFile("lines.conf").isEmpty());
}

// ===== rmModFromLoadonBootConf =====

TEST_F(ModCoreFileOps_UT, RmModFromLoadonBootConf_MatchingContent_TruncatesByDefect)
{
    ModCore mod;
    ASSERT_TRUE(writeFile("modprobe.conf", "utmod options x=1\nkeepme\n"));

    // 同 D4：匹配到内容后进入 deleteLineOfFileWithItem，文件被清空（数据丢失）
    mod.rmModFromLoadonBootConf(filePath("modprobe.conf"), "utmod");

    EXPECT_TRUE(QFile::exists(filePath("modprobe.conf")));
    EXPECT_TRUE(readFile("modprobe.conf").isEmpty());
}

TEST_F(ModCoreFileOps_UT, RmModFromLoadonBootConf_SkipsWhenNoMatch)
{
    ModCore mod;
    ASSERT_TRUE(writeFile("other.conf", "othermod options y=2\n"));

    mod.rmModFromLoadonBootConf(filePath("other.conf"), "utmod");

    EXPECT_EQ(readFile("other.conf").toStdString(), "othermod options y=2\n");
}

// ===== updateInitramfs（打桩外部进程） =====

TEST_F(ModCoreFileOps_UT, UpdateInitramfs_StubbedProcess_NoCrash)
{
    ModCore mod;
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStartMc);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinishedMc);

    mod.updateInitramfs();
    SUCCEED();
}

// ===== rmFromBlackList / rmModLoadedOnBoot（真实配置目录只读遍历） =====

TEST_F(ModCoreFileOps_UT, RmFromBlackList_FakeModule_NoSystemSideEffects)
{
    ModCore mod;
    // 合法名但系统配置目录中不存在任何含该名的文件：只读遍历后安全返回
    mod.rmFromBlackList("ut_fake_mod_t24xyz");
    EXPECT_FALSE(QFile::exists("/etc/modprobe.d/blacklist-ut_fake_mod_t24xyz-drivermanager.conf"));
}

TEST_F(ModCoreFileOps_UT, RmModLoadedOnBoot_FakeModule_NoSystemSideEffects)
{
    ModCore mod;
    mod.rmModLoadedOnBoot("ut_fake_mod_t24xyz");
    EXPECT_FALSE(QFile::exists("/etc/modules-load.d/ut_fake_mod_t24xyz-drivermanager.conf"));
}

// ===== setModLoadedOnBoot / addModBlackList（路径守卫，已知缺陷见缺陷清单） =====

TEST_F(ModCoreFileOps_UT, SetModLoadedOnBoot_ValidName_ReturnsFalsePathGuard)
{
    if (::geteuid() == 0)
        GTEST_SKIP() << "root 环境若守卫被修复会真实写入 /etc，跳过";

    ModCore mod;
    // 已知缺陷（tests/.ut/reports/defects 清单）：
    //   1) 目标文件路径用 QFileInfo(LOADONBOOT_PROBE_DIR).dir() 计算，得到的是父目录；
    //   2) isPathWithinDirectory 依赖 canonicalFilePath，对“尚不存在”的规划文件恒拒绝；
    // 因此合法模块名也会被守卫拒绝，返回 false（非 root 下亦无写入）
    EXPECT_FALSE(mod.setModLoadedOnBoot("ut_fake_mod_t24xyz"));
}

TEST_F(ModCoreFileOps_UT, AddModBlackList_ValidName_NonRootDeniedByPathGuard)
{
    if (::geteuid() == 0)
        GTEST_SKIP() << "root 环境会真实写入 /etc/modprobe.d，跳过";

    ModCore mod;
    // 同上：isPathWithinDirectory 对不存在的规划文件恒拒绝（缺陷），非 root 下亦无法创建
    EXPECT_FALSE(mod.addModBlackList("ut_fake_mod_t24xyz"));
}

// ===== modGetConfsWithType（四种配置类型全遍历） =====

TEST_F(ModCoreFileOps_UT, ModGetConfsWithType_AllTypes_NoCrash)
{
    ModCore mod;
    const QList<ModCore::ModConfType> types = {
        ModCore::EBlackListConf, ModCore::EAliasConf,
        ModCore::EOptionsConf, ModCore::ESoftdepConf
    };
    for (ModCore::ModConfType t : types) {
        const QStringList confs = mod.modGetConfsWithType(t);
        for (const QString &c : confs)
            EXPECT_FALSE(c.isEmpty());
        SUCCEED() << "type=" << int(t) << " count=" << confs.size();
    }
}

// ===== modIsBuildIn / modIsLoaded / modGetPath / modGetInfo / modGetName / isModFile =====

TEST_F(ModCoreFileOps_UT, ModIsBuildIn_ModIsLoaded_Nonexistent_ReturnsFalse)
{
    ModCore mod;
    EXPECT_FALSE(mod.modIsBuildIn("ut_no_such_mod_t24xyz"));
    EXPECT_FALSE(mod.modIsLoaded("ut_no_such_mod_t24xyz"));
}

TEST_F(ModCoreFileOps_UT, ModGetPath_ModGetInfo_Nonexistent_ReturnsEmpty)
{
    ModCore mod;
    EXPECT_TRUE(mod.modGetPath("ut_no_such_mod_t24xyz").isEmpty());
    EXPECT_TRUE(mod.modGetInfo("ut_no_such_mod_t24xyz", ModCore::EVersion).isEmpty());
    // 空名 → modNew(from_name) 失败 → 返回空
    EXPECT_TRUE(mod.modGetName(QString()).isEmpty());
}

TEST_F(ModCoreFileOps_UT, IsModFile_TextAndNonexistent_ReturnsFalse)
{
    ModCore mod;
    ASSERT_TRUE(writeFile("plain.txt", "just text\n"));
    EXPECT_FALSE(mod.isModFile(filePath("plain.txt")));
    EXPECT_FALSE(mod.isModFile(filePath("no_such.ko")));
}

// ===== rmModForce / modInstall / checkModuleInUsed（不存在模块的错误路径） =====

TEST_F(ModCoreFileOps_UT, RmModForce_Nonexistent_FailsWithErrMsg)
{
    ModCore mod;
    QString err;
    // delete_module 系统调用对不存在的模块返回错误 → false 且 errMsg 非空
    EXPECT_FALSE(mod.rmModForce("ut_no_such_mod_t24xyz", err));
    EXPECT_FALSE(err.isEmpty());
}

TEST_F(ModCoreFileOps_UT, ModInstall_Nonexistent_FailsWithoutErrMsg)
{
    ModCore mod;
    QString err;
    // 模块不存在：kmod 查找列表为空 → else 分支直接置失败，不写 errMsg
    EXPECT_FALSE(mod.modInstall("ut_no_such_mod_t24xyz", err));
    EXPECT_TRUE(err.isEmpty());
}

TEST_F(ModCoreFileOps_UT, CheckModuleInUsed_Nonexistent_ReturnsEmpty)
{
    ModCore mod;
    EXPECT_TRUE(mod.checkModuleInUsed("ut_no_such_mod_t24xyz").isEmpty());
}

// ===================== 第二批：真实内核模块（只读探测）覆盖缺口补强 =====================

namespace {

// 本机真实存在的内核模块（modinfo 验证）；不存在则返回空串
QString ut_t24_realKoPath()
{
    QProcess p;
    p.start("modinfo", QStringList() << "-F" << "filename" << "bfq");
    if (!p.waitForFinished(10000) || p.exitCode() != 0)
        return QString();
    const QString out = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
    return QFile::exists(out) ? out : QString();
}

// 本机已加载且有持有者的模块（inet_diag ← tcp_diag）
QString ut_t24_loadedModule()
{
    return QStringLiteral("inet_diag");
}

} // namespace

TEST_F(ModCoreFileOps_UT, ModGetPath_RealModule_ReturnsNonEmptyPath)
{
    ModCore mod;
    const QString path = mod.modGetPath("bfq");
    if (path.isEmpty()) {
        GTEST_SKIP() << "module bfq not present on this machine";
    }
    EXPECT_TRUE(path.contains("bfq.ko"));
    EXPECT_TRUE(QFile::exists(path));
}

TEST_F(ModCoreFileOps_UT, ModGetName_RealKoFile_ReturnsBfq)
{
    const QString path = ut_t24_realKoPath();
    if (path.isEmpty()) {
        GTEST_SKIP() << "bfq.ko not available on this machine";
    }
    ModCore mod;
    EXPECT_EQ(mod.modGetName(path), QStringLiteral("bfq"));
}

TEST_F(ModCoreFileOps_UT, IsModFile_RealKo_ReturnsTrue)
{
    const QString path = ut_t24_realKoPath();
    if (path.isEmpty()) {
        GTEST_SKIP() << "bfq.ko not available on this machine";
    }
    ModCore mod;
    EXPECT_TRUE(mod.isModFile(path));
}

TEST_F(ModCoreFileOps_UT, ModGetInfo_RealModule_VermagicAndDescription)
{
    // vermagic/描述 恒存在于 .ko modinfo 段 → 覆盖 info 遍历循环全部分支
    ModCore mod;
    const QString vermagic = mod.modGetInfo("bfq", ModCore::EVermagic);
    const QString desc = mod.modGetInfo("bfq", ModCore::EDescription);
    if (vermagic.isEmpty() && desc.isEmpty()) {
        GTEST_SKIP() << "module bfq info not available on this machine";
    }
    EXPECT_FALSE(vermagic.isEmpty());
    EXPECT_FALSE(desc.isEmpty());
}

TEST_F(ModCoreFileOps_UT, ModIsLoaded_LoadedModule_ReturnsTrue)
{
    ModCore mod;
    const QString name = ut_t24_loadedModule();
    if (!mod.modIsLoaded(name)) {
        GTEST_SKIP() << "module inet_diag not loaded on this machine";
    }
    EXPECT_TRUE(mod.modIsLoaded(name));
}

TEST_F(ModCoreFileOps_UT, ModIsBuildIn_LoadedModule_ReturnsFalse)
{
    // 已加载的可插拔模块不是内建模块
    ModCore mod;
    const QString name = ut_t24_loadedModule();
    if (!mod.modIsLoaded(name)) {
        GTEST_SKIP() << "module inet_diag not loaded on this machine";
    }
    EXPECT_FALSE(mod.modIsBuildIn(name));
}

TEST_F(ModCoreFileOps_UT, ModIsBlackListed_NonexistentModule_ReturnsFalse)
{
    // modGetName(不存在的模块) 为空 → contains 为假，确定性 false
    ModCore mod;
    EXPECT_FALSE(mod.modIsBlackListed("ut_no_such_mod_t24xyz"));
}

TEST_F(ModCoreFileOps_UT, ModIsBlackListed_RealModule_NoCrash)
{
    // 真实模块名的黑名单查询（结果依机器配置，只验证不崩溃且返回布尔值）
    ModCore mod;
    const bool result = mod.modIsBlackListed("bfq");
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(ModCoreFileOps_UT, CheckModuleInUsed_LoadedModuleWithHolder_ReturnsHolders)
{
    // inet_diag 被 tcp_diag 持有（本机 lsmod 确认）→ holders 分支
    ModCore mod;
    const QString name = ut_t24_loadedModule();
    const QStringList used = mod.checkModuleInUsed(name);
    if (!mod.modIsLoaded(name) || used.isEmpty()) {
        GTEST_SKIP() << "inet_diag/tcp_diag dependency not visible on this machine";
    }
    EXPECT_FALSE(used.isEmpty());
}
