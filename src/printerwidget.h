#pragma once

#include "deviceinfowidgetbase.h"

class PrinterWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit PrinterWidget(QWidget *parent = nullptr);

    void initWidget() override;
};

