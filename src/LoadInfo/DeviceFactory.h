#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include<QString>
class DeviceGenerator;


class DeviceFactory
{
public:
    static DeviceGenerator *getDeviceGenerator();
    static void setGeneratorKey(const QString &key);
protected:
    DeviceFactory();

private:
    static QString s_GeneratorKey;
};

#endif // DEVICEFACTORY_H
