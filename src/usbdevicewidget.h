#pragma once

#include "deviceinfowidgetbase.h"

class UsbdeviceWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit UsbdeviceWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

