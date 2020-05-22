#include "DeviceMonitor.h"
#include <QDebug>
#include <QDate>
#include <math.h>
#include <QSize>
DeviceMonitor::DeviceMonitor()
    : DeviceBaseInfo(), m_Name(""), m_Vendor(""), m_Model(""), m_DisplayInput(""), m_VGA("Disable")
    , m_HDMI("Disable"), m_DVI("Disable"), m_Interface(""), m_ScreenSize("")
    , m_AspectRatio(""), m_MainScreen(""), m_CurrentResolution(""), m_SerialNumber(""), m_ProductionWeek(""),
      m_Width(0), m_Height(0)
{

}

// 获得显示屏的大小
QString DeviceMonitor::parseMonitorSize(const QString &sizeDescription, double &inch, QSize &retSize)
{
    inch = 0.0;

    QString res = sizeDescription;
    QRegExp re("^([\\d]*)x([\\d]*) mm$");
    if ( re.exactMatch(sizeDescription) ) {
        double width = re.cap(1).toDouble();
        double height = re.cap(2).toDouble();
        retSize = QSize(width, height);
        width /= 2.54;
        height /= 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + QObject::tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    re.setPattern("([0-9]\\d*)mm x ([0-9]\\d*)mm");
    if ( re.exactMatch(sizeDescription) ) {
        double width = re.cap(1).toDouble();
        double height = re.cap(2).toDouble();
        retSize = QSize(width, height);
        width /= 2.54;
        height /= 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + QObject::tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    return res;
}

void DeviceMonitor::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

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
//    m_CurrentResolution = listResolution.last();
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

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    // 加载其他属性
    loadOtherDeviceInfo(mapInfo);
}

void DeviceMonitor::setInfoFromHwinfo(QMap<QString, QString> mapInfo)
{
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

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    // 加载其他属性
    loadOtherDeviceInfo(mapInfo);
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

bool DeviceMonitor::setRateInfoFromXradr(const QString &main, const QString &rate)
{
    // 判断该显示器设备是否已经设置过从xrandr获取的消息
    if (m_CurrentResolution.contains("@")) {
        return false;
    }

    if (m_CurrentResolution.isEmpty()) {
        QStringList rateList = rate.split(" ");
        rateList.removeAll("");
        rateList[1].replace(QRegExp("\\*.?"), "");
        m_CurrentResolution = QString("%1@%2Hz").arg(rateList[0].trimmed()).arg(rateList[1]);
    }


    // 去掉不用的信息
    QString mInfo = main;
    mInfo.replace(QRegExp("\\(.*\\)"), "");
    QRegExp re(".*([0-9]{3,5})mm\\sx\\s([0-9]{3,5})mm");
    if (!re.exactMatch(mInfo)) {
        return false;
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
const QString &DeviceMonitor::vendor()const
{
    return m_Vendor;
}
const QString &DeviceMonitor::model()const
{
    return m_Model;
}
const QString &DeviceMonitor::displayInput()const
{
    return m_DisplayInput;
}
const QString &DeviceMonitor::VGA()const
{
    return m_VGA;
}
const QString &DeviceMonitor::HDMI()const
{
    return m_HDMI;
}
const QString &DeviceMonitor::DVI()const
{
    return m_DVI;
}
const QString &DeviceMonitor::interface()const
{
    return m_Interface;
}
const QString &DeviceMonitor::screenSize()const
{
    return m_ScreenSize;
}
const QString &DeviceMonitor::aspectRatio()const
{
    return m_AspectRatio;
}
const QString &DeviceMonitor::mainScreen()const
{
    return m_MainScreen;
}
const QString &DeviceMonitor::currentResolution()const
{
    return m_CurrentResolution;
}
const QString &DeviceMonitor::serialNumber()const
{
    return m_SerialNumber;
}
const QString &DeviceMonitor::productionWeek()const
{
    return m_ProductionWeek;
}
const QString &DeviceMonitor::supportResolutions()const
{
    return m_SupportResolution;
}

void DeviceMonitor::initFilterKey()
{

}

bool DeviceMonitor::setMainInfoFromXrandr(const QString &info)
{
//    // 去掉不用的信息
//    QString mInfo = info;
//    mInfo.replace(QRegExp("\\(.*\\)"),"");
//    QRegExp re(".*([0-9]{3,5})mm\\sx\\s([0-9]{3,5})mm");
//    if(!re.exactMatch(mInfo)){return false;}

//    if(m_Width != re.cap(1).toInt()){return false;}
//    if(m_Height != re.cap(2).toInt()){return false;}

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
    if ( re.exactMatch(m_CurrentResolution) ) {
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
    if (a < b)
        std::swap(a, b);
    if (a % b == 0)
        return b;
    else
        return gcd(b, a % b);
}
bool DeviceMonitor::findAspectRatio(int width, int height, int &ar_w, int &ar_h)
{
    float r1 = float(width) / float(height);

    for (ar_w = 21; ar_w > 0; --ar_w) {
        for (ar_h = 21; ar_h > 0; --ar_h) {
            if ( std::abs(r1 - float(ar_w) / float(ar_h)) / r1 < 0.01) {
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
        m_ScreenSize = QString("%1英寸(%2mm X %3mm)").arg(QString::number(inch, 'f', 1)).arg(m_Width).arg(m_Height);
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

    int width = 0;
    bool trWidthOk = false;
    int height = 0;
    bool trHeightOk = false;
    width = width_field.toInt(&trWidthOk, 16);
    height = height_field.toInt(&trHeightOk, 16);
    if (trWidthOk == false || trHeightOk == false) {
        return;
    }
    if (height <= 0) return;
    if (width <= 0)return;
    double inch = std::sqrt(height * height + width * width) / 2.54;
    m_ScreenSize = QString("%1英寸(%2cm X %3cm)").arg(QString::number(inch, 'f', 1)).arg(width).arg(height);
}
