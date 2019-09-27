#pragma once

#include "deviceinfowidgetbase.h"

class ComputerOverviewWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit ComputerOverviewWidget(QWidget *parent = nullptr);
    void initWidget() override;
};
