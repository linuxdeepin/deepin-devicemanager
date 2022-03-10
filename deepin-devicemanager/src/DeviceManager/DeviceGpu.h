#ifndef DEVICEGPU_H
#define DEVICEGPU_H
#include"DeviceInfo.h"

class DeviceGpu: public DeviceBaseInfo
{
public:
    DeviceGpu();

    /**@brief:设置从lshw中获取的信息*/
    void setLshwInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从hwinfo中获取的信息*/
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从xrandr中获取的信息*/
    void setXrandrInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从dmesg中获取的信息*/
    void setDmesgInfo(const QString &info);

    /**@brief:设置从gpuinfo命令中获取的信息，华为KLU笔记本和PanGuV机器专用*/
    void setGpuInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:获取设备信息*/
    const QString &name() const;
    const QString &vendor() const;
    const QString &model() const;
    const QString &version() const;
    const QString &graphicsMemory() const;
    const QString &busWidth() const;
    const QString &IRQ() const;
    const QString &capabilities() const;
    const QString &displayOutput() const;
    const QString &VGA() const;
    const QString &HDMI() const;
    const QString &eDP() const;
    const QString &displayPort() const;
    const QString &DVI() const;
    const QString &clock() const;
    const QString &description() const;
    const QString &driver() const;
    const QString &currentResolution() const;
    const QString &minimumResolution() const;
    const QString &maximumResolution() const;
    const QString &busInfo() const;
    const QString &ioPort() const;
    const QString &memory() const;
    const QString &physID() const;

protected:
    void initFilterKey() override;
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
    QString            m_DVI;
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
