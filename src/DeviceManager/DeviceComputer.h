#ifndef DEVICECOMPUTER_H
#define DEVICECOMPUTER_H
#include"DeviceInfo.h"


class DeviceComputer : public DeviceBaseInfo
{
public:
    DeviceComputer();

    /**@brief:获取从 cat /etc/os-release 中获取的信息 */
    void setInfoFromCatOsRelease(const QString& info);

    /**@brief:获取从 cat /proc/boradinfo 中获取的信息 */
    void setInfoFromCatBoardInfo(const QString& info);

    /**@brief:获取从 lsb_release -a 中获取的信息 */
    void setUOSInfo(const QString& info);

    /**@brief:获取从 cat /proc/version 中获取的信息 */
    void setInfoFromCatDevice(const QString& info);

    /**@brief:获取从 sudo dmidecode -t system 中获取的信息 */
    void setInfoFromDmidecode(const QString& info);

    /**@brief:获取从 sudo dmidecode -t Chassis 中获取的信息 */
    void setChassisInfoFromDmidecode(const QString& info);

    /**@brief:获取从 lshw 中获取的信息*/
    void setInfoFromLshw(const QString& info);

    // 获取属性 cat /etc/os-release
    const QString& prettyName()const;
    const QString& name()const;
    const QString& osVersionID()const;
    const QString& osVersion()const;
    const QString& id()const;
    const QString& bugReportUrl()const;
    const QString& homeUrl()const;

    // 获取属性 lsb_release -a
    const QString& distributorID();
    const QString& description();
    const QString& release();
    const QString& codeName();

    // 获取属性 cat /proc/version
    const QString& os();

    // 获取属性 sudo dmidecode -t system
    const QString& vendor();
    const QString& prodectName();
    const QString& version();
    const QString& family();
    const QString& type();

protected:
    void initFilterKey() override;

private:

    // 从cat /etc/os-release里面获取的内容
    QString            m_PrettyName;
    QString            m_Name;
    QString            m_OsVersionID;
    QString            m_OsVersion;
    QString            m_ID;
    QString            m_BugReportUrl;
    QString            m_HomeUrl;

    // 从 lsb_release -a 里面获取的内容
    QString            m_DistributorID;
    QString            m_Description;
    QString            m_Release;
    QString            m_CodeName;

    // 从 cat /proc/version 里面获取内容
    QString            m_OS;

    // 从 sudo dmidecode -t system 里面获取内容
    QString            m_Vendor;
    QString            m_ProductName;
    QString            m_Version;
    QString            m_Family;

    QString            m_Type;
};

#endif // DEVICECOMPUTER_H
