// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | HttpDriverInterface(ctor) | low | - | 1 | 1 |
// | getDriverInfoFromJson | high | complexity:12 | 3 | 3 |
// | checkDriverInfo(json) | high | cognitive:19 | 3 | 7 |
// | getRequestBoard | high | complexity:11 | 3 | 3 |
// | getRequestJson | high | - | 2 | 2 |
// | getRequestPrinter | high | - | 2 | 2 |
// | getRequestCamera | high | - | 2 | 2 |
// | isPkgInstalled | low | - | 1 | 3 |
// | getInstance | mid | - | 2 | 3 |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（main 入口与 ~HttpDriverInterface 析构已豁免）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]（level 最大值、空 JSON、空列表）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（分支以独立 TEST_F 覆盖，未用参数化）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无异常路径）
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]（统一 stub_ext）
//
// 分支清单 → 用例映射：
//   getDriverInfoFromJson: B1 非法 JSON false      | GetDriverInfoFromJson_InvalidJson_ReturnsFalse
//                         B2 msg!=success false    | GetDriverInfoFromJson_MsgNotSuccess_ReturnsFalse
//                         B3 合法条目字段解析      | GetDriverInfoFromJson_ValidJson_ParsesRepoFields
//   checkDriverInfo:       B1 空 JSON false          | CheckDriverInfo_EmptyJson_ReturnsFalse
//                         B2 JSON 解析失败 false    | CheckDriverInfo_InvalidJson_ReturnsFalse
//                         B3 空列表 false           | CheckDriverInfo_EmptyList_ReturnsFalse
//                         B4 max level 未安装 true  | CheckDriverInfo_MaxLevelNotInstalled_ReturnsTrue
//                         B5 max level 已安装 false | CheckDriverInfo_MaxLevelInstalled_ReturnsFalse
//                         B6 同名同版本 false       | CheckDriverInfo_SameNameAndVersion_ReturnsFalse
//                         B7 多条目仅取最高等级     | CheckDriverInfo_MultiEntry_OnlyMaxLevelCompared
//   isPkgInstalled:        B1 apt 输出为空 false     | IsPkgInstalled_EmptyOutput_ReturnsFalse
//                         B2 第二行含版本 true      | IsPkgInstalled_VersionInSecondLine_ReturnsTrue
//                         B3 第二行不含版本 false   | IsPkgInstalled_VersionMissing_ReturnsFalse
//   getRequestJson:        B1 网络错误 ""            | GetRequestJson_NetworkError_ReturnsEmpty
//                         B2 正常返回 payload       | GetRequestJson_Success_ReturnsPayload
//   getRequestBoard:       B1 厂商为空 ""            | GetRequestBoard_EmptyManufacturer_ReturnsEmpty
//                         B2 V25 分支新版 URL       | GetRequestBoard_V25_BuildsVersionedUrl
//                         B3 旧分支 product/class   | GetRequestBoard_Legacy_BuildsProductUrl
//   getRequestPrinter:     B1 无厂商仅 arch/system   | GetRequestPrinter_NoManufacturer_OmitsParam
//                         B2 HP 归一化              | GetRequestPrinter_HPManufacturer_NormalizesName
//   getRequestCamera:      B1 带 desc                | GetRequestCamera_WithDesc_AppendsDesc
//                         B2 空 desc 无 desc 参数   | GetRequestCamera_EmptyDesc_OmitsDescParam
//   getInstance:           B1 单例同指针             | GetInstance_CalledTwice_ReturnsSamePointer
//
// 环境隔离（self-checker §6，均经 stub.set_lamda 拦截）：QNetworkAccessManager::get / QTimer::start /
// QIODevice::readAll / QNetworkReply::error 全部 stub，不发起真实网络请求；
// Utils::getOsBuild/getUrl/getVersion/executeServerCmd、Common::getArchStore
// 静态方法 stub，不读真实系统文件。qnam 本体未触发（get 已被替换）。

#include "../ut_Head.h"
#include "drivercontrol/httpdriverinterface.h"
#include "drivercontrol/utils.h"

#include <DSysInfo>

#include <QEventLoop>
#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <stubext.h>
#include <gtest/gtest.h>

namespace {

// 最小化 QNetworkReply 子类：仅用于承载指针类型，内容全部经 stub 注入
class FakeReply : public QNetworkReply
{
public:
    explicit FakeReply(const QByteArray &data, QNetworkReply::NetworkError err)
        : m_data(data), m_err(err)
    {
        setOpenMode(QIODevice::ReadOnly);
    }
    void abort() override { }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        const qint64 n = qMin<qint64>(maxlen, m_data.size() - m_pos);
        if (n > 0)
            memcpy(data, m_data.constData() + m_pos, static_cast<size_t>(n));
        m_pos += n;
        return n;
    }
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    QByteArray m_data;
    qint64 m_pos = 0;
    QNetworkReply::NetworkError m_err;
};

} // namespace

// getRequestBoard/Printer/Camera 的返回值是 getRequestJson 的结果（JSON 文本），
// 出站 URL 经 qnam.get 桩捕获到 g_lastRequestUrl 供断言
static QString g_lastRequestUrl;

class HttpDriverInterfaceTest : public ::testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override { g_lastRequestUrl.clear(); }
    void TearDown() override
    {
        stub.clear();
    }

    // 安装网络栈 stub：get 返回 FakeReply，readAll/error 返回受控内容；
    // QEventLoop::exec 立即返回（getRequestJson 中 finished 仅连 loop.quit，
    // 数据在 exec 后经 reply->readAll() 读取，故无需发射信号）；
    // QTimer::start 置空（exec 已不再等待，避免真实定时器残留）
    void installNetStub(const QByteArray &payload, QNetworkReply::NetworkError err)
    {
        stub.set_lamda(static_cast<QNetworkReply *(QNetworkAccessManager::*)(const QNetworkRequest &)>(&QNetworkAccessManager::get),
                       [payload, err](QNetworkAccessManager *, const QNetworkRequest &request) -> QNetworkReply * {
                           g_lastRequestUrl = request.url().toString();
                           return new FakeReply(payload, err);
                       });
        stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                       [](QEventLoop *, QEventLoop::ProcessEventsFlags) -> int { return 0; });
        stub.set_lamda(static_cast<void (QTimer::*)(int)>(&QTimer::start),
                       [](QTimer *, int) { });
        stub.set_lamda(&QIODevice::readAll, [payload](QIODevice *) { return payload; });
        stub.set_lamda(&QNetworkReply::error, [err](QNetworkReply *) { return err; });
    }

    // 安装 URL 组装所需静态依赖的 stub（含 DSysInfo，避免读真实系统配置）
    void installUrlDepsStub()
    {
        stub.set_lamda(&Utils::getOsBuild, []() { return QString("0100"); });
        stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
        stub.set_lamda(&Utils::getVersion, [](QString &major, QString &minor) {
            major = "25"; minor = "0"; return true;
        });
        stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
        stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
            return QByteArray();
        });
    }

    // getRequestPrinter/getRequestCamera 依赖 DSysInfo 打桩，值仅拼接进 system 参数
    void installDsysInfoStub()
    {
        stub.set_lamda(&Dtk::Core::DSysInfo::uosType,
                       []() { return Dtk::Core::DSysInfo::UosType(0); });
        stub.set_lamda(&Dtk::Core::DSysInfo::uosEditionType,
                       []() { return Dtk::Core::DSysInfo::UosEdition(0); });
    }
};

// ---------- getInstance ----------

TEST_F(HttpDriverInterfaceTest, GetInstance_CalledTwice_ReturnsSamePointer)
{
    // Arrange — 首次获取（单例惰性构造）
    HttpDriverInterface *first = HttpDriverInterface::getInstance();

    // Act — 再次获取
    HttpDriverInterface *second = HttpDriverInterface::getInstance();

    // Assert
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    EXPECT_EQ(HttpDriverInterface::getInstance(), first);
}

TEST_F(HttpDriverInterfaceTest, GetInstance_DirectCall_ReturnsNonNullPointer)
{
    // Arrange — 指针初始化为空，验证惰性构造产生有效实例
    HttpDriverInterface *inst = nullptr;

    // Act — 直接获取单例
    inst = HttpDriverInterface::getInstance();

    // Assert — 非 null，且与再次获取结果一致
    EXPECT_NE(inst, static_cast<HttpDriverInterface *>(nullptr));
    EXPECT_EQ(HttpDriverInterface::getInstance(), inst);
}

// ---------- getDriverInfoFromJson ----------

TEST_F(HttpDriverInterfaceTest, GetDriverInfoFromJson_InvalidJson_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    QList<RepoDriverInfo> lst;

    // Act
    const bool ok = iface->getDriverInfoFromJson("{invalid", lst);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(lst.isEmpty());
}

TEST_F(HttpDriverInterfaceTest, GetDriverInfoFromJson_MsgNotSuccess_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    QList<RepoDriverInfo> lst;
    const QByteArray payload = R"({"msg":"error","code":"1"})";

    // Act
    const bool ok = iface->getDriverInfoFromJson(QString::fromUtf8(payload), lst);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_TRUE(lst.isEmpty());
}

TEST_F(HttpDriverInterfaceTest, GetDriverInfoFromJson_ValidJson_ParsesRepoFields)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    QList<RepoDriverInfo> lst;
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[
        {"deb_manufacturer":"NVIDIA","deb_version":"535.100","packages":"nvidia-driver","level":5,"arch":"amd64"},
        {"deb_version":"1.0.0","packages":"misc-driver","level":1}]}})";

    // Act
    const bool ok = iface->getDriverInfoFromJson(QString::fromUtf8(payload), lst);

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_EQ(lst.size(), 2);
    EXPECT_EQ(lst.at(0).strPackages, "nvidia-driver");
    EXPECT_EQ(lst.at(0).strDebVersion, "535.100");
    EXPECT_EQ(lst.at(0).strDebManufacturer, "NVIDIA");
    EXPECT_EQ(lst.at(1).strPackages, "misc-driver");
    EXPECT_EQ(lst.at(1).strDebVersion, "1.0.0");
}

// ---------- checkDriverInfo(json, DriverInfo&) ----------

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_EmptyJson_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info;

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString(), info);

    // Assert
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_InvalidJson_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info;

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8("not-a-json"), info);

    // Assert
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_EmptyList_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info;
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8(R"({"msg":"success","code":"0","data":{"list":[]}})"), info);

    // Assert
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_MaxLevelNotInstalled_ReturnsTrue)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info; // 空 driverName / version：命中"未安装需更新"分支
    // apt 输出为空 → isPkgInstalled 为 false → 需要更新
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[
        {"packages":"nvidia-driver","deb_version":"535.100","level":5}]}})";

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8(payload), info);

    // Assert
    EXPECT_TRUE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_MaxLevelInstalled_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info; // 空名 + 已安装：无需更新
    // apt 输出第二行包含 deb_version → isPkgInstalled true
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray("nvidia-driver:\n  Installed: 535.100\n  Candidate: 535.100\n");
    });
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[
        {"packages":"nvidia-driver","deb_version":"535.100","level":5}]}})";

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8(payload), info);

    // Assert
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_SameNameAndVersion_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info;
    info.driverName = "nvidia-driver";
    info.version = "535.100";
    // 最高等级条目与本地同名同版本 → 免安装分支返回 false
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[
        {"packages":"nvidia-driver","deb_version":"535.100","level":5}]}})";

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8(payload), info);

    // Assert
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_MultiEntry_OnlyMaxLevelCompared)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    DriverInfo info;
    info.driverName = "pkg-high";
    info.version = "2.0"; // 与最高等级条目版本不同 → 循环结束后仍需更新
    // 低等级条目（pkg-low）已安装，但只比较最高等级（pkg-high）：
    // executeServerCmd 按包名分流，确保低等级条目不会触发"已安装"判定
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &args, const QString &, int, bool) {
        if (args.contains("pkg-low"))
            return QByteArray("pkg-low:\n  Installed: 1.0\n");
        return QByteArray();
    });
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[
        {"packages":"pkg-low","deb_version":"1.0","level":1},
        {"packages":"pkg-high","deb_version":"9.9","level":5}]}})";

    // Act
    const bool needsUpdate = iface->checkDriverInfo(QString::fromUtf8(payload), info);

    // Assert
    EXPECT_TRUE(needsUpdate);
}

// ---------- isPkgInstalled ----------

TEST_F(HttpDriverInterfaceTest, IsPkgInstalled_EmptyOutput_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });

    // Act
    const bool installed = iface->isPkgInstalled("some-pkg", "1.0");

    // Assert
    EXPECT_FALSE(installed);
}

TEST_F(HttpDriverInterfaceTest, IsPkgInstalled_VersionInSecondLine_ReturnsTrue)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray("some-pkg:\n  Installed: 3.5-1\n  Candidate: 3.5-1\n");
    });

    // Act
    const bool installed = iface->isPkgInstalled("some-pkg", "3.5-1");

    // Assert
    EXPECT_TRUE(installed);
}

TEST_F(HttpDriverInterfaceTest, IsPkgInstalled_VersionMissing_ReturnsFalse)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray("some-pkg:\n  Installed: (none)\n  Candidate: 3.5-1\n");
    });

    // Act
    const bool installed = iface->isPkgInstalled("some-pkg", "3.5-1");

    // Assert
    EXPECT_FALSE(installed);
}

// ---------- getRequestJson ----------

TEST_F(HttpDriverInterfaceTest, GetRequestJson_NetworkError_ReturnsEmpty)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    installNetStub(QByteArray("ignored"), QNetworkReply::ConnectionRefusedError);

    // Act
    const QString json = iface->getRequestJson("http://mock-url/query");

    // Assert — 返回空串；成员 strJsonDriverInfo 在错误判定前已写入原始载荷
    EXPECT_TRUE(json.isEmpty());
    EXPECT_EQ(iface->strJsonDriverInfo, QString::fromUtf8("ignored"));
}

TEST_F(HttpDriverInterfaceTest, GetRequestJson_Success_ReturnsPayload)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    const QByteArray payload = R"({"msg":"success","code":"0","data":[]})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act
    const QString json = iface->getRequestJson("http://mock-url/query");

    // Assert
    EXPECT_EQ(json, QString::fromUtf8(payload));
    EXPECT_EQ(iface->strJsonDriverInfo, QString::fromUtf8(payload));
}

// ---------- getRequestBoard ----------

TEST_F(HttpDriverInterfaceTest, GetRequestBoard_EmptyManufacturer_ReturnsEmpty)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();

    // Act
    const QString url = iface->getRequestBoard("", "SomeBoard", 1, 2);

    // Assert
    EXPECT_TRUE(url.isEmpty());
}

TEST_F(HttpDriverInterfaceTest, GetRequestBoard_V25_BuildsVersionedUrl)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    installUrlDepsStub(); // V25 分支：getVersion true 且 major==25
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act — 返回值为 getRequestJson 的 JSON 结果
    const QString json = iface->getRequestBoard("Intel", "UHD-630", 1, 2);

    // Assert — 出站 URL（V25 分支）与返回载荷
    EXPECT_EQ(g_lastRequestUrl, QString("http://mock-url?deb_manufacturer=Intel&desc=UHD-630&arch=amd64")
              + "&system=1-0&majorVersion=25&minorVersion=0"); // getOsBuild "0100" → build[1]=='1'
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

TEST_F(HttpDriverInterfaceTest, GetRequestBoard_Legacy_BuildsProductUrl)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::getOsBuild, []() { return QString("0100"); });
    stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
    stub.set_lamda(&Utils::getVersion, [](QString &major, QString &minor) {
        major = "20"; minor = "5"; return true;
    });
    stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act — getVersion major="20" 走旧分支
    const QString json = iface->getRequestBoard("Intel", "UHD-630", 1, 2);

    // Assert — 出站 URL（旧分支含 product/class 参数）与返回载荷
    EXPECT_EQ(g_lastRequestUrl, QString("http://mock-url?arch=amd64&system=1-0")
              + "&deb_manufacturer=Intel&product=UHD-630&class_p=1&class=2");
    EXPECT_FALSE(g_lastRequestUrl.contains("desc=")); // desc 参数仅 V25 分支使用
    EXPECT_FALSE(g_lastRequestUrl.contains("majorVersion"));
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

// ---------- getRequestPrinter ----------

TEST_F(HttpDriverInterfaceTest, GetRequestPrinter_NoManufacturer_OmitsParam)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
    stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    installDsysInfoStub();
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act — 厂商/描述为空则省略对应参数
    const QString json = iface->getRequestPrinter("", "");

    // Assert — system=0-0 来自 DSysInfo 桩（uosType 0 / uosEditionType 0）
    EXPECT_EQ(g_lastRequestUrl, QString("http://mock-url?arch=amd64&system=0-0"));
    EXPECT_FALSE(g_lastRequestUrl.contains("deb_manufacturer"));
    EXPECT_FALSE(g_lastRequestUrl.contains("desc="));
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

TEST_F(HttpDriverInterfaceTest, GetRequestPrinter_HPManufacturer_NormalizesName)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
    stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    installDsysInfoStub();
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act — Hewlett-Packard 归一化为 HP
    const QString json = iface->getRequestPrinter("Hewlett-Packard", "LaserJet-1020");

    // Assert
    EXPECT_TRUE(g_lastRequestUrl.contains("deb_manufacturer=HP"));
    EXPECT_TRUE(g_lastRequestUrl.contains("desc=LaserJet-1020"));
    EXPECT_FALSE(g_lastRequestUrl.contains("Hewlett-Packard"));
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

// ---------- getRequestCamera ----------

TEST_F(HttpDriverInterfaceTest, GetRequestCamera_WithDesc_AppendsDesc)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
    stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    installDsysInfoStub();
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act
    const QString json = iface->getRequestCamera("HD-WebCam");

    // Assert
    EXPECT_TRUE(g_lastRequestUrl.contains("arch=amd64"));
    EXPECT_TRUE(g_lastRequestUrl.contains("desc=HD-WebCam"));
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

TEST_F(HttpDriverInterfaceTest, GetRequestCamera_EmptyDesc_OmitsDescParam)
{
    // Arrange
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&Utils::getUrl, []() { return QString("http://mock-url"); });
    stub.set_lamda(&Common::getArchStore, []() { return QString("amd64"); });
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    installDsysInfoStub();
    const QByteArray payload = R"({"msg":"success","code":"0","data":{"list":[]}})";
    installNetStub(payload, QNetworkReply::NoError);

    // Act — 无描述时省略 desc 参数
    const QString json = iface->getRequestCamera();

    // Assert
    EXPECT_FALSE(g_lastRequestUrl.contains("desc="));
    EXPECT_TRUE(g_lastRequestUrl.contains("arch=amd64"));
    EXPECT_EQ(json, QString::fromUtf8(payload));
}

// ---------- checkDriverInfo(DriverInfo&) 重载（按设备类型分发）与单例析构 ----------

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_NetworkType_BoardJsonNotInstalled_ReturnsTrue)
{
    // Arrange — Network 类型走 getRequestBoard 分发；仓库最高等级驱动本地未安装
    // （网络层经 stub.set_lamda 拦截，返回固定 JSON，不发起真实请求）
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&HttpDriverInterface::getRequestBoard,
                   [](HttpDriverInterface *, QString, QString, int, int) -> QString {
                       return QString::fromUtf8(R"({"msg":"success","code":"0","data":{"list":[
                           {"packages":"nvidia-driver","deb_version":"535.100","level":5}]}})");
                   });
    // apt 输出为空 → isPkgInstalled false → 需要更新
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray();
    });
    DriverInfo info;
    info.type = DR_Network;
    info.vendorId = QStringLiteral("10de");
    info.modelId = QStringLiteral("2504");

    // Act
    const bool needsUpdate = iface->checkDriverInfo(info);

    // Assert — 分发后委托 json 重载解析，未安装返回 true
    EXPECT_TRUE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_PrinterType_PrinterJsonEmpty_ReturnsFalse)
{
    // Arrange — Printer 类型走 getRequestPrinter 分发，返回空 JSON → 委托层 false
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&HttpDriverInterface::getRequestPrinter,
                   [](HttpDriverInterface *, QString, QString) -> QString {
                       return QString();
                   });
    DriverInfo info;
    info.type = DR_Printer;
    info.vendorName = QStringLiteral("HP");
    info.modelName = QStringLiteral("LaserJet");

    // Act
    const bool needsUpdate = iface->checkDriverInfo(info);

    // Assert — 空 JSON 委托返回 false
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, CheckDriverInfo_BoardSameNameVersion_ReturnsFalse)
{
    // Arrange — Board JSON 最高等级驱动与本地同名同版本 → 无需更新
    HttpDriverInterface *iface = HttpDriverInterface::getInstance();
    stub.set_lamda(&HttpDriverInterface::getRequestBoard,
                   [](HttpDriverInterface *, QString, QString, int, int) -> QString {
                       return QString::fromUtf8(R"({"msg":"success","code":"0","data":{"list":[
                           {"packages":"nvidia-driver","deb_version":"535.100","level":5}]}})");
                   });
    DriverInfo info;
    info.type = DR_Network;
    info.vendorId = QStringLiteral("10de");
    info.modelId = QStringLiteral("2504");
    info.driverName = QStringLiteral("nvidia-driver");
    info.version = QStringLiteral("535.100");

    // Act
    const bool needsUpdate = iface->checkDriverInfo(info);

    // Assert — 本地已匹配推荐驱动，返回 false
    EXPECT_FALSE(needsUpdate);
}

TEST_F(HttpDriverInterfaceTest, GetInstance_DestroyedInstance_RecreatedOnNextCall)
{
    // Arrange — 销毁单例实例：先摘除原子指针再 delete（覆盖析构函数）
    HttpDriverInterface *old = HttpDriverInterface::getInstance();
    ASSERT_NE(old, nullptr);
    HttpDriverInterface::s_Instance.store(nullptr);

    // Act
    delete old;
    HttpDriverInterface *fresh = HttpDriverInterface::getInstance();

    // Assert — 析构后重新获取得到新实例
    EXPECT_NE(fresh, nullptr);
    EXPECT_NE(fresh, old);
}
