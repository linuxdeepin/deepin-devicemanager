#ifndef DEVICEDETAILPAGE_H
#define DEVICEDETAILPAGE_H

#include <QObject>
#include <DWidget>
#include <QScrollArea>
#include <DCommandLinkButton>

class TextBrowser;
class DeviceBaseInfo;
class QVBoxLayout;

using namespace Dtk::Widget;

class DetailButton: public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit DetailButton(const QString &txt);
    void initWidgets();
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    DCommandLinkButton   *mp_Button;
};

class DetailSeperator: public DWidget
{
    Q_OBJECT
public:
    explicit DetailSeperator(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
};


class PageDetail : public DWidget
{
    Q_OBJECT
public:
    explicit PageDetail(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param lstInfo ：需要显示的设备
     */
    void showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief addWidgets 添加widget到布局
     * @param widget ：需要添加的widget
     */
    void addWidgets(QWidget *widget);

private:
    QVBoxLayout      *mp_ScrollAreaLayout;
    QScrollArea      *mp_ScrollArea;
};

#endif // DEVICEDETAILPAGE_H
