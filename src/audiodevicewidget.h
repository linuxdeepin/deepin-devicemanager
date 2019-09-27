#pragma once

#include "deviceinfowidgetbase.h"

class AudiodeviceWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit AudiodeviceWidget(QWidget *parent = nullptr);
    void initWidget() override;
signals:

public slots:
};

