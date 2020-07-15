#ifndef DEVICECOMPUTER_H
#define DEVICECOMPUTER_H
#include "DeviceInfo.h"


class DeviceComputer : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceComputer)
public:
    DeviceComputer();

    /**
     * @brief homeUrl:获取URL属性值
     * @return QString:URL属性值
     */
    const QString &homeUrl()const;

    /**
     * @brief osDescription:获取操作系统描述属性值
     * @return QString:操作系统描述属性值
     */
    const QString &osDescription();

    /**
     * @brief os:获取操作系统属性值
     * @return QString:操作系统属性值
     */
    const QString &os();

    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor();

    /**
     * @brief name:获取名称属性值
     * @return QString:名称属性值
     */
    const QString &name();

    /**
     * @brief type:获取类型属性值
     * @return QString:类型属性值
     */
    const QString &type();

    /**
     * @brief setHomeUrl:设置URL属性值
     * @param value:URL属性值
     */
    void setHomeUrl(const QString &value);

    /**
     * @brief setOsDescription:设置操作系统描述属性值
     * @param value:操作系统描述属性值
     */
    void setOsDescription(const QString &value);

    /**
     * @brief setOS:设置操作系统属性值
     * @param value:操作系统属性值
     */
    void setOS(const QString &value);

    /**
     * @brief setVendor:设置制造商属性值
     * @param value:制造商属性值
     */
    void setVendor(const QString &value);

    /**
     * @brief setName:设置名称属性值
     * @param value:名称属性值
     */
    void setName(const QString &value);

    /**
     * @brief setType:设置类型属性值
     * @param value:类型属性值
     */
    void setType(const QString &value);

    /**
     * @brief setVendor:设置制造商属性值
     * @param dm1Vendor:制造商属性值1
     * @param dm2Vendor:制造商属性值2
     */
    void setVendor(const QString &dm1Vendor, const QString &dm2Vendor);

    /**
     * @brief setName:设置名称属性值
     * @param dm1Name:名称属性值1
     * @param dm2Name:名称属性值1
     * @param dm1Family:family属性值
     * @param dm1Version:版本属性值
     */
    void setName(const QString &dm1Name, const QString &dm2Name, const QString &dm1Family, const QString &dm1Version);

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

private:
    QString            m_HomeUrl;
    QString            m_OsDescription;
    QString            m_OS;
    QString            m_Vendor;
    QString            m_Name;
    QString            m_Type;
};

#endif // DEVICECOMPUTER_H
