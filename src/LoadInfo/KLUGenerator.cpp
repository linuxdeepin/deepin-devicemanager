#include "KLUGenerator.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceGpu.h"

KLUGenerator::KLUGenerator()
{

}

void KLUGenerator::generatorGpuDevice()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("gpuinfo");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceGpu device;
        device.setGpuInfo(*it);
        DeviceManager::instance()->addGpuDevice(device);
    }
}
