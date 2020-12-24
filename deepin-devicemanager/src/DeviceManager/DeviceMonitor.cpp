// 项目自身文件
#include "DeviceMonitor.h"

// Qt库文件
#include <QDebug>
#include <QDate>
#include <QSize>

// 其它头文件
#include <math.h>

DeviceMonitor::DeviceMonitor()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_DisplayInput("")
    , m_VGA("Disable")
    , m_HDMI("Disable")
    , m_DVI("Disable")
    , m_Interface("")
    , m_ScreenSize("")
    , m_AspectRatio("")
    , m_MainScreen("")
    , m_CurrentResolution("")
    , m_SerialNumber("")
    , m_ProductionWeek("")
    , m_Driver("")
    , m_Width(0)
    , m_Height(0)
{
    // 初始化可显示属性
    initFilterKey();
}

// 获得显示屏的大小
QString DeviceMonitor::parseMonitorSize(const QString &sizeDescription, double &inch, QSize &retSize)
{
    inch = 0.0;

    // 根据不同的正则表达式解析屏幕大小字符串
    QString res = sizeDescription;
    QRegExp re("^([\\d]*)x([\\d]*) mm$");
    if (re.exactMatch(sizeDescription)) {

        // 获取屏幕宽高 int
        m_Width = re.cap(1).toInt();
        m_Height = re.cap(2).toInt();
        retSize = QSize(m_Width, m_Height);

        // 获取屏幕尺寸大小 inch
        double width = m_Width / 2.54;
        double height = m_Height / 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + QObject::tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    re.setPattern("([0-9]\\d*)mm x ([0-9]\\d*)mm");
    if (re.exactMatch(sizeDescription)) {

        // 获取屏幕宽高 int
        m_Width = re.cap(1).toInt();
        m_Height = re.cap(2).toInt();
        retSize = QSize(m_Width, m_Height);

        // 获取屏幕尺寸大小 inch
        double width = m_Width / 2.54;
        double height = m_Height / 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + QObject::tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    return res;
}

void DeviceMonitor::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    //设置由hwinfo --monitor获取信息
    setAttribute(mapInfo, "Model", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "", m_DisplayInput);
    setAttribute(mapInfo, "Size", m_ScreenSize);
    setAttribute(mapInfo, "", m_MainScreen);
    setAttribute(mapInfo, "Resolution", m_SupportResolution);

    double inch = 0.0;
    QSize size(0, 0);
    QString inchValue = parseMonitorSize(m_ScreenSize, inch, size);
    m_ScreenSize = inchValue;

    // 获取当前分辨率 和 当前支持分辨率
    QStringList listResolution = m_SupportResolution.split(" ");
    m_CurrentResolution = listResolution.last();
    m_SupportResolution = "";
    foreach (const QString &word, listResolution) {
        if (word.contains("@")) {
            m_SupportResolution.append(word);
            m_SupportResolution.append("  ,  ");
        }
    }

    // 计算显示比例
    caculateScreenRatio();

    m_SupportResolution.replace(QRegExp(",  $"), "");

    m_ProductionWeek  = transWeekToDate(mapInfo["Year of Manufacture"], mapInfo["Week of Manufacture"]);
    setAttribute(mapInfo, "Serial ID", m_SerialNumber);

    // 加载其他属性
    getOtherMapInfo(mapInfo);
}

void DeviceMonitor::setInfoFromSelfDefine(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "CurResolution", m_CurrentResolution);
    setAttribute(mapInfo, "SupportResolution", m_SupportResolution);
    setAttribute(mapInfo, "Size", m_ScreenSize);
    setAttribute(mapInfo, "Date", m_ProductionWeek);
    // 加载其他属性
    //loadOtherDeviceInfo(mapInfo);
}

void DeviceMonitor::setInfoFromEdid(const QMap<QString, QString> &mapInfo)
{
    m_Name = "Monitor " + mapInfo["Vendor"];
    setAttribute(mapInfo, "Size", m_ScreenSize);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    getOtherMapInfo(mapInfo);
}

QString DeviceMonitor::transWeekToDate(const QString &year, const QString &week)
{
    int y = year.toInt();
    int w = week.toInt();
    QDate date(y, 1, 1);
    date = date.addDays(w * 7 - 1);
    return date.toString("yyyy-MM");
}

bool DeviceMonitor::setInfoFromXradr(const QString &main, const QString &edid)
{
    // 判断该显示器设备是否已经设置过从xrandr获取的消息
    if (!m_Interface.isEmpty()) {
        return false;
    }

    if (main.contains("disconnected")) {
        return false;
    }

    // 获取屏幕的主要信息，包括借口(HDMI VGA)/是否主显示器和屏幕大小，
    // 但是这里计算的屏幕大小仅仅用来匹配是否是同一个显示器,真正的屏幕大小计算是根据edid计算的
    if (!setMainInfoFromXrandr(main)) {
        return false;
    }

    // 根据edid计算屏幕大小
    if (edid.isEmpty() == false) {
        caculateScreenSize(edid);
    }

    return true;
}

bool DeviceMonitor::setCurrentResolution(const QString &resolution, const QString &rate)
{
    // 判断该显示器设备是否已经设置过从xrandr获取的消息
    if (m_CurrentResolution.contains("@")) {
        return false;
    }

    m_CurrentResolution = QString("%1@%2Hz").arg(resolution).arg(rate);
    m_CurrentResolution.replace(" ", "");
    return true;
}

const QString &DeviceMonitor::name()const
{
    return m_Name;
}

const QString &DeviceMonitor::driver() const
{
    return m_Driver;
}

QString DeviceMonitor::subTitle()
{
    return m_Name;
}

const QString DeviceMonitor::getOverviewInfo()
{
    QString ov;

    ov = QString("%1(%2)").arg(m_Name).arg(m_ScreenSize);

    return ov;
}

void DeviceMonitor::initFilterKey()
{
    addFilterKey(QObject::tr("Date"));
}

void DeviceMonitor::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Type"), m_Model);
    addBaseDeviceInfo(tr("Display Input"), m_DisplayInput);
    addBaseDeviceInfo(tr("Interface Type"), m_Interface);
}

void DeviceMonitor::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Support Resolution"), m_SupportResolution);
    addOtherDeviceInfo(tr("Current Resolution"), m_CurrentResolution);
    addOtherDeviceInfo(tr("Primary Monitor"), m_MainScreen);
    addOtherDeviceInfo(tr("Display Ratio"), m_AspectRatio);
    addOtherDeviceInfo(tr("Size"), m_ScreenSize);
    addOtherDeviceInfo(tr("Serial Number"), m_SerialNumber);
    addOtherDeviceInfo(tr("Product Date"), m_ProductionWeek);

    mapInfoToList();
}

void DeviceMonitor::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}

bool DeviceMonitor::setMainInfoFromXrandr(const QString &info)
{
    // 判断是否是同一个设备
    // 这个代码一开始是有的，但是后来被删除了(目前没有搞清楚为什么被删除了)，现在有加上了(为了解决Bug37377)
    // 代码的主要作用是判断是否是同一个设备，如果需要删除这个，请务必考虑到Bug37377
    QString mInfo = info;
    mInfo.replace(QRegExp("\\(.*\\)"), "");
    QRegExp re(".*([0-9]{3,5})mm\\sx\\s([0-9]{3,5})mm");
    if (!re.exactMatch(mInfo)) {
        return false;
    }

    if (m_Width != re.cap(1).toInt()) {
        return false;
    }

    if (m_Height != re.cap(2).toInt()) {
        return false;
    }

    // 设置用的是哪个接口
    if (info.startsWith("VGA")) {
        m_Interface = "VGA";
    } else if (info.startsWith("HDMI")) {
        m_Interface = "HDMI";
    } else if (info.startsWith("eDP")) {
        m_Interface = "eDP";
    } else if (info.startsWith("DisplayPort")) {
        m_Interface = "DisplayPort";
    }

    // 设置是否是主显示器
    if (info.contains("primary")) {
        m_MainScreen = "Yes";
    } else {
        m_MainScreen = "NO";
    }

    return true;
}

void DeviceMonitor::caculateScreenRatio()
{
    QRegExp re("^([\\d]*)x([\\d]*)(.*)$");
    if (re.exactMatch(m_CurrentResolution)) {
        int width = re.cap(1).toInt();
        int height = re.cap(2).toInt();
        int gys = gcd(width, height);
        int w = width / gys;
        int h = height / gys;

        if (w > 21) {
            findAspectRatio(w, h, w, h);
        }

        m_AspectRatio = QString::number(w) + " : " + QString::number(h);
    }
}

int DeviceMonitor::gcd(int a, int b)
{
    if (a < b) {
        std::swap(a, b);
    }

    if (a % b == 0) {
        return b;
    } else {
        return gcd(b, a % b);
    }
}

bool DeviceMonitor::findAspectRatio(int width, int height, int &ar_w, int &ar_h)
{
    float r1 = float(width) / float(height);

    for (ar_w = 21; ar_w > 0; --ar_w) {
        for (ar_h = 21; ar_h > 0; --ar_h) {
            if (std::abs(r1 - float(ar_w) / float(ar_h)) / r1 < 0.01) {
                int r = gcd(ar_w, ar_h);
                ar_w /= r;
                ar_h /= r;
                return true;
            }
        }
    }

    return false;
}

void DeviceMonitor::caculateScreenSize()
{
    // 527x296 mm
    QRegExp re(".*([0-9]{3,5})x([0-9]{3,5})\\smm$");
    if (re.exactMatch(m_ScreenSize)) {
        m_Width = re.cap(1).toInt();
        m_Height = re.cap(2).toInt();

        double inch = std::sqrt((m_Width / 2.54) * (m_Width / 2.54) + (m_Height / 2.54) * (m_Height / 2.54)) / 10.0;
//        m_ScreenSize = QString("%1英寸(%2mm X %3mm)").arg(QString::number(inch, 'f', 1)).arg(m_Width).arg(m_Height);
        m_ScreenSize = QString("%1 %2(%3mm X %4mm)").arg(QString::number(inch, 'f', 1)).arg(QObject::tr("inch")).arg(m_Width).arg(m_Height);
    }
}

void DeviceMonitor::caculateScreenSize(const QString &edid)
{
    QStringList list = edid.split('\n');
    if (list.size() < 2) {
        return;
    }

    QString secondItem = list.at(1);
    QString width_field = secondItem.mid(10, 2);
    QString height_field = secondItem.mid(12, 2);

    double width = 0;
    bool trWidthOk = false;
    double height = 0;
    bool trHeightOk = false;
    width = width_field.toInt(&trWidthOk, 16);
    height = height_field.toInt(&trHeightOk, 16);
    if (trWidthOk == false || trHeightOk == false) {
        return;
    }

    if (height <= 0) {
        return;
    }

    if (width <= 0) {
        return;
    }

    double inch = std::sqrt(height * height + width * width) / 2.54;
//    m_ScreenSize = QString("%1英寸(%2cm X %3cm)").arg(QString::number(inch, 'f', 1)).arg(width).arg(height);
    m_ScreenSize = QString("%1 %2(%3cm X %4cm)").arg(QString::number(inch, '0', 1)).arg(QObject::tr("inch")).arg(width).arg(height);
}
