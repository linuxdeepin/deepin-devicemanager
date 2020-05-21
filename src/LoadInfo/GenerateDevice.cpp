#include "GenerateDevice.h"
#include "CmdTool.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceComputer.h"

GenerateDevice::GenerateDevice()
{

}

void GenerateDevice::generatorComputerDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();

}
