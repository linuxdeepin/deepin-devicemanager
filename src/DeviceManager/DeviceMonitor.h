#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H
#include "DeviceInfo.h"


class DeviceMonitor : public DeviceBaseInfo
{

public:
    DeviceMonitor();

    // 设置从hwinfo中获取的信息
    void setInfoFromHwinfo(QMap<QString, QString> mapInfo);

    // 设置从xrandr中获取的信息
    bool setInfoFromXradr(const QString &main, const QString &edid);

    // 设置从xrandr中获取的信息
    bool setCurrentResolution(const QString &resolution, const QString &rate);

    // 将年周转化为年月
    QString transWeekToDate(const QString &year, const QString &week);

    // 获取屏幕大小英寸
    QString parseMonitorSize(const QString &sizeDescription, double &inch, QSize &retSize);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &displayInput()const;
    const QString &VGA()const;
    const QString &HDMI()const;
    const QString &DVI()const;
    const QString &interface()const;
    const QString &screenSize()const;
    const QString &aspectRatio()const;
    const QString &mainScreen()const;
    const QString &currentResolution()const;
    const QString &serialNumber()const;
    const QString &productionWeek()const;
    const QString &supportResolutions()const;

protected:
    void initFilterKey() override;

private:
    /**@brief:设置是否主显示屏幕等信息*/
    bool setMainInfoFromXrandr(const QString &info);

    /**@brief:计算屏幕比例*/
    void caculateScreenRatio();
    int gcd(int a, int b); // 和计算屏幕比例有关，拷贝过来的
    bool findAspectRatio(int width, int height, int &ar_w, int &ar_h);// 和计算屏幕比例有关，拷贝过来的

    /**@brief:计算屏幕大小*/
    void caculateScreenSize();

    /**@brief:根据edid计算屏幕大小*/
    void caculateScreenSize(const QString &edid);



private:
    QString         m_Name;              //<! 【名称】
    QString         m_Vendor;            //<! 【制造商】
    QString         m_Model;             //<! 【型号】
    QString         m_DisplayInput;      //<! 【显示输入】
    QString         m_VGA;               //<! 【VGA】
    QString         m_HDMI;              //<! 【HDMI】
    QString         m_DVI;               //<! 【DVI】
    QString         m_Interface;         //<! 【显示屏借口类型】
    QString         m_ScreenSize;        //<! 【屏幕尺寸】
    QString         m_AspectRatio;       //<! 【显示比例】
    QString         m_MainScreen;        //<! 【主显示器】
    QString         m_CurrentResolution; //<! 【当前分辨率】
    QString         m_SerialNumber;      //<! 【流水号】
    QString         m_ProductionWeek;    //<! 【生产年月】
    QString         m_SupportResolution; //<!  支持的屏幕分辨率

    int             m_Width;             //<!  屏幕的宽度
    int             m_Height;            //<!  屏幕的高度
};

#endif // DEVICEMONITOR_H
