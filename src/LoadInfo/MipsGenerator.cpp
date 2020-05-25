#include "MipsGenerator.h"
#include "CmdTool.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMouse.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceKeyboard.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"

MipsGenerator::MipsGenerator()
{

}


void MipsGenerator::getOthersInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }

        // 这里特殊处理数位板问题(武汉那边的数位板)
        if ((*it)["Device"].contains("PM")) {
            DeviceOthers device;
            device.setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
        }

        // 过滤
        if ((*it)["Hardware Class"] != "unknown" || (*it)["Driver"] == "btusb" || (*it)["Driver"] == "snd-usb-audio") {
            continue;
        }
        DeviceOthers device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addOthersDevice(device);
    }
}
