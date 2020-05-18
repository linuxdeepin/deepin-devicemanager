#ifndef DEVICEOTHERPCI_H
#define DEVICEOTHERPCI_H
#include"DeviceInfo.h"

class DeviceOtherPCI :public DeviceBaseInfo
{
public:
    DeviceOtherPCI();

    // 其他pci设备
    const QString& name()const;
    const QString& vendor()const;
    const QString& model()const;
    const QString& version()const;
    const QString& busInfo()const;
    const QString& irq()const;
    const QString& memory()const;
    const QString& width()const;
    const QString& clock()const;
    const QString& capabilities()const;
    const QString& description()const;
    const QString& driver()const;
    const QString& latency()const;
    const QString& inputOutput()const;

protected:
    void initFilterKey();
private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Description;                  //<! 【描述】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Driver;                       //<! 【驱动】
    QString             m_Latency;                      //<! 【延迟】
    QString             m_InputOutput;                  //<! 【输入输出】
};

#endif // DEVICEOTHERPCI_H
