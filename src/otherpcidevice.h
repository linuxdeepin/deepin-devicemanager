#pragma once

#include "deviceinfowidgetbase.h"

class OtherPciDeviceWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit OtherPciDeviceWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

