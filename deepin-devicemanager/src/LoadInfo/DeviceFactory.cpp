#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "KLUGenerator.h"
#include "KLVGenerator.h"
#include "PanguGenerator.h"
#include "PanguVGenerator.h"
#include <QProcess>
#include <QFile>
#include "../commondefine.h"

QString DeviceFactory::s_GeneratorKey = "";

DeviceFactory::DeviceFactory()
{

}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    DeviceGenerator *generator = nullptr;
    if (!generator) {
        if (s_GeneratorKey == "x86_64")
            generator = new X86Generator();
        else if (s_GeneratorKey == "mips64") {
            generator = new MipsGenerator();
        } else if (s_GeneratorKey == "aarch64") {
            generator = new ArmGenerator();
        }  else if (s_GeneratorKey == "KLU") {
            generator = new KLUGenerator();
        } else if (s_GeneratorKey == "KLV") {
            generator = new KLVGenerator();
        } else if (s_GeneratorKey == "PanGu") {
            generator = new PanguGenerator();
        }  else if (s_GeneratorKey == "PanGuV") {
            generator = new PanguVGenerator();
        } else {
            generator = new X86Generator();
        }
    }
    return generator;
}

void DeviceFactory::setGeneratorKey(const QString &key)
{
    s_GeneratorKey = key;
}
