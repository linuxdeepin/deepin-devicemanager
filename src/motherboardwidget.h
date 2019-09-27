#pragma once

#include "deviceinfowidgetbase.h"
#include "deviceattributedefine.h"

class MotherboardWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit MotherboardWidget(QWidget *parent = nullptr);

    void initWidget() override;
};
