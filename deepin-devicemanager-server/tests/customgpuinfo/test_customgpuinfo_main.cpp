// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | getGpuBaseInfo | mid | complexity:5 | 2 | 4 |
 * | getGpuMemInfoForFTDTM | high | complexity:7,transitive_loop_depth:3 | 4 | 8 |
 * | main | high | transitive_loop_depth:4 | 4 | 4 |
 */

// customgpuinfo/main.cpp 自带 main()，无法并入共享测试二进制；
// 此处将 main 宏替换后直接被测源文件，主函数以 custom_gpuinfo_original_main 名称参与测试
#include <QFile>
#include <QProcess>
#include <QMap>
#include <QRegularExpression>

#define main custom_gpuinfo_original_main
#include "../../customgpuinfo/main.cpp"
#undef main

#include <gtest/gtest.h>

#include <sstream>

#include "stubext.h"

using namespace stub_ext;

namespace {
constexpr int kArgcOne = 1;

// hwinfo/glxinfo 均经 QProcess 探测，统一打桩
void stubQProcessOutput(stub_ext::StubExt &stub, const QByteArray &output, bool finished = true)
{
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {});
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [finished](QProcess *, int) -> bool { return finished; });
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [output](QProcess *) -> QByteArray { return output; });
}

// /sys/kernel/debug/gc/total_mem 文件读取打桩（QFile::open + QIODevice::readAll）
// 两者均为 Qt 虚函数，成员指针地址被 Itanium ABI 编码为虚表索引，直接 addrof 会段错误；
// 且技能规则禁止对 Qt 内置类使用 VADDR。故按 mangled name 取导出符号地址后打桩。
extern "C" bool _ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE(
    QFile *self, QIODevice::OpenMode mode);
extern "C" QByteArray _ZN9QIODevice7readAllEv(QIODevice *self);

void stubFileContent(stub_ext::StubExt &stub, const QByteArray &content, bool openOk = true)
{
    stub.set_lamda(&_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE,
                   [openOk](QFile *, QIODevice::OpenMode) -> bool { return openOk; });
    stub.set_lamda(&_ZN9QIODevice7readAllEv,
                   [content](QIODevice *) -> QByteArray { return content; });
}
} // namespace

class CustomGpuInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ---------- getGpuBaseInfo (customgpuinfo/main.cpp:21-50) ----------
TEST_F(CustomGpuInfoTest, GetGpuBaseInfo_ProcessNotFinished_ReturnsFalse)
{
    // Arrange
    stubQProcessOutput(stub, QByteArray(), false);

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_FALSE(getGpuBaseInfo(mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());   // 进程未结束，不产出任何字段
}

TEST_F(CustomGpuInfoTest, GetGpuBaseInfo_RendererAndVendorLines_ParsesAllFields)
{
    // Arrange
    QByteArray output =
        "OpenGL core profile version string: 4.6\n"
        "OpenGL vendor string: Intel\n"
        "OpenGL renderer string: Mesa Intel(R) UHD Graphics\n"
        "OpenGL version string: 4.6 (Compatibility Profile)\n";
    stubQProcessOutput(stub, output);

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_TRUE(getGpuBaseInfo(mapInfo));

    EXPECT_EQ("Intel", mapInfo[kVendor]);
    EXPECT_EQ("Mesa Intel(R) UHD Graphics", mapInfo[kName]);
    EXPECT_EQ("Mesa Intel(R) UHD Graphics", mapInfo[kModel]);
}

// 边界：无可匹配行时仍返回 true，mapInfo 保持为空
TEST_F(CustomGpuInfoTest, GetGpuBaseInfo_NoMatchingLines_ReturnsTrueWithEmptyMap)
{
    // Arrange
    stubQProcessOutput(stub, QByteArray("some plain text\nanother line\n"));

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_TRUE(getGpuBaseInfo(mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());
}

// 边界：值列包含冒号时仅截取第一个冒号之前作为键
TEST_F(CustomGpuInfoTest, GetGpuBaseInfo_ValueWithColons_ParsesFirstPairOnly)
{
    // Arrange
    QByteArray output = "OpenGL renderer string: Adapter: Model: X\n";
    stubQProcessOutput(stub, output);

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_TRUE(getGpuBaseInfo(mapInfo));
    EXPECT_EQ("Adapter: Model: X", mapInfo[kName]);
}

// ---------- getGpuMemInfoForFTDTM (customgpuinfo/main.cpp:52-96) ----------
TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_OpenFails_ReturnsFalse)
{
    // Arrange
    stubFileContent(stub, QByteArray(), false);

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_FALSE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());   // 文件打开失败，不产出内存信息
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_EmptyContent_ReturnsFalse)
{
    // Arrange
    stubFileContent(stub, QByteArray());

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_FALSE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());   // 空文件内容不产出内存信息
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_NoMemPattern_ReturnsFalse)
{
    // Arrange
    stubFileContent(stub, QByteArray("no numeric memory info here"));

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_FALSE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());   // 无匹配数值格式，不产出内存信息
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_MBValue_ConvertsToGB)
{
    // Arrange
    stubFileContent(stub, QByteArray("Total: 512 (MB)\n"));

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_TRUE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_EQ("0.50GB", mapInfo[kGraphicsMemory]);
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_GBValue_KeptAsGB)
{
    // Arrange
    stubFileContent(stub, QByteArray("Total: 8 GB\n"));

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_TRUE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_EQ("8.00GB", mapInfo[kGraphicsMemory]);
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_KBValue_ConvertsToGB)
{
    // Arrange
    stubFileContent(stub, QByteArray("Total: 1048576 KB\n"));

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_TRUE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_EQ("1.00GB", mapInfo[kGraphicsMemory]);
}

TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_BytesValue_ConvertsToGB)
{
    // Arrange
    stubFileContent(stub, QByteArray("Total: 1073741824 B\n"));

    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_TRUE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_EQ("1.00GB", mapInfo[kGraphicsMemory]);
}

// 边界：带小数的 MB 值保留两位小数
TEST_F(CustomGpuInfoTest, GetGpuMemInfoForFTDTM_DecimalMB_ConvertsPrecisely)
{
    // Arrange
    stubFileContent(stub, QByteArray("1536.5 MB\n"));

    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_TRUE(getGpuMemInfoForFTDTM(mapInfo));
    EXPECT_EQ("1.50GB", mapInfo[kGraphicsMemory]);
}

// ---------- main (customgpuinfo/main.cpp:98-108) ----------
// 分支：显存信息获取成功 -> 输出并返回 0
TEST_F(CustomGpuInfoTest, Main_GetGpuMemInfoSucceeds_ReturnsZero)
{
    // Arrange
    stub.set_lamda(static_cast<bool (*)(QMap<QString, QString> &)>(getGpuMemInfoForFTDTM),
                   [](QMap<QString, QString> &mapInfo) -> bool {
                       mapInfo.insert(kGraphicsMemory, "8.00GB");
                       return true;
                   });
    char arg0[] = "customgpuinfo";
    char *argv[] = { arg0, nullptr };
    std::ostringstream oss;
    std::streambuf *oldBuf = std::cout.rdbuf(oss.rdbuf());
    // Act
    int ret = custom_gpuinfo_original_main(kArgcOne, argv);
    std::cout.rdbuf(oldBuf);
    // Assert
    EXPECT_EQ(0, ret);
    EXPECT_NE(std::string::npos, oss.str().find("8.00GB"));   // 成功路径输出内存信息
}

// 分支：显存信息获取失败 -> 返回 1
TEST_F(CustomGpuInfoTest, Main_GetGpuMemInfoFails_ReturnsOne)
{
    // Arrange
    stub.set_lamda(static_cast<bool (*)(QMap<QString, QString> &)>(getGpuMemInfoForFTDTM),
                   [](QMap<QString, QString> &) -> bool { return false; });
    char arg0[] = "customgpuinfo";
    char *argv[] = { arg0, nullptr };
    std::ostringstream oss;
    std::streambuf *oldBuf = std::cout.rdbuf(oss.rdbuf());
    // Act
    int ret = custom_gpuinfo_original_main(kArgcOne, argv);
    std::cout.rdbuf(oldBuf);
    // Assert
    EXPECT_EQ(1, ret);
    EXPECT_TRUE(oss.str().empty());   // 失败路径不打印内存信息
}

TEST_F(CustomGpuInfoTest, Main_EmptyMemInfo_ReturnsZeroWithNoOutput)
{
    // Arrange
    stub.set_lamda(static_cast<bool (*)(QMap<QString, QString> &)>(getGpuMemInfoForFTDTM),
                   [](QMap<QString, QString> &mapInfo) -> bool {
                       mapInfo.clear();   // 空映射：循环边界（空集合）
                       return true;
                   });
    char arg0[] = "customgpuinfo";
    char *argv[] = { arg0, nullptr };
    std::ostringstream oss;
    std::streambuf *oldBuf = std::cout.rdbuf(oss.rdbuf());
    // Act
    int ret = custom_gpuinfo_original_main(kArgcOne, argv);
    std::cout.rdbuf(oldBuf);
    // Assert
    EXPECT_EQ(0, ret);
    EXPECT_TRUE(oss.str().empty());   // 空映射不产生任何输出
}

TEST_F(CustomGpuInfoTest, Main_MultipleMemEntries_PrintsAllEntriesAndReturnsZero)
{
    // Arrange
    stub.set_lamda(static_cast<bool (*)(QMap<QString, QString> &)>(getGpuMemInfoForFTDTM),
                   [](QMap<QString, QString> &mapInfo) -> bool {
                       mapInfo.insert("EntryA", "1.00GB");
                       mapInfo.insert("EntryB", "2.00GB");
                       mapInfo.insert("EntryC", "3.00GB");   // 循环边界（多元素）
                       return true;
                   });
    char arg0[] = "customgpuinfo";
    char *argv[] = { arg0, nullptr };
    std::ostringstream oss;
    std::streambuf *oldBuf = std::cout.rdbuf(oss.rdbuf());
    // Act
    int ret = custom_gpuinfo_original_main(kArgcOne, argv);
    std::cout.rdbuf(oldBuf);
    // Assert
    EXPECT_EQ(0, ret);
    EXPECT_NE(std::string::npos, oss.str().find("1.00GB"));
    EXPECT_NE(std::string::npos, oss.str().find("2.00GB"));
    EXPECT_NE(std::string::npos, oss.str().find("3.00GB"));
}
