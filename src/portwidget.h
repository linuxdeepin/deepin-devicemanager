#pragma once

#include "deviceinfowidgetbase.h"

class PortWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit PortWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

