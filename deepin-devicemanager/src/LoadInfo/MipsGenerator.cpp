// 项目自身文件
#include "MipsGenerator.h"

// 其它头文件
#include "CmdTool.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceCpu.h"
#include "../DeviceManager/DeviceGpu.h"
#include "../DeviceManager/DeviceStorage.h"
#include "../DeviceManager/DeviceMemory.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "../DeviceManager/DeviceBios.h"
#include "../DeviceManager/DeviceAudio.h"
#include "../DeviceManager/DeviceBluetooth.h"
#include "../DeviceManager/DeviceNetwork.h"
#include "../DeviceManager/DeviceImage.h"
#include "../DeviceManager/DeviceOthers.h"
#include "../DeviceManager/DeviceComputer.h"
#include "../DeviceManager/DevicePower.h"
#include "../DeviceManager/DeviceCdrom.h"
#include "../DeviceManager/DevicePrint.h"
#include "MacroDefinition.h"

MipsGenerator::MipsGenerator()
{

}

void MipsGenerator::generatorComputerDevice()
{
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer *device = new DeviceComputer() ;

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        device->setType(sysInfo[0]["description"]);
        device->setVendor(sysInfo[0]["vendor"]);
        device->setName(sysInfo[0]["product"]);
    }

    // 龙心机器从sudo dmidecode -t 1中获取机器信息
    const QList<QMap<QString, QString> >  &dmiInfo = DeviceManager::instance()->cmdInfo("dmidecode1");
    if (dmiInfo.size() > 1) {
        device->setName(dmiInfo[1]["Product Name"]);
    }

    // setOsDescription
    QString os = DEFAULT_STR;
    DSysInfo::UosEdition type = DSysInfo::uosEditionType();
    if (DSysInfo::UosProfessional == type) {
        os =  PROF_STR;
    } else if (DSysInfo::UosHome == type) {
        os =  HOME_STR;
    } else if (DSysInfo::UosCommunity == type) {
        os =  COMMUNITY_STR;
    } else if (DSysInfo::UosEnterprise == type) {
        os =  ENTERPRISE_STR;
    } else if (DSysInfo::UosEnterpriseC == type) {
        os =  ENTERPRISEC_STR;
    } else if (DSysInfo::UosEuler == type) {
        os =  EULER_STR;
    }
    device->setOsDescription(os);

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
        int index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
            info.insert(index, reg.cap(1));
        }
        device->setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}
