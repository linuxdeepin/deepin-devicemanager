#pragma once

#include "deviceinfowidgetbase.h"

class CpuWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit CpuWidget(QWidget *parent = nullptr);

    void initWidget() override;
};
