#pragma once

#include "deviceinfowidgetbase.h"

class MouseWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit MouseWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

