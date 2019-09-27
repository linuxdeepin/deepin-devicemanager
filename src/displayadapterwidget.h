#pragma once

#include "deviceinfowidgetbase.h"

class DisplayadapterWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit DisplayadapterWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

