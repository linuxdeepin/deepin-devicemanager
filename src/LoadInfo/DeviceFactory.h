#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H
class DeviceGenerator;


class DeviceFactory
{
public:
    static DeviceGenerator *getDeviceGenerator();
protected:
    DeviceFactory();
};

#endif // DEVICEFACTORY_H
