#pragma once

#include "deviceinfowidgetbase.h"

class PowerWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit PowerWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

