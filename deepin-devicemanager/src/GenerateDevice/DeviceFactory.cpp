// 项目自身文件
#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "KLUGenerator.h"
#include "PanguGenerator.h"
#include "PanguVGenerator.h"
#include "commonfunction.h"

// Qt库文件
#include <QProcess>
#include <QFile>

DeviceFactory::DeviceFactory()
{

}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    // 获取系统架构
    QString arch = Common::getArch();

    // 根据架构创建设备信息生成器
    DeviceGenerator *generator = nullptr;
    if (arch == "x86_64")
        generator = new X86Generator();
    else if (arch == "mips64")
        generator = new MipsGenerator();
    else if (arch == "aarch64")
        generator = new ArmGenerator();
    else
        generator = new X86Generator();

    return generator;
}
