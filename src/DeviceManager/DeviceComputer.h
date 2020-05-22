#ifndef DEVICECOMPUTER_H
#define DEVICECOMPUTER_H
#include"DeviceInfo.h"


class DeviceComputer : public DeviceBaseInfo
{
public:
    DeviceComputer();

    /**@brief:获取从 cat /etc/os-release 中获取的信息 */
    void setInfoFromCatOsRelease(const QString &info);

    /**@brief:获取从 cat /proc/boradinfo 中获取的信息 */
    void setInfoFromCatBoardInfo(const QString &info);

    /**@brief:获取从 lsb_release -a 中获取的信息 */
    void setUOSInfo(const QString &info);

    /**@brief:获取从 cat /proc/version 中获取的信息 */
    void setInfoFromCatDevice(const QString &info);

    /**@brief:获取从 sudo dmidecode -t Chassis 中获取的信息 */
    void setChassisInfoFromDmidecode(const QString &info);

    /**@brief:获取从 lshw 中获取的信息*/
    void setInfoFromLshw(const QString &info);

    const QString &homeUrl()const;
    const QString &osDescription();
    const QString &os();
    const QString &vendor();
    const QString &name();
    const QString &type();
    void setHomeUrl(const QString &value);
    void setOsDescription(const QString &value);
    void setOS(const QString &value);
    void setVendor(const QString &value);
    void setName(const QString &value);
    void setType(const QString &value);

protected:
    void initFilterKey() override;

private:
    QString            m_HomeUrl;
    QString            m_OsDescription;
    QString            m_OS;
    QString            m_Vendor;
    QString            m_Name;
    QString            m_Type;
};

#endif // DEVICECOMPUTER_H
