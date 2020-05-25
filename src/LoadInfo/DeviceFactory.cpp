#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
DeviceFactory::DeviceFactory()
{

}
DeviceGenerator *DeviceFactory::s_Generator = nullptr;
DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    if (!s_Generator) {
        s_Generator = new X86Generator();
    }
    return s_Generator;
}
