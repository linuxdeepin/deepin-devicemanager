#pragma once

#include <QWidget>
#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"
#include "DLabel"
#include "deviceattributedefine.h"

struct TableHeader
{
    QString head;
    int length;
};

struct ArticleStruct
{
    QString name;
    QString value;

    bool autoHide = true;
};


class QLabel;
class QVBoxLayout;
class TableWidgetAlwaysFocus;

struct DeviceInfo
{
    Dtk::Widget::DLabel* title = nullptr;
    QList<Dtk::Widget::DLabel*>  nameLabels;
    QList<Dtk::Widget::DLabel*>  contentLabels;
};

class DeviceInfoWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoWidgetBase(QWidget *parent = nullptr, const QString& deviceName = "");
    virtual ~DeviceInfoWidgetBase();

    virtual void initWidget() = 0;

    void initContextMenu();

    // version1.0

    void setTitle(const QString& title);
    void addInfo(const QStringList& names, const QStringList& contents);
    void addInfo(const QList<ArticleStruct> articles);

    void addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents);
    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);
    void createTable();

    void addStrecch();
    void initDownWidget();
    QString getDeviceName();

    virtual void deviceListClicked(){}

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    bool onExportToFile();

private:
    bool writeToTextStream(QTextStream& ts);
    virtual bool exportToTxt(const QString& txtFile);
    virtual bool exportToDoc(const QString& docFile);
    virtual bool exportToXls(const QString& xlsFile);
    virtual bool exportToHtml(const QString& htmlFile);

protected:
    QString deviceName_ = "DeviceInfoWidgetBase";

    TableWidgetAlwaysFocus* tableWidget_ = nullptr;
    DeviceInfo* titleInfo_ = nullptr;
    QList<DeviceInfo> deviceInfos_;

    QVBoxLayout* vLayout_ = nullptr;
    Dtk::Widget::DScrollArea* downWidgetScrollArea_ = nullptr;
    Dtk::Widget::DWidget*   downWidget_  = nullptr;
    QVBoxLayout* downWidgetLayout = nullptr;
    Dtk::Widget::DMenu* contextMenu_ = nullptr;

    static bool isTitleFontInit_;
    static QFont titleFont_;
    static bool isSubTitleFontInit_;
    static QFont subTitleFont_;
};
