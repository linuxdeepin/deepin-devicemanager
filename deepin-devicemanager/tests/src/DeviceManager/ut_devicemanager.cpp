/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../src/DeviceManager/DeviceManager.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceCpu.h"
#include "../src/DeviceManager/DeviceGpu.h"
#include "../src/DeviceManager/DeviceStorage.h"
#include "../src/DeviceManager/DeviceMemory.h"
#include "../src/DeviceManager/DeviceMonitor.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../src/DeviceManager/DeviceBluetooth.h"
#include "../src/DeviceManager/DeviceAudio.h"
#include "../src/DeviceManager/DeviceNetwork.h"
#include "../src/DeviceManager/DeviceImage.h"
#include "../src/DeviceManager/DeviceOthers.h"
#include "../src/DeviceManager/DevicePower.h"
#include "../src/DeviceManager/DevicePrint.h"
#include "../src/DeviceManager/DeviceOtherPCI.h"
#include "../src/DeviceManager/DeviceComputer.h"
#include "../src/DeviceManager/DeviceCdrom.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QIODevice>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceManager_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

TEST_F(DeviceManager_UT, DeviceManager_UT_clear)
{
    DeviceManager::instance()->clear();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_getDeviceTypes)
{
    DeviceManager::instance()->getDeviceTypes();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setDeviceListClass)
{
    DeviceManager::instance()->setDeviceListClass();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_getDeviceList)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    DeviceManager::instance()->getDeviceList("over", bInfo);
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addMouseDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    DeviceManager::instance()->addMouseDevice(device);
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addMouseInfoFromLshw)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");
    device->setInfoFromHwinfo(mapinfo);
    DeviceManager::instance()->addMouseInfoFromLshw(mapinfo);
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addCpuDevice)
{
    DeviceCpu *cpu = new DeviceCpu;
    DeviceManager::instance()->addCpuDevice(cpu);
    DeviceManager::instance()->m_ListDeviceCPU.clear();
    delete cpu;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addStorageDeivce)
{
    DeviceStorage *st = new DeviceStorage;
    DeviceManager::instance()->addStorageDeivce(st);
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete st;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addLshwinfoIntoStorageDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromlshw(mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.append(device);
    DeviceManager::instance()->addLshwinfoIntoStorageDevice(mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addLshwinfoIntoNVMEStorageDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromlshw(mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.append(device);
    DeviceManager::instance()->addLshwinfoIntoNVMEStorageDevice(mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setStorageInfoFromSmartctl)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromlshw(mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.append(device);
    DeviceManager::instance()->setStorageInfoFromSmartctl("test", mapinfo);
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setStorageDeviceMediaType)
{
    DeviceManager::instance()->setStorageDeviceMediaType("test", "/n");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setKLUStorageDeviceMediaType)
{
    DeviceManager::instance()->setKLUStorageDeviceMediaType("test", "/n");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addGpuDevice)
{
    DeviceGpu *gpu = new DeviceGpu;
    DeviceManager::instance()->addGpuDevice(gpu);
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setGpuInfoFromXrandr)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceGpu *gpu = new DeviceGpu;
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);
    DeviceManager::instance()->setGpuInfoFromXrandr(mapinfo);
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setGpuSizeFromDmesg)
{
    DeviceManager::instance()->setGpuSizeFromDmesg("2g");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addMemoryDevice)
{
    DeviceMemory *m = new DeviceMemory;
    DeviceManager::instance()->addMemoryDevice(m);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setMemoryInfoFromDmidecode(mapinfo);
    DeviceManager::instance()->m_ListDeviceMemory.clear();
    delete m;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addMonitor)
{
    DeviceMonitor *m = new DeviceMonitor;
    DeviceManager::instance()->addMonitor(m);
    DeviceManager::instance()->m_ListDeviceMonitor.clear();
    delete m;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setMonitorInfoFromXrandr)
{
    DeviceManager::instance()->setMonitorInfoFromXrandr("/", "/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setCurrentResolution)
{
    DeviceManager::instance()->setCurrentResolution("/", "/", "/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addBiosDevice)
{
    DeviceBios *bios = new DeviceBios;
    DeviceManager::instance()->addBiosDevice(bios);
    DeviceManager::instance()->m_ListDeviceBios.clear();
    delete bios;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setLanguageInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setLanguageInfo(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addBluetoothDevice)
{
    DeviceBluetooth *b = new DeviceBluetooth;
    DeviceManager::instance()->addBluetoothDevice(b);
    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete b;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setBluetoothInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setBluetoothInfoFromLshw(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addAudioDevice)
{
    DeviceAudio *a = new DeviceAudio;
    DeviceManager::instance()->addAudioDevice(a);
    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete a;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setAudioInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setAudioInfoFromLshw(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setAudioChipFromDmesg)
{
    DeviceManager::instance()->setAudioChipFromDmesg("st");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addNetworkDevice)
{
    DeviceNetwork *n = new DeviceNetwork;
    DeviceManager::instance()->addNetworkDevice(n);
    DeviceManager::instance()->m_ListDeviceNetwork.clear();
    delete n;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addImageDevice)
{
    DeviceImage *i = new DeviceImage;
    DeviceManager::instance()->addImageDevice(i);
    DeviceManager::instance()->m_ListDeviceImage.clear();
    delete i;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addKeyboardDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    DeviceManager::instance()->addKeyboardDevice(device);
    DeviceManager::instance()->m_ListDeviceKeyboard.clear();
    delete device;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setKeyboardInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setKeyboardInfoFromLshw(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addOthersDevice)
{
    DeviceOthers *o = new DeviceOthers;
    DeviceManager::instance()->addOthersDevice(o);
    DeviceManager::instance()->m_ListDeviceOthers.clear();
    delete o;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_setOthersDeviceInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->setOthersDeviceInfoFromLshw(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addPowerDevice)
{
    DevicePower *p = new DevicePower;
    DeviceManager::instance()->addPowerDevice(p);
    DeviceManager::instance()->m_ListDevicePower.clear();
    delete p;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addPrintDevice)
{
    DevicePrint *print = new DevicePrint;
    DeviceManager::instance()->addPrintDevice(print);
    DeviceManager::instance()->m_ListDevicePrint.clear();
    delete print;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addOtherPCIDevice)
{
    DeviceOtherPCI *o = new DeviceOtherPCI;
    DeviceManager::instance()->addOtherPCIDevice(o);
    DeviceManager::instance()->m_ListDeviceOtherPCI.clear();
    delete o;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addComputerDevice)
{
    DeviceComputer *c = new DeviceComputer;
    DeviceManager::instance()->addComputerDevice(c);
    DeviceManager::instance()->m_ListDeviceComputer.clear();
    delete c;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addCdromDevice)
{
    DeviceCdrom *c = new DeviceCdrom;
    DeviceManager::instance()->addCdromDevice(c);
    DeviceManager::instance()->m_ListDeviceCdrom.clear();
    delete c;
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addLshwinfoIntoCdromDevice)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->addLshwinfoIntoCdromDevice(mapinfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addBusId)
{
    DeviceManager::instance()->addBusId(QStringList() << "bus");
    DeviceManager::instance()->getBusId();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addCmdInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("/n", info);
    DeviceManager::instance()->addCmdInfo(cmdInfo);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_cmdInfo)
{
    DeviceManager::instance()->cmdInfo("/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_exportToTxt)
{
    DeviceManager::instance()->exportToTxt("/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_exportToXlsx)
{
    DeviceManager::instance()->exportToXlsx("/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_exportToDoc)
{
    //    DeviceManager::instance()->exportToDoc("/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_exportToHtml)
{
    DeviceManager::instance()->exportToHtml("/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_currentXlsRow)
{
    DeviceManager::instance()->currentXlsRow();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_overviewToTxt)
{
    QTextStream out;
    DeviceManager::instance()->overviewToTxt(out);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_overviewToHtml)
{
    QFile out;
    DeviceManager::instance()->overviewToHtml(out);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_overviewToDoc)
{
    Docx::Document doc;
    //    DeviceManager::instance()->overviewToDoc(doc);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_overviewToXlsx)
{
    QXlsx::Document xlsx;
    QXlsx::Format font;
    DeviceManager::instance()->overviewToXlsx(xlsx, font);
}

TEST_F(DeviceManager_UT, DeviceManager_UT_infoToHtml)
{
    QDomDocument doc;
    DeviceManager::instance()->infoToHtml(doc, "/", "/");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_getDeviceOverview)
{
    QDomDocument doc;
    DeviceManager::instance()->getDeviceOverview();
    DeviceManager::instance()->getDeviceDriverPool();
}

TEST_F(DeviceManager_UT, DeviceManager_UT_addInputInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    DeviceManager::instance()->addInputInfo("/", mapinfo);
    DeviceManager::instance()->inputInfo("test");
}

TEST_F(DeviceManager_UT, DeviceManager_UT_isDeviceExistInPairedDevice)
{
    DeviceManager::instance()->isDeviceExistInPairedDevice("/");
    DeviceManager::instance()->setCpuNum(0);
}
