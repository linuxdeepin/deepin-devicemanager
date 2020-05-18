#include "DevicePrint.h"
#include <QDebug>

DevicePrint::DevicePrint()
    : DeviceBaseInfo(), m_Name(""), m_Vendor(""), m_Model(""), m_SerialNumber("")
    , m_InterfaceType(""), m_URI(""), m_Status(""), m_Shared("")
{
    initFilterKey();
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


    loadOtherDeviceInfo(info);
}

void DevicePrint::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Serial ID", m_SerialNumber);
    setAttribute(mapInfo, "Hotplug", m_InterfaceType);

    loadOtherDeviceInfo(mapInfo);
}

const QString &DevicePrint::name()const
{
    return m_Name;
}
const QString &DevicePrint::vendor()const
{
    return m_Vendor;
}
const QString &DevicePrint::model()const
{
    return m_Model;
}
const QString &DevicePrint::serialNumber()const
{
    return m_SerialNumber;
}
const QString &DevicePrint::interfaceType()const
{
    return m_InterfaceType;
}
const QString &DevicePrint::URI()const
{
    return m_URI;
}
const QString &DevicePrint::status()const
{
    return m_Status;
}
const QString &DevicePrint::shared()const
{
    return m_Shared;
}

void DevicePrint::initFilterKey()
{
    addFilterKey(QObject::tr("copies"));
    //addFilterKey(QObject::tr("device-uri"));
    //addFilterKey(QObject::tr("finishings"));
    addFilterKey(QObject::tr("job-cancel-after"));
    addFilterKey(QObject::tr("job-hold-until"));
    addFilterKey(QObject::tr("job-priority"));
    //addFilterKey(QObject::tr("job-sheets"));
    addFilterKey(QObject::tr("marker-change-time"));
    //addFilterKey(QObject::tr("media-source"));
    //addFilterKey(QObject::tr("media-type"));
    addFilterKey(QObject::tr("number-up"));
    addFilterKey(QObject::tr("orientation-requested"));
    addFilterKey(QObject::tr("print-color-mode"));
    //addFilterKey(QObject::tr("print-quality"));
    //addFilterKey(QObject::tr("printer-commands"));
    //addFilterKey(QObject::tr("printer-info"));
    addFilterKey(QObject::tr("printer-is-accepting-jobs"));
    addFilterKey(QObject::tr("printer-is-shared"));
    addFilterKey(QObject::tr("printer-is-temporary"));
    //addFilterKey(QObject::tr("printer-location"));
    addFilterKey(QObject::tr("printer-make-and-model"));
    //addFilterKey(QObject::tr("printer-state"));
    addFilterKey(QObject::tr("printer-state-change-time"));
    addFilterKey(QObject::tr("printer-state-reasons"));
    addFilterKey(QObject::tr("printer-type"));
    addFilterKey(QObject::tr("printer-uri-supported"));
    addFilterKey(QObject::tr("sides"));
}
