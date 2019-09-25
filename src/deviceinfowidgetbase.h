#pragma once

#include <QWidget>
#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"

#include "deviceattributedefine.h"

class QLabel;
class QVBoxLayout;

struct DeviceInfo
{
    QLabel* title = nullptr;
    QList<QLabel*>  nameLabels;
    QList<QLabel*>  contentLabels;
};

//enum class ExportFileType{ Txt, Doc, Xls, Html, Invalid};

class DeviceInfoWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoWidgetBase(QWidget *parent = nullptr, const QString& deviceName = "");
    virtual ~DeviceInfoWidgetBase();

    void initContextMenu();

    void setTitle(const QString& title);
    void addInfo(const QStringList& names, const QStringList& contents);
    void addLinefeed();
    void addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents);
    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);
    void addStrecch();

    void initDownWidget();

    QString getDeviceName();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    bool exportToFile();

//    virtual bool exportToTxt() = 0;
//    virtual bool exportToDoc() = 0;
//    virtual bool exportToXls() = 0;
//    virtual bool exportToHtml() = 0;

private:
    QString deviceName_ = "DeviceInfoWidgetBase";

    DeviceInfo* titleInfo_ = nullptr;
    QList<DeviceInfo> deviceInfos_;

    QVBoxLayout* vLayout_ = nullptr;
    Dtk::Widget::DTableWidget* tableWidget_ = nullptr;
    Dtk::Widget::DWidget*   downWidget_  = nullptr;
    QVBoxLayout* downWidgetLayout = nullptr;
    Dtk::Widget::DMenu* contextMenu_ = nullptr;
};
