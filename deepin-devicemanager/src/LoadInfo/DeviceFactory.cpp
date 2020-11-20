// 项目自身文件
#include "DeviceFactory.h"

// Qt库文件
#include <QProcess>
#include <QFile>

// 其它头文件
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "KLUGenerator.h"
#include "PanguGenerator.h"
#include "PanguVGenerator.h"
#include "../commondefine.h"

QString DeviceFactory::s_GeneratorKey = "";

DeviceFactory::DeviceFactory()
{

}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    // 根据架构创建设备信息生成器
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
        }  else if (s_GeneratorKey == "PanGu") {
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
    // 设置机器架构
    s_GeneratorKey = key;
}
