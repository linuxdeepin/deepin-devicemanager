#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include <QProcess>
#include <QFile>
#include "../commondefine.h"
DeviceFactory::DeviceFactory()
{

}
DeviceGenerator *DeviceFactory::s_Generator = nullptr;
DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    QProcess process;
    process.start("uname -m");
    process.waitForFinished(1000);
    QString struction = process.readAllStandardOutput().trimmed();
    process.exitCode();

#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + "uname_m.txt");
    bool res = inputDeviceFile.open(QIODevice::ReadOnly);
    if (res) {
        struction = inputDeviceFile.readAll().trimmed();
    } else {
        struction = "x86_64";
    }
    inputDeviceFile.close();
#endif


    if (!s_Generator) {
        if (struction == "x86_64")
            s_Generator = new X86Generator();
        else if (struction == "mips64") {
            s_Generator = new MipsGenerator();
        } else if (struction == "aarch64") {
            s_Generator = new ArmGenerator();
        } else {
            s_Generator = new X86Generator();
        }
    }
    return s_Generator;
}
