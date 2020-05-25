#ifndef MIPSGENERATOR_H
#define MIPSGENERATOR_H
#include "DeviceGenerator.h"

class MipsGenerator : public DeviceGenerator
{
public:
    MipsGenerator();

protected:
    void getOthersInfoFromHwinfo();
};

#endif // MIPSGENERATOR_H
