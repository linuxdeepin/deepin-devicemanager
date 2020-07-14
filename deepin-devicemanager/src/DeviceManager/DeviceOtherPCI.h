#ifndef DEVICEOTHERPCI_H
#define DEVICEOTHERPCI_H
#include"DeviceInfo.h"

class DeviceOtherPCI : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceOtherPCI)
public:
    DeviceOtherPCI();

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const;

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
       * @brief width获取位宽属性值
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
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver()const;

    /**
     * @brief latency:获取latency属性值
     * @return QString:latency属性值
     */
    const QString &latency()const;

    /**
     * @brief inputOutput:获取inputOutput属性值
     * @return QString:inputOutput属性值
     */
    const QString &inputOutput()const;

protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
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
