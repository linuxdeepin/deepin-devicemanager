// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#define protected public

#include "securityutils.h"

#undef private
#undef protected

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class SecurityUtilsTest : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

// isValidModName

TEST_F(SecurityUtilsTest, isValidModName_validName_returnsTrue)
{
    // Arrange
    QString modName = "e1000e";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(result, true);
}

TEST_F(SecurityUtilsTest, isValidModName_validWithUnderscoreDash_returnsTrue)
{
    // Arrange
    QString modName = "my_module-1";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(result, true);
}

TEST_F(SecurityUtilsTest, isValidModName_emptyInput_returnsFalse)
{
    // Arrange
    QString modName = "";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(result, true);
}

TEST_F(SecurityUtilsTest, isValidModName_withSlash_returnsFalse)
{
    // Arrange
    QString modName = "evil/../module";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(modName.contains("/"), true);
}

TEST_F(SecurityUtilsTest, isValidModName_withDot_returnsFalse)
{
    // Arrange
    QString modName = "module.ko";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(modName.contains("."), true);
}

TEST_F(SecurityUtilsTest, isValidModName_withSpace_returnsFalse)
{
    // Arrange
    QString modName = "my module";

    // Act
    bool result = isValidModName(modName);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(modName.contains(" "), true);
}

// isPathWithinDirectory

TEST_F(SecurityUtilsTest, isPathWithinDirectory_insideDir_returnsTrue)
{
    // Arrange
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/subdir/file.txt";
    QDir().mkpath(tmpDir.path() + "/subdir");
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.close();

    // Act
    bool result = isPathWithinDirectory(filePath, tmpDir.path());

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(result, true);
}

TEST_F(SecurityUtilsTest, isPathWithinDirectory_outsideDir_returnsFalse)
{
    // Arrange
    QTemporaryDir tmpDir1;
    QTemporaryDir tmpDir2;
    ASSERT_TRUE(tmpDir1.isValid() && tmpDir2.isValid());
    QString filePath = tmpDir2.path() + "/file.txt";
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.close();

    // Act
    bool result = isPathWithinDirectory(filePath, tmpDir1.path());

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(result, true);
}

TEST_F(SecurityUtilsTest, isPathWithinDirectory_emptyPath_returnsFalse)
{
    // Arrange
    QString filePath = "";
    QString baseDir = QDir::tempPath();

    // Act
    bool result = isPathWithinDirectory(filePath, baseDir);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(filePath.isEmpty(), true);
}

TEST_F(SecurityUtilsTest, isPathWithinDirectory_emptyBaseDir_returnsFalse)
{
    // Arrange
    QString filePath = QDir::tempPath() + "/file.txt";
    QString baseDir = "";

    // Act
    bool result = isPathWithinDirectory(filePath, baseDir);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(baseDir.isEmpty(), true);
}

TEST_F(SecurityUtilsTest, isPathWithinDirectory_nonexistentPath_returnsFalse)
{
    // Arrange
    QString filePath = "/nonexistent/path/file.txt";
    QString baseDir = QDir::tempPath();

    // Act
    bool result = isPathWithinDirectory(filePath, baseDir);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(result, true);
}

// isSafeSysfsPath

TEST_F(SecurityUtilsTest, isSafeSysfsPath_validDevicesPath_returnsTrue)
{
    // Arrange
    QString path = "/devices/pci0000:00/0000:00:01.0";

    // Act
    bool result = isSafeSysfsPath(path);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(path.contains(".."), false);
}

TEST_F(SecurityUtilsTest, isSafeSysfsPath_withDotDot_returnsFalse)
{
    // Arrange
    QString path = "/devices/../../etc/passwd";

    // Act
    bool result = isSafeSysfsPath(path);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(path.contains(".."), true);
}

TEST_F(SecurityUtilsTest, isSafeSysfsPath_emptyInput_returnsFalse)
{
    // Arrange
    QString path = "";

    // Act
    bool result = isSafeSysfsPath(path);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(path.isEmpty(), true);
}

// validateModNameForInstall

TEST_F(SecurityUtilsTest, validateModNameForInstall_validName_returnsTrueNoError)
{
    // Arrange
    QString modName = "e1000e";
    QString kernelRelease = "5.10.0-amd64-desktop";
    QString errMsg;

    // Act
    bool result = validateModNameForInstall(modName, kernelRelease, errMsg);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(errMsg.isEmpty(), true);
}

TEST_F(SecurityUtilsTest, validateModNameForInstall_invalidModName_returnsFalseWithError)
{
    // Arrange
    QString modName = "../evil";
    QString kernelRelease = "5.10.0-amd64-desktop";
    QString errMsg;

    // Act
    bool result = validateModNameForInstall(modName, kernelRelease, errMsg);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(errMsg.isEmpty(), false);
}

TEST_F(SecurityUtilsTest, validateModNameForInstall_traversalModName_returnsFalse)
{
    // Arrange
    QString modName = "mod/../../../etc";
    QString kernelRelease = "5.10.0-amd64-desktop";
    QString errMsg;

    // Act
    bool result = validateModNameForInstall(modName, kernelRelease, errMsg);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(modName.contains("/"), true);
}

TEST_F(SecurityUtilsTest, validateModNameForInstall_emptyModName_returnsFalseWithError)
{
    // Arrange
    QString modName = "";
    QString kernelRelease = "5.10.0-amd64-desktop";
    QString errMsg;

    // Act
    bool result = validateModNameForInstall(modName, kernelRelease, errMsg);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(errMsg.isEmpty(), false);
    EXPECT_EQ(modName.isEmpty(), true);
}
