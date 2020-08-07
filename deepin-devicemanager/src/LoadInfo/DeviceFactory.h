
#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include<QString>
class DeviceGenerator;


class DeviceFactory
{
public:
    /**
     * @brief getDeviceGenerator:获取设备生成器指针
     * @return  设备生成器指针:
     */
    static DeviceGenerator *getDeviceGenerator();

    /**
     * @brief setGeneratorKey:设置生成器的关键字
     * @param key:关键字
     */
    static void setGeneratorKey(const QString &key);

protected:
    DeviceFactory();

private:
    static QString s_GeneratorKey;
};

#endif // DEVICEFACTORY_H
