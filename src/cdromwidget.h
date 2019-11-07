#pragma once

#include "deviceinfowidgetbase.h"

class CDRomWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit CDRomWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

