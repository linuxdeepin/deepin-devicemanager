#include "ThreadPool.h"
#include<QDebug>
#include <unistd.h>
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"


DeviceTask::DeviceTask(DeviceType deviceType, ThreadPool *parent)
    : m_Type(deviceType), mp_Parent(parent)
{
}
DeviceTask::~DeviceTask()
{

}
void DeviceTask::run()
{
    DeviceInfoParser infoParser;
    switch (m_Type) {
    case DT_Audio:
        infoParser.loadAudioInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Audio Device Info..."));
        }
        break;
    case DT_Bios:
        infoParser.loadBiosInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading BIOS Info..."));
        }
        break;
    case DT_Bluetoorh:
        infoParser.loadBluetoothInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Bluetooth Device Info..."));
        }
        break;
    case DT_Cdrom:
        infoParser.loadCdromInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading CD-ROM Info..."));
        }
        break;
    case DT_Computer:
        infoParser.loadComputerInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Operating System Info..."));
        }
        break;
    case DT_Cpu:
        infoParser.loadCpuInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading CPU Info..."));
        }
        break;
    case DT_Gpu:
        infoParser.loadGpuInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Gpu info..."));
        }
        break;
    case DT_Image:
        infoParser.loadImageInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Image Devices Info..."));
        }
        break;
    case DT_Keyboard:
        infoParser.loadKeyboardInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Keyboard info..."));
        }
        break;
    case DT_Memory:
        infoParser.loadMemoryInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Memory Info..."));
        }
        break;
    case DT_Monitor:
        infoParser.loadMonitorInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Monitor Info..."));
        }
        break;
    case DT_Mouse:
        infoParser.loadMouseInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Mouse info..."));
        }
        break;
    case DT_Network:
        infoParser.loadNetworkInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Network Adapter Info..."));
        }
        break;
    case DT_OtherPCI:

        break;
    case DT_Others:
        infoParser.loadOtherDevices();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Other Devices Info..."));
        }
        break;
    case DT_Power:
        infoParser.loadPowerInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Power Info..."));
        }
        break;
    case DT_Print:
        infoParser.loadPrinterInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Printer Info..."));
        }
        break;
    default: {

    }
    }
}






ThreadPool::ThreadPool(QObject *parent) : QThreadPool(parent)
{
    m_ListDeviceType.push_back(DT_Audio);
    m_ListDeviceType.push_back(DT_Bios);
    m_ListDeviceType.push_back(DT_Bluetoorh);
    m_ListDeviceType.push_back(DT_Computer);
    m_ListDeviceType.push_back(DT_Cpu);
    m_ListDeviceType.push_back(DT_Gpu);
    m_ListDeviceType.push_back(DT_Image);
    m_ListDeviceType.push_back(DT_Keyboard);
    m_ListDeviceType.push_back(DT_Memory);
    m_ListDeviceType.push_back(DT_Monitor);
    m_ListDeviceType.push_back(DT_Mouse);
    m_ListDeviceType.push_back(DT_Network);
    m_ListDeviceType.push_back(DT_Power);
    m_ListDeviceType.push_back(DT_Print);
    m_ListDeviceType.push_back(DT_Others);
    m_ListDeviceType.push_back(DT_Cdrom);
    //setMaxThreadCount(8);
}

void ThreadPool::finishedTask(const QString &info)
{
    m_FinishedNum++;
    if (m_FinishedNum == m_ListDeviceType.size()) {
        emit finished("finish");
    } else {
        emit finished(info);
    }
}

void ThreadPool::loadDeviceInfo()
{
    DeviceManager::instance()->clear();
    DeviceInfoParser::clear();
    m_FinishedNum = 0;
    foreach (DeviceType type, m_ListDeviceType) {
        start(new DeviceTask(type, this));
    }
}

