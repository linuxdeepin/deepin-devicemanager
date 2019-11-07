#pragma once

#include "deviceinfowidgetbase.h"

class MemoryWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit MemoryWidget(QWidget *parent = nullptr);

    void initWidget() override;

    void initTableWdiget();
    void updateWholeDownWidget();

private:
    int canUpgrade_ = -1;
};
