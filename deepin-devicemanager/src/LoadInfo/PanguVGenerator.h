#ifndef PANGUVGENERATOR_H
#define PANGUVGENERATOR_H

#include <QObject>
#include "KLUGenerator.h"

class PanguVGenerator : public KLUGenerator
{
public:
    PanguVGenerator();

    /**
     * @brief generatorMonitorDevice:生成显示设备信息
     */
    virtual void generatorMonitorDevice();
};

#endif // PANGUVGENERATOR_H
