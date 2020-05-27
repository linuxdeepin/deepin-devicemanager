#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H
class DeviceGenerator;


class DeviceFactory
{
public:
    static DeviceGenerator *getDeviceGenerator();
    static void clear();
protected:
    DeviceFactory();
private:
    static DeviceGenerator *s_Generator;
};

#endif // DEVICEFACTORY_H
