#ifndef DEVICECOMPUTER_H
#define DEVICECOMPUTER_H
#include"DeviceInfo.h"


class DeviceComputer : public DeviceBaseInfo
{
public:
    DeviceComputer();

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

    void setVendor(const QString &dm1Vendor, const QString &dm2Vendor);
    void setName(const QString &dm1Name, const QString &dm2Name, const QString &dm1Family, const QString &dm1Version);

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
