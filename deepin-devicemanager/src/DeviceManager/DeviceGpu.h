#ifndef DEVICEGPU_H
#define DEVICEGPU_H
#include"DeviceInfo.h"

class DeviceGpu: public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceGpu)
public:
    DeviceGpu();

    /**
     * @brief setLshwInfo:设置从lshw里面获取的信息
     * @param mapInfo:由lshw获取的信息map
     */
    void setLshwInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setHwinfoInfo:设置由hwinfo --display命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setXrandrInfo:设置从xrandr中获取的信息
     * @param mapInfo:由xrandr获取的信息map
     */
    void setXrandrInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setDmesgInfo:设置从dmesg中获取的显存信息
     * @param info: dmesg中获取的显存信息
     */
    void setDmesgInfo(const QString &info);

    /**
       * @brief setGpuInfo:设置从gpuinfo命令中获取的信息，华为KLU笔记本和PanGuV机器专用
       * @param mapInfo:由gpuinfo获取的信息map
       */
    void setGpuInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name() const;

    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor() const;

    /**
     * @brief model:获取型号属性值
     * @return QString:型号属性值
     */
    const QString &model() const;

    /**
       * @brief version:获取版本号属性值
       * @return QString:版本号属性值
       */
    const QString &version() const;

    /**
     * @brief graphicsMemory:获取显存属性值
     * @return QString:显存属性值
     */
    const QString &graphicsMemory() const;

    /**
     * @brief busWidth:获取位宽属性值
     * @return QString:位宽属性值
     */
    const QString &busWidth() const;

    /**
     * @brief IRQ:获取中断属性值
     * @return QString:中断属性值
     */
    const QString &IRQ() const;

    /**
       * @brief capabilities:获取功能属性值
       * @return QString:功能属性值
       */
    const QString &capabilities() const;

    /**
     * @brief displayOutput:获取显示输出属性值
     * @return QString:显示输出属性值
     */
    const QString &displayOutput() const;

    /**
     * @brief VGA:获取VGA属性值
     * @return QString:VGA属性值
     */
    const QString &VGA() const;

    /**
     * @brief HDMI:获取HDMI属性值
     * @return QString:HDMI属性值
     */
    const QString &HDMI() const;

    /**
     * @brief eDP:获取eDP属性值
     * @return QString:eDP属性值
     */
    const QString &eDP() const;

    /**
     * @brief displayPort:获取DisplayPort属性值
     * @return QString:DisplayPort属性值
     */
    const QString &displayPort() const;

    /**
     * @brief clock:获取时钟属性值
     * @return QString:时钟属性值
     */
    const QString &clock() const;

    /**
     * @brief description:获取描述属性值
     * @return QString:描述属性值
     */
    const QString &description() const;

    /**
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver() const;

    /**
     * @brief currentResolution:获取当前分辨率属性值
     * @return QString:当前分辨率属性值
     */
    const QString &currentResolution() const;

    /**
     * @brief minimumResolution:获取最小支持分辨率属性值
     * @return QString:最小支持分辨率属性值
     */
    const QString &minimumResolution() const;

    /**
     * @brief maximumResolution:获取最大支持分辨率属性值
     * @return QString:最大支持分辨率属性值
     */
    const QString &maximumResolution() const;

    /**
     * @brief busInfo:获取总线信息属性值
     * @return QString:总线信息属性值
     */
    const QString &busInfo() const;

    /**
     * @brief ioPort:获取ioport属性值
     * @return QString:ioport属性值
     */
    const QString &ioPort() const;

    /**
     * @brief memory:获取内存地址属性值
     * @return QString:内存地址属性值
     */
    const QString &memory() const;

    /**
     * @brief physID:获取物理ID属性值
     * @return QString:物理ID属性值
     */
    const QString &physID() const;

protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

private:
    QString            m_Name;                    //<! 【名称】
    QString            m_Vendor;                  //<! 【制造商】
    QString            m_Model;                   //<! 【型号】
    QString            m_Version;                 //<! 【版本】
    QString            m_GraphicsMemory;          //<! 【显存】
    QString            m_Width;                   //<! 【位宽】
    QString            m_DisplayPort;             //<! 【DisplayPort】
    QString            m_Clock;                   //<! 【时钟】
    QString            m_IRQ;                     //<! 【中断】
    QString            m_Capabilities;            //<! 【功能】
    QString            m_DisplayOutput;           //<! 【显示输出】
    QString            m_VGA;                     //<! 【VGA】
    QString            m_HDMI;                    //<! 【HDMI】
    QString            m_eDP;
    QString            m_Description;             //<! 【描述】
    QString            m_Driver;                  //<! 【驱动】
    QString            m_CurrentResolution;       //<! 【当前分辨率】
    QString            m_MinimumResolution;       //<! 【最小分辨率】
    QString            m_MaximumResolution;       //<! 【最大分辨率】
    QString            m_Type;                    //<! 【类型】
    QString            m_BusInfo;                 //<! 【总线信息】
    QString            m_IOPort;                  //<! 【ioport】
    QString            m_MemAddress;              //<! 【内存地址】
    QString            m_PhysID;                  //<! 【物理ID】

    QString            m_UniqueKey;               //<! 【标识一个适配器的唯一标识】
};

#endif // DEVICEGPU_H
