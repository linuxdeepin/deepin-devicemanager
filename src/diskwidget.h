#pragma once

#include "deviceinfowidgetbase.h"

class DiskWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit DiskWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

