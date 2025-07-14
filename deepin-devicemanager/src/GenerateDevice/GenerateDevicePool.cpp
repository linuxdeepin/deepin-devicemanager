// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GenerateDevicePool.h"

#include <QDateTime>
#include <QLoggingCategory>

#include "DeviceGenerator.h"
#include "DeviceFactory.h"
#include "DeviceManager.h"
#include "DDLog.h"

using namespace DDLog;

GenerateTask::GenerateTask(DeviceType deviceType)
    : m_Type(deviceType)
{
    qCDebug(appLog) << "GenerateTask constructor, type:" << m_Type;
}

GenerateTask::~GenerateTask()
{
    qCDebug(appLog) << "GenerateTask destructor";
}

void GenerateTask::run()
{
    qCDebug(appLog) << "GenerateTask::run start, type:" << m_Type;

    DeviceGenerator *generator = DeviceFactory::getDeviceGenerator();

    if (!generator) {
        qCWarning(appLog) << "GenerateTask::run get generator failed";
        return;
    }

    switch (m_Type) {
    case DT_Computer:
        qCDebug(appLog) << "GenerateTask::run generate computer device";
        generator->generatorComputerDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Cpu:
        qCDebug(appLog) << "GenerateTask::run generate cpu device";
        generator->generatorCpuDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Bios:
        qCDebug(appLog) << "GenerateTask::run generate bios device";
        generator->generatorBiosDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Memory:
        qCDebug(appLog) << "GenerateTask::run generate memory device";
        generator->generatorMemoryDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Storage:
        qCDebug(appLog) << "GenerateTask::run generate storage device";
        generator->generatorDiskDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Gpu:
        qCDebug(appLog) << "GenerateTask::run generate gpu device";
        generator->generatorGpuDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Monitor:
        qCDebug(appLog) << "GenerateTask::run generate monitor device";
        generator->generatorMonitorDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Network:
        qCDebug(appLog) << "GenerateTask::run generate network device";
        generator->generatorNetworkDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Audio:
        qCDebug(appLog) << "GenerateTask::run generate audio device";
        generator->generatorAudioDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Bluetoorh:
        qCDebug(appLog) << "GenerateTask::run generate bluetooth device";
        generator->generatorBluetoothDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Keyboard:
        qCDebug(appLog) << "GenerateTask::run generate keyboard device";
        generator->generatorKeyboardDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Mouse:
        qCDebug(appLog) << "GenerateTask::run generate mouse device";
        generator->generatorMouseDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Print:
        qCDebug(appLog) << "GenerateTask::run generate print device";
        generator->generatorPrinterDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Image:
        qCDebug(appLog) << "GenerateTask::run generate image device";
        generator->generatorCameraDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Cdrom:
        qCDebug(appLog) << "GenerateTask::run generate cdrom device";
        generator->generatorCdromDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Others:
        qCDebug(appLog) << "GenerateTask::run generate others device";
        generator->generatorOthersDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Power:
        qCDebug(appLog) << "GenerateTask::run generate power device";
        generator->generatorPowerDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    default:
        qCDebug(appLog) << "GenerateTask::run generate unknown device";
        break;
    }

    emit finished(generator->getBusIDFromHwinfo());
    delete generator;
    generator = nullptr;
}


GenerateDevicePool::GenerateDevicePool()
    : QThreadPool()
{
    qCDebug(appLog) << "GenerateDevicePool constructor";
    initType();
}

void GenerateDevicePool::generateDevice()
{
    qCDebug(appLog) << "GenerateDevicePool::generateDevice start";
    m_FinishedGenerator = 0;

    QList<DeviceType>::iterator it = m_TypeList.begin();
    for (; it != m_TypeList.end(); ++it) {
        // qCDebug(appLog) << "GenerateDevicePool::generateDevice start task for type:" << (*it);
        GenerateTask *task = new GenerateTask((*it));
        connect(task, &GenerateTask::finished, this, &GenerateDevicePool::slotFinished);
        QThreadPool::globalInstance()->start(task);
//        task->setAutoDelete(true);
    }

    // 当所有设备执行完毕之后，开始执行生成其它设备的任务
    // 这里是为了确保其它设备在最后一个生成
    qint64 beginMSecond = QDateTime::currentMSecsSinceEpoch();
    while (true) {
        // qCDebug(appLog) << "GenerateDevicePool::generateDevice wait for tasks finished, finished count:" << m_FinishedGenerator;
        qint64 curMSecond = QDateTime::currentMSecsSinceEpoch();
        if (m_FinishedGenerator == m_TypeList.size()  || curMSecond - beginMSecond > 4000) {
            // qCDebug(appLog) << "GenerateDevicePool::generateDevice all tasks finished or timeout, generate others device";
            DeviceGenerator *generator = DeviceFactory::getDeviceGenerator();
            generator->generatorOthersDevice();
            generator->generatorInfoFromToml(DT_Others);

            // 指针使用结束释放
            delete generator;
            generator = nullptr;

            break;
        }
    }
}

void GenerateDevicePool::initType()
{
    qCDebug(appLog) << "GenerateDevicePool::initType start";
    m_TypeList.push_back(DT_Bluetoorh);
    m_TypeList.push_back(DT_Keyboard);
    m_TypeList.push_back(DT_Mouse);
    m_TypeList.push_back(DT_Image);
    m_TypeList.push_back(DT_Storage);
    m_TypeList.push_back(DT_Computer);
    m_TypeList.push_back(DT_Cpu);
    m_TypeList.push_back(DT_Bios);
    m_TypeList.push_back(DT_Memory);
    m_TypeList.push_back(DT_Gpu);
    m_TypeList.push_back(DT_Monitor);
    m_TypeList.push_back(DT_Network);
    m_TypeList.push_back(DT_Audio);
    m_TypeList.push_back(DT_Print);
    m_TypeList.push_back(DT_Cdrom);
    m_TypeList.push_back(DT_Power);
//    m_TypeList.push_back(DT_Others);
    qCDebug(appLog) << "GenerateDevicePool::initType end, initialized" << m_TypeList.size() << "device types";
}

void GenerateDevicePool::slotFinished(const QStringList &lst)
{
    qCDebug(appLog) << "GenerateDevicePool::slotFinished, busIDs:" << lst << "finished:" << m_FinishedGenerator+1 << "/" << m_TypeList.size();
    DeviceManager::instance()->addBusId(lst);
    m_FinishedGenerator++;
}
