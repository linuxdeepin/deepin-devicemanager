#ifndef AUTHORIZEDENABLE_H
#define AUTHORIZEDENABLE_H

#include "DeviceEnable.h"

class AuthorizedEnable : public DeviceEnable
{
    Q_OBJECT
    Q_DISABLE_COPY(AuthorizedEnable)
public:
    AuthorizedEnable();

    /**
     * @brief enable 根据唯一标识启用设备
     * @param key 启用的设备的唯一标识
     * @return 返回是否成功
     */
    bool enable(const QString& key) override;

    /**
     * @brief enable 根据唯一标识禁用设备
     * @param key 禁用的设备的唯一标识
     * @return 返回是否成功
     */
    bool disable(const QString& key) override;
};

#endif // AUTHORIZEDENABLE_H
