/**
 * @brief: A class used to describe audio devices, an object of DeviceAudio is an audio device
 * @author: Jun.Liu  liujuna@uniontech.com
 * @maintainer : XiaoMei.Ji  jixiaomei@uniontech.com
 * @date: 2020.5
*/

#ifndef DEVICEAUDIO_H
#define DEVICEAUDIO_H
#include"DeviceInfo.h"


class DeviceAudio : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceAudio)
public:
    DeviceAudio();

    /**
       * @brief setInfoFromHwinfo:设置由hwinfo --sound命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromLshw:设置从lshw里面获取的信息
       * @param mapInfo:由lshw获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromCatDevices:设置从cat /proc/bus/input/devices里面获取的信息
       * @param mapInfo:由cat /proc/bus/input/devices获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromCatDevices(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromCatAudio:设置从cat /proc/asound/card0/codec#0里面获取的信息 , KLU专用
       * @param mapInfo:由cat /proc/asound/card0/codec#0获取的信息map
       */
    void setInfoFromCatAudio(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setAudioChipFromDmesg:设置从dmesg里面获取的有关声卡型号信息
       * @param mapInfo:由dmesg获取的有关声卡型号信息
       */
    bool setAudioChipFromDmesg(const QString &info);

    /**
       * @brief name:获取名称属性值
       * @return QString:名称属性值
       */
    const QString &name()const override;

    /**
     * @brief driver
     * @return
     */
    const QString &driver()const override;

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    EnableDeviceStatus setEnable(bool enable) override;

    /**
     * @brief enable : 判断当前是否是禁用状态
     * @return
     */
    bool enable() override;

    /**
       * @brief vendor:获取制造商属性值
       * @return QString:制造商属性值
       */
    const QString &vendor()const;

    /**
       * @brief model:获取型号属性值
       * @return QString:型号属性值
       */
    const QString &model()const;

    /**
       * @brief version:获取版本号属性值
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
       * @brief busInfo:获取总线信息属性值
       * @return QString:总线信息属性值
       */
    const QString &busInfo()const;

    /**
       * @brief irq:获取中断属性值
       * @return QString:中断属性值
       */
    const QString &irq()const;

    /**
       * @brief memory:获取内存地址属性值
       * @return QString:内存地址属性值
       */
    const QString &memory()const;

    /**
       * @brief width:获取位宽属性值
       * @return QString:位宽属性值
       */
    const QString &width()const;

    /**
       * @brief clock:获取时钟频率属性值
       * @return QString:时钟频率属性值
       */
    const QString &clock()const;

    /**
       * @brief capabilities:获取功能属性值
       * @return QString:功能属性值
       */
    const QString &capabilities()const;

    /**
       * @brief description:获取描述属性值
       * @return QString:描述属性值
       */
    const QString &description()const;

    /**
       * @brief chip:获取芯片型号属性值
       * @return QString:芯片属性值
       */
    const QString &chip() const;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    QString subTitle() override;

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    const QString getOverviewInfo() override;

protected:

    /**
       * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
       */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

    /**
     * @brief loadOtherDeviceInfo:加载基本信息
     */
    void loadOtherDeviceInfo() override;

    /**
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;



private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Description;                  //<! 【描述】
    QString             m_Chip;                         //<! 【芯片型号】
    QString             m_Driver;                       //<! 【驱动】

    QString             m_UniqueKey;                    //<! 【标识唯一设备】
};

#endif // DEVICEAUDIO_H
