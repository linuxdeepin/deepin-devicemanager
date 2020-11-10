#ifndef PANGUVGENERATOR_H
#define PANGUVGENERATOR_H

#include <QObject>
#include "KLUGenerator.h"

class PanguVGenerator : public KLUGenerator
{
public:
    PanguVGenerator();

    /**@brief:generator monitor info*/
    virtual void generatorMonitorDevice();

    /**@brief:generator disk info from smartctl*/
    virtual void getDiskInfoFromSmartCtl();

    /**@brief:generator disk info from lshw*/
    virtual void getDiskInfoFromLshw();
};

#endif // PANGUVGENERATOR_H
