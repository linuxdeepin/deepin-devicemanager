#ifndef DEVICEDETAILPAGE_H
#define DEVICEDETAILPAGE_H

#include <QObject>
#include <DWidget>
#include <QScrollArea>
#include <QHBoxLayout>
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
protected:
    void paintEvent(QPaintEvent *e) override;
};

class DetailSeperator: public DWidget
{
    Q_OBJECT
public:
    explicit DetailSeperator(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
};

class ScrollAreaWidget: public DWidget
{
    Q_OBJECT
public:
    explicit ScrollAreaWidget(DWidget *parent = nullptr);

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

    /**
     * @brief showInfoOfNum : 点击表头时，详细信息显示到相应的位置
     * @param index : 需要显示到相应的位置
     */
    void showInfoOfNum(int index);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief addWidgets 添加widget到布局
     * @param widget ：需要添加的widget
     */
    void addWidgets(TextBrowser *widget);

    /**
     * @brief clearWidget : 清楚widget
     */
    void clearWidget();

private slots:
    void slotBtnClicked();

private:
    QVBoxLayout      *mp_ScrollAreaLayout;
    QScrollArea      *mp_ScrollArea;

    QList<TextBrowser *>           m_ListTextBrowser;
    QList<QHBoxLayout *>           m_ListHlayout;
    QList<DetailButton *>          m_ListDetailButton;
    QList<DetailSeperator *>       m_ListDetailSeperator;
};

#endif // DEVICEDETAILPAGE_H
