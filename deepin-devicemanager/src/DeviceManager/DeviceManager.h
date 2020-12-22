/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H
#include <QList>
#include <QMap>
#include <QMutex>
#include <QDomDocument>
#include <QObject>
#include <QFile>

#include "document.h"
#include "xlsxdocument.h"

//class DeviceMouse;
class DeviceCpu;
class DeviceStorage;
class DeviceGpu;
class DeviceMemory;
class DeviceMonitor;
class DeviceBios;
class DeviceBluetooth;
class DeviceAudio;
class DeviceNetwork;
class DeviceImage;
//class DeviceKeyboard;
class DeviceOthers;
class DevicePower;
class DevicePrint;
class DeviceOtherPCI;
class DeviceComputer;
class DeviceCdrom;
class DeviceInput;
class DeviceBaseInfo;

/**
 * @brief The DeviceManager class
 * 设备的管理类(包括设备的获取、增加、修改)
 */

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    static DeviceManager *instance()
    {
        if (!sInstance) {
            sInstance = new DeviceManager;
        }
        return sInstance;
    }

    /**
     * @brief clear:清除所有设备信息
     */
    void clear();

    /**
     * @brief getDeviceTypes
     * @return 返回设备类型列表
     */
    const QList<QPair<QString, QString>> &getDeviceTypes();

    /**
     * @brief setDeviceListClass:设置设备信息List的分类
     */
    void setDeviceListClass();

    /**
     * @brief getDeviceList : 获取设备列表
     * @param name : 该设备的类型
     * @return ：返回设备列表
     */
    bool getDeviceList(const QString &name, QList<DeviceBaseInfo *> &lst);

    // 鼠标设备相关 **************************************************************************************

    /**
     * @brief addMouseDevice:添加鼠标设备
     * @param device:被添加的鼠标设备
     */
    void addMouseDevice(DeviceInput *const device);

    /**
     * @brief getMouseDevices:获取所有鼠标设备
     * @return 所有鼠标设备组成的List
     */
    const QList<DeviceBaseInfo *> &getMouseDevices();

    /**
     * @brief addMouseInfoFromLshw:添加从lshw获取的鼠标信息
     * @param mapInfo:从lshw获取的鼠标信息map
     * @return 布尔值:true-设置成功；false-设置失败
     */
    bool addMouseInfoFromLshw(const QMap<QString, QString> &mapInfo);


    // CPU设备相关 **************************************************************************************
    /**
     * @brief addCpuDevice:添加CPU设备
     * @param device:被添加的CPU设备
     */
    void addCpuDevice(DeviceCpu *const device);

    /**
     * @brief getCPUDevices:获取所有CPU设备
     * @return:所有CPU设备组成的List
     */
    const QList<DeviceBaseInfo *> &getCPUDevices();

    // 存储设备相关 **************************************************************************************
    /**
     * @brief addStorageDeivce:添加存储设备
     * @param device:被添加的存储设备
     */
    void addStorageDeivce(DeviceStorage *const device);

    /**
     * @brief addLshwinfoIntoStorageDevice:添加由lshw获取的存储设备信息
     * @param mapInfo:由lshw获取的存储设备信息
     */
    void addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo);
    void addLshwinfoIntoNVMEStorageDevice(const QMap<QString, QString> &mapInfo);

    /**
     * @brief getStorageDevices:获取所有存储设备
     * @return 所有存储设备组成的List
     */
    const QList<DeviceBaseInfo *> &getStorageDevices();

    /**
     * @brief setStorageDeviceMediaType:设置存储设备介质类型
     * @param name:逻辑名称
     * @param value:存储设备介质类型信息
     * @return 布尔值:true-设置成功；false-设置失败
     */
    bool setStorageDeviceMediaType(const QString &name, const QString &value);

    /**
     * @brief setKLUStorageDeviceMediaType:KLU机器设置存储设备介质类型
     * @param name:逻辑名称
     * @param value:存储设备介质类型信息
     * @return 布尔值:true-设置成功；false-设置失败
     */
    bool setKLUStorageDeviceMediaType(const QString &name, const QString &value);

    /**
     * @brief setStorageInfoFromSmartctl:设置由smartctl获取的存储设备信息
     * @param name:逻辑名称
     * @param mapInfo:由smartctl获取的存储设备信息map
     */
    void setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    // GPU设备相关 **************************************************************************************
    /**
     * @brief addGpuDevice:添加显卡
     * @param device:被添加的显卡
     */
    void addGpuDevice(DeviceGpu *const device);

    /**
     * @brief getGPUDevices:获取所有显卡
     * @return 所有显卡组成的List
     */
    const QList<DeviceBaseInfo *> &getGPUDevices();
    /**@brief:设置从lshw获取的gup信息，内部会做判断是否是该设备*/
    /**
     * @brief setGpuInfoFromLshw:设置由lshw获取的显卡信息
     * @param mapInfo:由lshw获取的显卡信息map
     */
    void setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setGpuInfoFromXrandr:设置由xrandr获取的显卡信息
     * @param mapInfo:由xrandr获取的显卡信息
     */
    void setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setGpuSizeFromDmesg:设置由Dmesg获取的显卡大小信息
     * @param info:由由Dmesg获取的显卡大小信息
     */
    void setGpuSizeFromDmesg(const QString &info);

    // 内存设备相关 *************************************************************************************
    /**
     * @brief addMemoryDevice:添加内存设备
     * @param device:被添加的内存设备
     */
    void addMemoryDevice(DeviceMemory *const device);

    /**
     * @brief getMemoryDevices:获取所有内存设备
     * @return 所有内存设备组成的List
     */
    const QList<DeviceBaseInfo *> &getMemoryDevices();

    /**
     * @brief setMemoryInfoFromDmidecode:设置由dmidecode获取的内存信息
     * @param mapInfo:由dmidecode获取的内存信息
     */
    void setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

    // 显示设备相关 *************************************************************************************
    /**
     * @brief addMonitor:添加显示设备
     * @param device:被添加的显示设备
     */
    void addMonitor(DeviceMonitor *const device);

    /**
     * @brief getMonitorDevices:获取所有显示设备
     * @return 所有显示设备组成的List
     */
    const QList<DeviceBaseInfo *> &getMonitorDevices();

    /**
     * @brief setMonitorInfoFromXrandr:设置由xrandr获取的显示设备信息
     * @param main:主显示器信息
     * @param edid:edid信息
     */
    void setMonitorInfoFromXrandr(const QString &main, const QString &edid);

    /**
     * @brief setCurrentResolution:设置当前分辨率
     * @param resolution:分辨率
     * @param rate:刷新率
     */
    void setCurrentResolution(const QString &resolution, const QString &rate);


    // Bios设备相关 ************************************************************************************
    /**
     * @brief addBiosDevice:添加BIOS设备
     * @param device:被添加的BIOS设备
     */
    void addBiosDevice(DeviceBios *const device);

    /**
     * @brief getBiosDevices:获取BIOS设备
     * @return 所有BIOS设备组成的List
     */
    const QList<DeviceBaseInfo *> &getBiosDevices();

    /**
     * @brief setLanguageInfo:设置语言信息
     * @param mapInfo:语言信息map
     */
    void setLanguageInfo(const QMap<QString, QString> &mapInfo);

    // 蓝牙设备相关
    /**
     * @brief addBluetoothDevice:添加蓝牙设备
     * @param device:被添加的蓝牙设备
     */
    void addBluetoothDevice(DeviceBluetooth *const device);

    /**
     * @brief getBluetoothDevices:获取所有蓝牙设备
     * @return:所有蓝牙设备组成的List
     */
    const QList<DeviceBaseInfo *> &getBluetoothDevices();

    /**
     * @brief setBluetoothInfoFromLshw:设置由lshw获取的蓝牙信息
     * @param mapInfo:由lshw获取的蓝牙信息map
     */
    void setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setBluetoothInfoFromHwinfo:设置由hwinfo获取的蓝牙信息
     * @param mapInfo:由hwinfo获取的蓝牙信息map
     * @return 布尔值:true设置成功；false设置失败
     */
    bool setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 音频设备相关
    /**
     * @brief addAudioDevice:添加声卡
     * @param device:被添加的声卡
     */
    void addAudioDevice(DeviceAudio *const device);

    /**
     * @brief getAudioDevices:获取所有的声卡
     * @return 所有声卡组成的List
     */
    const QList<DeviceBaseInfo *> &getAudioDevices();

    /**
     * @brief setAudioInfoFromLshw:设置由lshw获取的声卡信息
     * @param mapInfo:由lshw获取的声卡信息map
     */
    void setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setAudioChipFromDmesg:从Dmesg获取的信息设置声卡芯片型号
     * @param info:声卡芯片型号
     */
    void setAudioChipFromDmesg(const QString &info);

    // 网络设备相关
    /**
     * @brief addNetworkDevice:添加网络适配器
     * @param device:被添加的网络适配器
     */
    void addNetworkDevice(DeviceNetwork *const device);

    /**
     * @brief getNetworkDevices:获取所有网络适配器
     * @return 所有网络适配器组成的List
     */
    const QList<DeviceBaseInfo *> &getNetworkDevices();

    /**
     * @brief setNetworkInfoFromHwinfo:设置由hwinfo获取的网络适配器信息
     * @param mapInfo:由hwinfo获取的网络适配器信息map
     */
    void setNetworkInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 图像设备相关
    /**
     * @brief addImageDevice:添加图像设备
     * @param device:被添加的图像设备
     */
    void addImageDevice(DeviceImage *const device);

    /**
     * @brief getImageDevices:获取所有图像设备
     * @return 所有图像设备组成的List
     */
    const QList<DeviceBaseInfo *> &getImageDevices();

    /**
     * @brief setCameraInfoFromLshw:设置由lshw获取的图像设备信息
     * @param mapInfo:由lshw获取的图像设备信息map
     */
    void setCameraInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 键盘设备相关
    /**
     * @brief addKeyboardDevice:添加键盘设备
     * @param device:被添加的键盘设备
     */
    void addKeyboardDevice(DeviceInput *const device);

    /**
     * @brief getKeyboardDevices:获取所有键盘设备
     * @return 所有键盘设备组成的List
     */
    const QList<DeviceBaseInfo *> &getKeyboardDevices();

    /**
     * @brief setKeyboardInfoFromLshw:设置由lshw获取的键盘信息
     * @param mapInfo:由lshw获取的键盘信息map
     */
    void setKeyboardInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 其它设备相关
    /**
     * @brief addOthersDevice:添加其他设备设备信息
     * @param device:其他设备设备信息
     */
    void addOthersDevice(DeviceOthers *const device);

    /**
     * @brief addOthersDeviceFromHwinfo:添加由hwinfo获取的其他设备信息
     * @param device:其他设备信息
     */
    void addOthersDeviceFromHwinfo(DeviceOthers *const device);

    /**
     * @brief getOthersDevices: 获取所有其他设备
     * @return 所有其他设备组成的List
     */
    const QList<DeviceBaseInfo *> &getOthersDevices();

    /**
     * @brief setOthersDeviceInfoFromLshw:设置由lshw获取的其他设备信息
     * @param mapInfo:由lshw获取的其他设备信息map
     */
    void setOthersDeviceInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 电源设备相关
    /**
     * @brief addPowerDevice:添加电池设备
     * @param device:被添加的电池设备
     */
    void addPowerDevice(DevicePower *const device);

    /**
     * @brief getPowerDevices:获取所有电池设备
     * @return 所有电池设备组成的List
     */
    const QList<DeviceBaseInfo *> &getPowerDevices();

    /**
     * @brief setPowerDaemonInfo:设置电池守护进程信息
     * @param mapInfo:守护进程信息map
     */
    void setPowerDaemonInfo(const QMap<QString, QString> &mapInfo);

    // 打印机设备相关
    /**
     * @brief addPrintDevice:添加打印机设备
     * @param device:被添加的打印机设备
     */
    void addPrintDevice(DevicePrint *const device);

    /**
     * @brief getPrintDevices:获取所有打印机设备
     * @return 所有打印机设备组成的List
     */
    const QList<DeviceBaseInfo *> &getPrintDevices();

    // 其它pci设备相关
    /**
     * @brief addOtherPCIDevice:添加其他PCI设备
     * @param device:被添加的其他PCI设备
     */
    void addOtherPCIDevice(DeviceOtherPCI *const device);
    /**
     * @brief getOtherPCIDevices:获取所有其他PCI设备
     * @return 所有其他PCI设备组成的List
     */
    const QList<DeviceBaseInfo *> &getOtherPCIDevices();

    // 计算机概况相关
    /**
     * @brief addComputerDevice:添加计算机设备信息
     * @param device:被添加的计算机设备信息
     */
    void addComputerDevice(DeviceComputer *const device);

    /**
     * @brief getComputerDevices:获取所有计算机设备
     * @return 所有计算机设备组成的List
     */
    const QList<DeviceBaseInfo *> &getComputerDevices();

    // 获取cdrom设备
    /**
     * @brief addCdromDevice:添加CDrom设备
     * @param device:被添加的CDrom设备
     */
    void addCdromDevice(DeviceCdrom *const device);

    /**
     * @brief getCdromDevices:获取所有CDrom设备
     * @return 所有CDrom设备组成的List
     */
    const QList<DeviceBaseInfo *> getCdromDevices();

    /**
     * @brief addLshwinfoIntoCdromDevice:添加由lshw获取的信息到CDrom设备
     * @param mapInfo:由lshw获取的信息map
     */
    void addLshwinfoIntoCdromDevice(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addBusId:添加总线ID
     * @param busId:被添加的总线ID
     */
    void addBusId(const QStringList &busId);

    /**
     * @brief getBusId: 获取所有的总线ID
     * @return 所有总线ID组成的QStringList
     */
    const QStringList &getBusId();

    /**
     * @brief addCmdInfo:添加命令以及由命令获取的信息解析出的map list
     * @param cmdInfo:命令以及由命令获取的信息解析出的map list
     */
    void addCmdInfo(const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo);

    /**
     * @brief cmdInfo:获取命令key对相应的信息map组成的List
     * @param key:命令值
     * @return 信息map组成的信息List
     */
    const QList<QMap<QString, QString>> &cmdInfo(const QString &key);

    /**
     * @brief exportToTxt:导出到txt
     * @param filePath:文件路径
     * @return true:导出成功，false:导出失败
     */
    bool exportToTxt(const QString &filePath);

    /**
     * @brief exportToXlsx:导出到xlsx
     * @param filePath:文件路径
     * @return true:导出成功，false:导出失败
     */
    bool exportToXlsx(const QString &filePath);

    /**
     * @brief exportToDoc:导出到doc
     * @param filePath:文件路径
     * @return true:导出成功，false:导出失败
     */
    bool exportToDoc(const QString &filePath);

    /**
     * @brief exportToHtml:导出到html
     * @param filePath:文件路径
     * @return true:导出成功，false:导出失败
     */
    bool exportToHtml(const QString &filePath);

    /**
     * @brief currentXlsRow:获取xlsx当前行
     * @return xlsx当前行
     */
    static int currentXlsRow();

    /**
     * @brief overviewToTxt:概况信息写到txt
     * @param out:txt文件流
     */
    void overviewToTxt(QTextStream &out);

    /**
     * @brief overviewToHtml:概况信息写到html
     * @param html:html文件
     */
    void overviewToHtml(QFile &html);

    /**
     * @brief overviewToDoc:概况信息写到doc
     * @param doc:doc文件
     */
    void overviewToDoc(Docx::Document &doc);

    /**
     * @brief overviewToXlsx:概况信息写到表格
     * @param xlsx:xlsx文件
     * @param boldFont:字体格式
     */
    void overviewToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont);

    /**
     * @brief infoToHtml:将信息写到html中
     * @param doc:文本
     * @param key:关键字
     * @param value:值
     */
    void infoToHtml(QDomDocument &doc, const QString &key, const QString &value);

    /**
     * @brief getDeviceOverview:获取所有设备设备概况信息
     * @param overiewMap:所有设备概况Map
     */
    const QMap<QString, QString>  &getDeviceOverview();

    /**
     * @brief getDeviceDriverPool：获取所有设备驱动与设备关联map
     * @return 所有设备的驱动与设备关联map
     */
    const QMap<QString, QMap<QString, QStringList>> &getDeviceDriverPool();

    /**
     * @brief addInputInfo : 管理从 cat /proc/bus/input/devices 获取到的信息
     * @param key
     * @param mapInfo
     */
    void addInputInfo(const QString &key, const QMap<QString, QString> &mapInfo);
    const QMap<QString, QString> &inputInfo(const QString &key);

    // 设备是否存在于蓝牙设备配对信息中
    bool isDeviceExistInPairedDevice(const QString &name);

    /**
     * @brief setCpuNum
     * @param num
     */
    void setCpuNum(int num);

protected:
    DeviceManager();
    ~DeviceManager();

private:
    static DeviceManager    *sInstance;

    QList<DeviceBaseInfo *>              m_ListDeviceMouse;                //<! 鼠标设备
    QList<DeviceBaseInfo *>              m_ListDeviceCPU;                  //<! cpu设备
    QList<DeviceBaseInfo *>              m_ListDeviceStorage;              //<! 存储设备
    QList<DeviceBaseInfo *>              m_ListDeviceGPU;                  //<! gpu设备
    QList<DeviceBaseInfo *>              m_ListDeviceMemory;               //<! memory设备
    QList<DeviceBaseInfo *>              m_ListDeviceMonitor;              //<! monitor设备
    QList<DeviceBaseInfo *>              m_ListDeviceBios;                 //<! bios设备
    QList<DeviceBaseInfo *>              m_ListDeviceBluetooth;            //<! 蓝牙设备
    QList<DeviceBaseInfo *>              m_ListDeviceAudio;                //<! 音频设备
    QList<DeviceBaseInfo *>              m_ListDeviceNetwork;              //<! 网络设备
    QList<DeviceBaseInfo *>              m_ListDeviceImage;                //<! 图像设备
    QList<DeviceBaseInfo *>              m_ListDeviceKeyboard;             //<! 键盘设备
    QList<DeviceBaseInfo *>              m_ListDeviceOthers;               //<! 其它设备
    QList<DeviceBaseInfo *>              m_ListDevicePower;                //<! 电源
    QList<DeviceBaseInfo *>              m_ListDevicePrint;                //<! 打印机
    QList<DeviceBaseInfo *>              m_ListDeviceOtherPCI;             //<! 其它PCI设备
    QList<DeviceBaseInfo *>              m_ListDeviceComputer;             //<! 计算机基本信息
    QList<DeviceBaseInfo *>              m_ListDeviceCdrom;                //<! cdrom设备

    QList<QPair<QString, QString>>       m_ListDeviceType;                 //<! 所有的设备类型及其对应的图标
    QStringList                                    m_BusIdList;            //<! 所有的设备总线ID
    QMap<QString, QList<QMap<QString, QString> > > m_cmdInfo;              //<! 所有设备信息获取命令
    QMap<QString, QString>                         m_OveriewMap;           //<! 所有的设备与其对应概况信息
    QMap<QString, QList<DeviceBaseInfo *>>         m_DeviceClassMap;       //<! 所有的设备类型与其对应设备列表
    QMap<QString, QMap<QString, QStringList>>      m_DeviceDriverPool;     //<! 所有的设备驱动与与其对应的设备类型，设备名称列表
    QMap<QString, QMap<QString, QString> >         m_InputDeviceInfo;

    int                                            m_CpuNum;               //<! 物理cpu个数

    static int m_CurrentXlsRow;       //<! xlsx表格当前行
};

#endif // DEVICEMANAGER_H
