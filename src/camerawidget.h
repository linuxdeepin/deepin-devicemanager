#pragma once

#include "deviceinfowidgetbase.h"

class CameraWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit CameraWidget(QWidget *parent = nullptr);
    void initWidget() override;
};

