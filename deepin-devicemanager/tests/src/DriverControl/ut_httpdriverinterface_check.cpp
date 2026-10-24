// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// HttpDriverInterface 深度用例(任务06):
// - checkDriverInfo: 最优等级选择 + 状态机(经 packageInstall 打桩)
// - packageInstall: apt policy 三态解析(经 Common::executeClientCmd 打桩, 不触真实 apt)
// - getRequestBoard: URL 构造(经 getOsBuild/getVersion/getRequestJson 打桩)
// - getRequest: DriverType 分发 + network error 信号
// - convertJsonToDeviceList: 全字段映射(ppds/models/size 格式化)
// 所有外部依赖(DBus/网络/子进程)全部打桩, 测试不依赖真实设备与服务。

#include "HttpDriverInterface.h"
#include "MacroDefinition.h"
#include "commonfunction.h"
#include "commontools.h"
#include "ut_Head.h"
#include "stub.h"

#include <QEventLoop>
#include <QFile>
#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QStringList>

#include <cstring>
#include <dlfcn.h>

#include <gtest/gtest.h>

// ---------------- stub 状态 ----------------
static int ut_pkgInstall_ret = 0;          // packageInstall 桩返回值
static int ut_pkgInstall_calls = 0;        // packageInstall 调用次数
static QString ut_pkgInstall_lastPkg;      // packageInstall 最后一次收到的包名
static QByteArray ut_apt_policy_out;       // executeClientCmd 桩返回的 apt policy 文本
static QString ut_os_build;                // getOsBuild 桩返回值
static bool ut_version_ok = false;         // getVersion 桩返回值
static QString ut_version_major, ut_version_minor;
static QString ut_board_url;               // getRequestJson 桩捕获的 URL
static QString ut_board_json_ret = "{}";   // getRequestJson 桩返回值
static QString ut_board_lastUrl;           // getRequestBoard 桩捕获
static QString ut_printer_lastMan, ut_printer_lastDesc;
static QString ut_checkInfo_lastJson;      // checkDriverInfo 桩捕获
static int ut_checkInfo_calls = 0;

static void ut_reset_http_stubs()
{
    ut_pkgInstall_ret = 0;
    ut_pkgInstall_calls = 0;
    ut_pkgInstall_lastPkg.clear();
    ut_apt_policy_out.clear();
    ut_os_build.clear();
    ut_version_ok = false;
    ut_version_major.clear();
    ut_version_minor.clear();
    ut_board_url.clear();
    ut_board_json_ret = "{}";
    ut_board_lastUrl.clear();
    ut_printer_lastMan.clear();
    ut_printer_lastDesc.clear();
    ut_checkInfo_lastJson.clear();
    ut_checkInfo_calls = 0;
    ut_apt_policy_out.clear();
}

// ---------------- 桩函数 ----------------
// 非静态成员桩: 自由函数首参即 this
static int ut_stub_packageInstall(HttpDriverInterface *, const QString &pkg, const QString &)
{
    ++ut_pkgInstall_calls;
    ut_pkgInstall_lastPkg = pkg;
    return ut_pkgInstall_ret;
}

// Common::executeClientCmd 是静态成员, 返回 QByteArray: 桩签名与原签名一致
static QByteArray ut_stub_executeClientCmd(const QString &, const QStringList &, const QString &, int, bool)
{
    return ut_apt_policy_out;
}

static QString ut_stub_getOsBuild(HttpDriverInterface *)
{
    return ut_os_build;
}

static bool ut_stub_getVersion(HttpDriverInterface *, QString &major, QString &minor)
{
    major = ut_version_major;
    minor = ut_version_minor;
    return ut_version_ok;
}

static QString ut_stub_getRequestJson(HttpDriverInterface *, const QString &strUrl)
{
    ut_board_url = strUrl;
    return ut_board_json_ret;
}

static QString ut_stub_getUrl()
{
    return QStringLiteral("http://ut.url");
}

static QString ut_stub_getArchStore()
{
    return QStringLiteral("amd64");
}

static QString ut_stub_getRequestBoard(HttpDriverInterface *, const QString &man, const QString &models, int, int)
{
    ut_board_lastUrl = man + "|" + models;
    return ut_board_json_ret;
}

static QString ut_stub_getRequestPrinter(HttpDriverInterface *, const QString &man, const QString &desc)
{
    ut_printer_lastMan = man;
    ut_printer_lastDesc = desc;
    return ut_board_json_ret;
}

static void ut_stub_checkDriverInfo(HttpDriverInterface *, QString strJson, DriverInfo *)
{
    ++ut_checkInfo_calls;
    ut_checkInfo_lastJson = strJson;
}

class UT_HttpDriverInterfaceCheck : public UT_HEAD
{
public:
    void SetUp() override { ut_reset_http_stubs(); }
    void TearDown() override { }
};

// ---------------- checkDriverInfo ----------------
TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_emptyJson_noop)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo("", &info);
    EXPECT_EQ(0, ut_pkgInstall_calls);
    EXPECT_TRUE(info.packages().isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_badJson_noop)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo("{\"msg\":", &info);
    EXPECT_EQ(0, ut_pkgInstall_calls);
    EXPECT_TRUE(info.packages().isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_emptyList_noop)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":[]}}", &info);
    EXPECT_EQ(0, ut_pkgInstall_calls);
    EXPECT_TRUE(info.packages().isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_installedNew)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    ut_pkgInstall_ret = 2;
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":[{\"packages\":\"pkg-a\",\"deb_version\":\"1.0.0\",\"level\":5,\"size\":4096}]}}",
        &info);
    EXPECT_EQ(QStringLiteral("pkg-a"), info.packages());
    EXPECT_EQ(QStringLiteral("1.0.0"), info.debVersion());
    EXPECT_EQ(QStringLiteral("4.00KB"), info.size());
    EXPECT_EQ(ST_DRIVER_IS_NEW, info.status());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_canUpdate)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    ut_pkgInstall_ret = 1;
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":[{\"packages\":\"pkg-a\",\"deb_version\":\"1.0.0\"}]}}",
        &info);
    EXPECT_EQ(ST_CAN_UPDATE, info.status());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_notInstall_defaultType)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    ut_pkgInstall_ret = 0;
    DriverInfo info;   // driverName 为空且非打印机
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":[{\"packages\":\"pkg-a\",\"deb_version\":\"1.0.0\"}]}}",
        &info);
    EXPECT_EQ(ST_NOT_INSTALL, info.status());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_notInstall_printerType_canUpdate)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    ut_pkgInstall_ret = 0;
    DriverInfo info;
    info.m_Type = DR_Printer;
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":[{\"packages\":\"pkg-a\",\"deb_version\":\"1.0.0\"}]}}",
        &info);
    EXPECT_EQ(ST_CAN_UPDATE, info.status());
}

TEST_F(UT_HttpDriverInterfaceCheck, checkDriverInfo_picksHighestLevel)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_stub_packageInstall);
    ut_pkgInstall_ret = 2;
    DriverInfo info;
    HttpDriverInterface::getInstance()->checkDriverInfo(
        "{\"msg\":\"success\",\"data\":{\"list\":"
        "[{\"packages\":\"pkg-low\",\"deb_version\":\"1.0\",\"level\":1},"
        "{\"packages\":\"pkg-high\",\"deb_version\":\"2.0\",\"level\":5}]}}",
        &info);
    EXPECT_EQ(QStringLiteral("pkg-high"), ut_pkgInstall_lastPkg);
    EXPECT_EQ(QStringLiteral("pkg-high"), info.packages());
    EXPECT_EQ(QStringLiteral("2.0"), info.debVersion());
}

// ---------------- packageInstall ----------------
TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_emptyAptOut_zero)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out.clear();
    EXPECT_EQ(0, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "1.0"));
}

TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_notInstalled_zero)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out = "ut-pkg:\n  Installed: (none)\n  Candidate: 2.0\n";
    EXPECT_EQ(0, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "2.0"));
}

TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_versionMatch_two)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out = "ut-pkg:\n  Installed: 1.2.3\n  Candidate: 1.2.3\n";
    EXPECT_EQ(2, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "1.2.3"));
}

TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_installedLower_one)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out = "ut-pkg:\n  Installed: 1.2.3\n  Candidate: 1.2.3\n";
    EXPECT_EQ(1, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "2.0"));
}

TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_installedHigher_two)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out = "ut-pkg:\n  Installed: 3.0\n  Candidate: 3.0\n";
    EXPECT_EQ(2, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "1.0"));
}

TEST_F(UT_HttpDriverInterfaceCheck, packageInstall_pkgNotFirstLine_two)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    ut_apt_policy_out = "some header\nut-pkg:\n  Installed: 1.2.3\n  Candidate: 1.2.3\n";
    EXPECT_EQ(2, HttpDriverInterface::getInstance()->packageInstall("ut-pkg", "1.2.3"));
}

// ---------------- getRequestBoard URL 构造 ----------------
TEST_F(UT_HttpDriverInterfaceCheck, getRequestBoard_emptyArgs_empty)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getOsBuild), ut_stub_getOsBuild);
    stub.set(ADDR(HttpDriverInterface, getVersion), ut_stub_getVersion);
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_stub_getRequestJson);
    EXPECT_TRUE(HttpDriverInterface::getInstance()->getRequestBoard("", "model").isEmpty());
    EXPECT_TRUE(ut_board_url.isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequestBoard_major25_url)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getOsBuild), ut_stub_getOsBuild);
    stub.set(ADDR(HttpDriverInterface, getVersion), ut_stub_getVersion);
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_stub_getRequestJson);
    stub.set(ADDR(CommonTools, getUrl), ut_stub_getUrl);
    stub.set(ADDR(Common, getArchStore), ut_stub_getArchStore);
    ut_version_ok = true;
    ut_version_major = "25";
    ut_version_minor = "0";
    ut_os_build.clear();
    ut_board_json_ret = "board-json";
    const QString url = HttpDriverInterface::getInstance()->getRequestBoard("UTMan", "UTModel");
    EXPECT_EQ(QStringLiteral("http://ut.url?deb_manufacturer=UTMan&desc=UTModel&arch=amd64"
                             "&majorVersion=25&minorVersion=0"),
              ut_board_url);
    EXPECT_EQ(QStringLiteral("board-json"), url);   // 返回值为 getRequestJson 的解析结果
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequestBoard_defaultBranch_url)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getOsBuild), ut_stub_getOsBuild);
    stub.set(ADDR(HttpDriverInterface, getVersion), ut_stub_getVersion);
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_stub_getRequestJson);
    stub.set(ADDR(CommonTools, getUrl), ut_stub_getUrl);
    stub.set(ADDR(Common, getArchStore), ut_stub_getArchStore);
    ut_version_ok = false;
    ut_os_build.clear();
    HttpDriverInterface::getInstance()->getRequestBoard("UTMan", "UTModel", 3, 7);
    EXPECT_EQ(QStringLiteral("http://ut.url?arch=amd64&deb_manufacturer=UTMan&product=UTModel"
                             "&class_p=3&class=7"),
              ut_board_url);
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequestBoard_buildTransform_url)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getOsBuild), ut_stub_getOsBuild);
    stub.set(ADDR(HttpDriverInterface, getVersion), ut_stub_getVersion);
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_stub_getRequestJson);
    stub.set(ADDR(CommonTools, getUrl), ut_stub_getUrl);
    stub.set(ADDR(Common, getArchStore), ut_stub_getArchStore);
    ut_version_ok = false;
    ut_os_build = "1100";   // 专业版: build[1]=='1' -> system="1-0"
    HttpDriverInterface::getInstance()->getRequestBoard("UTMan", "UTModel");
    EXPECT_EQ(QStringLiteral("http://ut.url?arch=amd64&system=1-0"
                             "&deb_manufacturer=UTMan&product=UTModel"),
              ut_board_url);
}

// ---------------- getRequest 类型分发 ----------------
TEST_F(UT_HttpDriverInterfaceCheck, getRequest_printerDispatch)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_stub_getRequestBoard);
    stub.set(ADDR(HttpDriverInterface, getRequestPrinter), ut_stub_getRequestPrinter);
    stub.set(ADDR(HttpDriverInterface, checkDriverInfo), ut_stub_checkDriverInfo);
    ut_board_json_ret = "printer-json";
    DriverInfo info;
    info.m_Type = DR_Printer;
    info.m_VendorName = "VPr";
    info.m_ModelName = "MPr";
    HttpDriverInterface::getInstance()->getRequest(&info);
    EXPECT_EQ(QStringLiteral("VPr"), ut_printer_lastMan);
    EXPECT_EQ(QStringLiteral("MPr"), ut_printer_lastDesc);
    EXPECT_TRUE(ut_board_lastUrl.isEmpty());
    EXPECT_EQ(QStringLiteral("printer-json"), ut_checkInfo_lastJson);
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequest_wifiDispatch)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_stub_getRequestBoard);
    stub.set(ADDR(HttpDriverInterface, getRequestPrinter), ut_stub_getRequestPrinter);
    stub.set(ADDR(HttpDriverInterface, checkDriverInfo), ut_stub_checkDriverInfo);
    ut_board_json_ret = "wifi-json";
    DriverInfo info;
    info.m_Type = DR_WiFi;
    info.m_VendorId = "VID";
    info.m_ModelId = "MID";
    HttpDriverInterface::getInstance()->getRequest(&info);
    EXPECT_EQ(QStringLiteral("VID|MID"), ut_board_lastUrl);
    EXPECT_TRUE(ut_printer_lastMan.isEmpty());
    EXPECT_EQ(QStringLiteral("wifi-json"), ut_checkInfo_lastJson);
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequest_scannerFallthrough_board)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_stub_getRequestBoard);
    stub.set(ADDR(HttpDriverInterface, getRequestPrinter), ut_stub_getRequestPrinter);
    stub.set(ADDR(HttpDriverInterface, checkDriverInfo), ut_stub_checkDriverInfo);
    DriverInfo info;
    info.m_Type = DR_Scaner;
    info.m_VendorId = "VScan";
    info.m_ModelId = "MScan";
    HttpDriverInterface::getInstance()->getRequest(&info);
    EXPECT_EQ(QStringLiteral("VScan|MScan"), ut_board_lastUrl);
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequest_unknownType_noCall)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_stub_getRequestBoard);
    stub.set(ADDR(HttpDriverInterface, getRequestPrinter), ut_stub_getRequestPrinter);
    stub.set(ADDR(HttpDriverInterface, checkDriverInfo), ut_stub_checkDriverInfo);
    DriverInfo info;
    info.m_Type = DR_Null;
    HttpDriverInterface::getInstance()->getRequest(&info);
    EXPECT_TRUE(ut_board_lastUrl.isEmpty());
    EXPECT_TRUE(ut_printer_lastMan.isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequest_networkError_signal)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_stub_getRequestBoard);
    ut_board_json_ret = "network error";
    HttpDriverInterface *hdi = HttpDriverInterface::getInstance();
    QSignalSpy spy(hdi, &HttpDriverInterface::sigRequestFinished);
    DriverInfo info;
    info.m_Type = DR_WiFi;
    hdi->getRequest(&info);
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_EQ(QStringLiteral("network error"), spy.at(0).at(1).toString());
    EXPECT_EQ(0, ut_checkInfo_calls);
}

// ---------------- convertJsonToDeviceList 全字段 ----------------
TEST_F(UT_HttpDriverInterfaceCheck, convertJson_fullFields)
{
    QList<RepoDriverInfo> lst;
    const QString json =
        "{\"msg\":\"success\",\"data\":{\"list\":[{"
        "\"arch\":\"amd64\",\"manufacturer\":\"man\",\"deb_manufacturer\":\"debman\","
        "\"version\":\"1.0\",\"deb_version\":\"1.0.0\",\"packages\":\"pkg-f\","
        "\"class_p\":\"1\",\"class\":\"2\",\"products\":\"prod\",\"deb\":\"debname\","
        "\"level\":9,\"system\":\"25\",\"desc\":\"d1\",\"adaptation\":\"adp\","
        "\"source\":\"src\",\"download_url\":\"http://d/1.deb\","
        "\"size\":15728640,"
        "\"models\":[\"m1\",\"m2\"],"
        "\"ppds\":[{\"desc\":\"pd\",\"manufacturer\":\"pm\",\"source\":\"ps\"}]"
        "}]}}";
    ASSERT_TRUE(HttpDriverInterface::getInstance()->convertJsonToDeviceList(json, lst));
    ASSERT_EQ(1, lst.size());
    const RepoDriverInfo &d = lst.at(0);
    EXPECT_EQ(QStringLiteral("amd64"), d.strArch);
    EXPECT_EQ(QStringLiteral("man"), d.strManufacturer);
    EXPECT_EQ(QStringLiteral("debman"), d.strDebManufacturer);
    EXPECT_EQ(QStringLiteral("1.0"), d.strVersion);
    EXPECT_EQ(QStringLiteral("1.0.0"), d.strDebVersion);
    EXPECT_EQ(QStringLiteral("pkg-f"), d.strPackages);
    EXPECT_EQ(QStringLiteral("1"), d.strClass_p);
    EXPECT_EQ(QStringLiteral("2"), d.strClass);
    EXPECT_EQ(QStringLiteral("prod"), d.strProducts);
    EXPECT_EQ(QStringLiteral("debname"), d.strDeb);
    EXPECT_EQ(9, d.iLevel);
    EXPECT_EQ(QStringLiteral("25"), d.strSystem);
    EXPECT_EQ(QStringLiteral("d1"), d.strDesc);
    EXPECT_EQ(QStringLiteral("adp"), d.strAdaptation);
    EXPECT_EQ(QStringLiteral("src"), d.strSource);
    EXPECT_EQ(QStringLiteral("http://d/1.deb"), d.strDownloadUrl);
    EXPECT_EQ(QList<QString>({QStringLiteral("m1"), QStringLiteral("m2")}), d.strModels);
    EXPECT_EQ(QStringLiteral("15.00MB"), d.strSize);
    ASSERT_EQ(1, d.lstPpds.size());
    EXPECT_EQ(QStringLiteral("pd"), d.lstPpds.at(0).strDesc);
    EXPECT_EQ(QStringLiteral("pm"), d.lstPpds.at(0).strManufacturer);
    EXPECT_EQ(QStringLiteral("ps"), d.lstPpds.at(0).strSource);
}

TEST_F(UT_HttpDriverInterfaceCheck, convertJson_sizeUnitGb)
{
    QList<RepoDriverInfo> lst;
    const QString json =
        "{\"msg\":\"success\",\"data\":{\"list\":[{\"packages\":\"pkg-g\",\"size\":2000000000}]}}";
    ASSERT_TRUE(HttpDriverInterface::getInstance()->convertJsonToDeviceList(json, lst));
    EXPECT_EQ(QStringLiteral("1.86GB"), lst.at(0).strSize);
}

// =====================================================================
// getRequestJson 真实方法体: 网络层打桩(模拟 QNetworkReply), 不发真实请求
// =====================================================================
class UtMockReply : public QNetworkReply
{
public:
    UtMockReply(const QByteArray &data, QNetworkReply::NetworkError err)
        : m_data(data), m_err(err)
    {
        setFinished(true);
        if (err != QNetworkReply::NoError)
            setError(err, QStringLiteral("ut error"));
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    }
    void abort() override { }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        if (m_err != QNetworkReply::NoError)
            return -1;
        const qint64 n = qMin<qint64>(maxlen, m_data.size() - m_pos);
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

static QByteArray ut_net_payload;
static QNetworkReply::NetworkError ut_net_error = QNetworkReply::NoError;

static QNetworkReply *ut_qnam_get(void *, const QNetworkRequest &)
{
    return new UtMockReply(ut_net_payload, ut_net_error);
}

static int ut_loop_exec(void *, QEventLoop::ProcessEventsFlags)
{
    return 0;   // 立即返回, 避免 10s 超时等待
}

static bool ut_qfile_openOk = true;
static QByteArray ut_qfile_content;

static bool ut_qfile_open(void *, QIODevice::OpenMode)
{
    return ut_qfile_openOk;
}

static QByteArray ut_qfile_readAll(void *)
{
    return ut_qfile_content;
}

// QFile::open 是虚函数, 取成员指针得到的是 vcall thunk(太小无法打桩),
// 需用 dlsym 取真实符号地址后打桩; 返回 false 表示符号不可用。
static bool ut_patch_file(Stub &stub)
{
    void *sym = dlsym(RTLD_DEFAULT,
                      "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    if (!sym)
        return false;
    stub.set(sym, reinterpret_cast<void *>(ut_qfile_open));
    stub.set(ADDR(QIODevice, readAll), ut_qfile_readAll);
    return true;
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequestJson_realBody_successPayload)
{
    Stub stub;
    stub.set(static_cast<QNetworkReply *(QNetworkAccessManager::*)(const QNetworkRequest &)>(&QNetworkAccessManager::get),
             ut_qnam_get);
    stub.set(ADDR(QEventLoop, exec), ut_loop_exec);
    ut_net_payload = "{\"msg\":\"success\"}";
    ut_net_error = QNetworkReply::NoError;

    const QString ret = HttpDriverInterface::getInstance()->getRequestJson("http://ut.url/json");
    EXPECT_EQ(QStringLiteral("{\"msg\":\"success\"}"), ret);
}

TEST_F(UT_HttpDriverInterfaceCheck, getRequestJson_realBody_networkError)
{
    Stub stub;
    stub.set(static_cast<QNetworkReply *(QNetworkAccessManager::*)(const QNetworkRequest &)>(&QNetworkAccessManager::get),
             ut_qnam_get);
    stub.set(ADDR(QEventLoop, exec), ut_loop_exec);
    ut_net_payload.clear();
    ut_net_error = QNetworkReply::HostNotFoundError;

    const QString ret = HttpDriverInterface::getInstance()->getRequestJson("http://ut.url/json");
    EXPECT_EQ(QStringLiteral("network error"), ret);
}

// =====================================================================
// getOsBuild / getVersion 真实方法体: 文件层打桩(硬编码路径 /etc/os-version,
// 不能写真实文件, 故打桩 QFile::open/readAll)
// =====================================================================
TEST_F(UT_HttpDriverInterfaceCheck, getOsBuild_realBody_found)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = true;
    ut_qfile_content = "Version=25\nOsBuild=1100\nMajorVersion=25\nMinorVersion=0\n";

    EXPECT_EQ(QStringLiteral("1100"), HttpDriverInterface::getInstance()->getOsBuild());
}

TEST_F(UT_HttpDriverInterfaceCheck, getOsBuild_realBody_notFound)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = true;
    ut_qfile_content = "Version=25\n";   // 无 OsBuild 行

    EXPECT_TRUE(HttpDriverInterface::getInstance()->getOsBuild().isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, getOsBuild_realBody_openFail)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = false;

    EXPECT_TRUE(HttpDriverInterface::getInstance()->getOsBuild().isEmpty());
}

TEST_F(UT_HttpDriverInterfaceCheck, getVersion_realBody_found)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = true;
    ut_qfile_content = "Version=25\nMajorVersion=25\nMinorVersion=0\n";

    QString major, minor;
    EXPECT_TRUE(HttpDriverInterface::getInstance()->getVersion(major, minor));
    EXPECT_EQ(QStringLiteral("25"), major);
    EXPECT_EQ(QStringLiteral("0"), minor);
}

TEST_F(UT_HttpDriverInterfaceCheck, getVersion_realBody_missingFields)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = true;
    ut_qfile_content = "Version=25\n";   // 无 Major/Minor 行

    QString major, minor;
    EXPECT_FALSE(HttpDriverInterface::getInstance()->getVersion(major, minor));
}

TEST_F(UT_HttpDriverInterfaceCheck, getVersion_realBody_openFail)
{
    Stub stub;
    ASSERT_TRUE(ut_patch_file(stub));
    ut_qfile_openOk = false;

    QString major, minor;
    EXPECT_FALSE(HttpDriverInterface::getInstance()->getVersion(major, minor));
}
