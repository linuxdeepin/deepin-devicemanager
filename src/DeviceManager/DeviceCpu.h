/**
* @brief: CPU设备信息
* @class[DeviceCPU]: DeviceCPU 表示的就是一个CPU设备
* @author: liujun
* @date:2020-04-09
*/
#ifndef DEVICECPU_H
#define DEVICECPU_H
#include"DeviceInfo.h"


class DeviceCpu : public DeviceBaseInfo
{
public:
    DeviceCpu();

    /**@brief:设置cpu信息*/
    void setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, const QMap<QString, QString> &catInfo);

    /**@brief:获取各种属性*/
    const QString &vendor() const;
    const QString &name() const;
    const QString &physicalID() const;
    const QString &coreID() const;
    const QString &threadNum() const;
    const QString &frequency() const;
    const QString &curFrequency() const;
    const QString &bogoMIPS() const;
    const QString &architecture() const;
    const QString &familly() const;
    const QString &model() const;
    const QString &step() const;
    const QString &cacheL1Data() const;
    const QString &cacheL1Order() const;
    const QString &cacheL2() const;
    const QString &cacheL3() const;
    const QString &extensions() const;
    const QString &flags() const;
    const QString &hardwareVirtual() const;
    const QString &logicalCPUNum() const;
    const QString &cpuCores() const;

protected:
    void initFilterKey() override;

private:
    void setInfoFromLscpu(const QMap<QString, QString> &mapInfo);
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);
    void setInfoFromDmidecode(const QMap<QString, QString> &mapInfo);
    void setInfoFromCatCpuinfo(const QMap<QString, QString> &mapInfo);

private:
    QString           m_Vendor;             //<! 制造商
    QString           m_Name;               //<! 名称
    QString           m_PhysicalID;         //<! 处理器ID
    QString           m_CoreID;             //<! 核心ID
    QString           m_ThreadNum;          //<! 线程数
    QString           m_Frequency;          //<! 频率
    QString           m_CurFrequency;       //<! 当前频率
    QString           m_BogoMIPS;           //<! BogoMIPS
    QString           m_Architecture;       //<! 架构
    QString           m_Familly;            //<! 家族
    QString           m_Model;              //<! 型号
    QString           m_Step;               //<! 步进
    QString           m_CacheL1Data;        //<! L1缓存（数据）
    QString           m_CacheL1Order;       //<! L1缓存（指令）
    QString           m_CacheL2;            //<! L2缓存
    QString           m_CacheL3;            //<! L3缓存
    QString           m_Extensions;         //<! 扩展指令集
    QString           m_Flags;              //<! 特性
    QString           m_HardwareVirtual;    //<! 硬件虚拟化

    // 这个是cpu的逻辑处理器个数，而不是core的逻辑处理器个数
    QString           m_LogicalCPUNum;      //<! 逻辑处理器个数
    QString           m_CPUCoreNum;         //<! cpu核数
};

#endif // DEVICECPU_H
