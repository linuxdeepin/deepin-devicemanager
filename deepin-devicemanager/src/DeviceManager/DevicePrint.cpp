// 项目自身文件
#include "DevicePrint.h"

// Qt库文件
#include <QDebug>

// 其它头文件
#include "EnableManager.h"

DevicePrint::DevicePrint()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_SerialNumber("")
    , m_InterfaceType("")
    , m_URI("")
    , m_Status("")
    , m_Shared("")
{
    // 初始化可显示属性
    initFilterKey();

    // 设备可禁用
    m_CanEnable = true;
}

void DevicePrint::setInfo(const QMap<QString, QString> &info)
{
    // 获取打印机类型和型号
    QString vt;
    setAttribute(info, "printer-info", vt);
    QStringList lstInfo = vt.split(" ");
    if (lstInfo.size() > 1) {
        m_Vendor = lstInfo[0];
        m_Model = vt.replace(m_Vendor, "").trimmed();
    }

    // 获取打印机名称
    setAttribute(info, "Name", m_Name);

    // 获取其它信息
    setAttribute(info, "device-uri", m_URI);
    setAttribute(info, "printer-state", m_Status);

    // 获取打印机接口
    QStringList lstUri = m_URI.split(":");
    if (lstUri.size() > 1) {
        m_InterfaceType = lstUri[0];
    }


    getOtherMapInfo(info);
}

const QString &DevicePrint::name()const
{
    return m_Name;
}

const QString &DevicePrint::driver() const
{
    return m_Driver;
}

QString DevicePrint::subTitle()
{
    return m_Name;
}

const QString DevicePrint::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DevicePrint::setEnable(bool e)
{
    // 设置启用禁用状态
    EnableDeviceStatus res = EnableManager::instance()->enablePrinter(m_Name, e);

    // 禁用成功，状态设为5
    if (res == EDS_Success && e == false) {
        m_Status = "5";
    } else if (res == EDS_Success && e == true) {
        // 启用成功状态设为3
        m_Status = "3";
    }

    return res;
}

bool DevicePrint::enable()
{
    return m_Status == "5" ? false : true;
}

void DevicePrint::initFilterKey()
{
    // 初始化可显示属性
    addFilterKey(QObject::tr("copies"));
    addFilterKey(QObject::tr("job-cancel-after"));
    addFilterKey(QObject::tr("job-hold-until"));
    addFilterKey(QObject::tr("job-priority"));
    addFilterKey(QObject::tr("marker-change-time"));
    addFilterKey(QObject::tr("number-up"));
    addFilterKey(QObject::tr("orientation-requested"));
    addFilterKey(QObject::tr("print-color-mode"));
    addFilterKey(QObject::tr("printer-is-accepting-jobs"));
    addFilterKey(QObject::tr("printer-is-shared"));
    addFilterKey(QObject::tr("printer-is-temporary"));
    addFilterKey(QObject::tr("printer-make-and-model"));
    addFilterKey(QObject::tr("printer-state-change-time"));
    addFilterKey(QObject::tr("printer-state-reasons"));
    addFilterKey(QObject::tr("printer-type"));
    addFilterKey(QObject::tr("printer-uri-supported"));
    addFilterKey(QObject::tr("sides"));
}

void DevicePrint::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Serial Number"), m_SerialNumber);
}

void DevicePrint::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Shared"), m_Shared);
    addOtherDeviceInfo(tr("URI"), m_URI);
    addOtherDeviceInfo(tr("Status"), m_Status);
    addOtherDeviceInfo(tr("Interface Type"), m_InterfaceType);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DevicePrint::loadTableData()
{
    // 加载表格数据
    QString name;
    if (!enable()) {
        name = "(" + tr("Disable") + ") " + m_Name;
    } else {
        name = m_Name;
    }

    m_TableData.append(name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
