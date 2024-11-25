// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "PanguGenerator.h"
#include "PanguVGenerator.h"
#include "HWGenerator.h"
#include "commonfunction.h"
#include "PanguxGenerator.h"
#include "DDLog.h"

// Qt库文件
#include <QProcess>
#include <QFile>

using namespace DDLog;

DeviceFactory::DeviceFactory()
{
    qCDebug(appLog) << "DeviceFactory constructor";
}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    qCDebug(appLog) << "DeviceFactory::getDeviceGenerator start";
    // 获取系统架构
    QString arch = Common::getArch();

    // 根据架构创建设备信息生成器
    DeviceGenerator *generator = nullptr;
    if (arch == "x86_64") {
        qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create X86Generator";
        generator = new X86Generator();
    } else if (arch == "mips64") {
        qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create MipsGenerator";
        generator = new MipsGenerator();
    } else if (arch == "aarch64") {
        QString type = Common::boardVendorType();
        qCDebug(appLog) << "DeviceFactory::getDeviceGenerator arch is aarch64, type is" << type;
        if (!type.isEmpty()) {
            if (type == "KLVV") {
                qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create KLVGenerator";
                generator = new HWGenerator();
            } else if (type == "PGUV" || type == "PGUW") {
                qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create PanguVGenerator";
                generator = new PanguVGenerator();
            } else if (type == "KLVU") {
                qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create KLUGenerator";
                generator = new HWGenerator();
            } else if (type == "PGUX") {
                qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create PanguXGenerator";
                generator = new PanguXGenerator();
            } else {
                qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create HWGenerator";
                generator = new HWGenerator();
            }
        } else {
            qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create ArmGenerator";
            generator = new ArmGenerator();
        }
    } else {
        qCDebug(appLog) << "DeviceFactory::getDeviceGenerator create X86Generator by default";
        generator = new X86Generator();
    }

    qCDebug(appLog) << "DeviceFactory::getDeviceGenerator end, arch:" << Common::getArch() << "type:" << Common::boardVendorType() << "generator:" << generator;
    return generator;
}
