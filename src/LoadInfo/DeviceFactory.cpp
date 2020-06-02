#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "KLUGenerator.h"
#include <QProcess>
#include <QFile>
#include "../commondefine.h"
DeviceFactory::DeviceFactory()
{

}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
//    QProcess process;
//    process.start("uname -m");
//    process.waitForFinished(1000);
//    QString struction = process.readAllStandardOutput().trimmed();
//    process.exitCode();

//#ifdef TEST_DATA_FROM_FILE
//    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + "uname_m.txt");
//    bool res = inputDeviceFile.open(QIODevice::ReadOnly);
//    if (res) {
//        struction = inputDeviceFile.readAll().trimmed();
//    } else {
//        struction = "x86_64";
//    }
//    inputDeviceFile.close();
//#endif

//    DeviceGenerator *generator = nullptr;
//    if (!generator) {
//        if (struction == "x86_64")
//            generator = new X86Generator();
//        else if (struction == "mips64") {
//            generator = new MipsGenerator();
//        } else if (struction == "aarch64") {
//            generator = new ArmGenerator();
//        } else {
//            generator = new X86Generator();
//        }
//    }
//    return generator;

    return new KLUGenerator();
}
