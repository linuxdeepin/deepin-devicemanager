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
    void updateDownWidget(const QString& currentChannel);

    void deviceListClicked() override;

public slots:
    void OnCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

private:
    bool canUpgrade_ = false;
    int currentRow_ = -1;//-1 for whole memory info
};
