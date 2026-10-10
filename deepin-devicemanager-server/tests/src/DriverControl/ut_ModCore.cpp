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
