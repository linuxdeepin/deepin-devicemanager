#pragma once

#include <QWidget>
#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"
#include "DLabel"
#include "deviceattributedefine.h"

class QLabel;
class QVBoxLayout;
class TableWidgetAlwaysFocus;
class QGridLayout;

struct TableHeader
{
    QString head;
    int length;
};

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

    virtual bool getOverViewInfo(ArticleStruct& info);

    virtual void initWidget() = 0;

    void initContextMenu();

    // version1.0
    void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QStringList& names, const QStringList& contents);
    void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles);

    void setTitle(const QString& title);
    void addInfo(const QStringList& names, const QStringList& contents);
    void addInfo(const QList<ArticleStruct>& articles);
    void addInfo(const QString& name, const QString& value);

    void addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents);
    void addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles);

    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);

    void addStrecch();
    void initDownWidget();

    QString getDeviceName();

    virtual void deviceListClicked();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    void OnCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    bool onExportToFile();

private:
    virtual bool exportToTxt(const QString& txtFile);
    virtual bool exportToDoc(const QString& docFile);
    virtual bool exportToXls(const QString& xlsFile);
    virtual bool exportToHtml(const QString& htmlFile);

protected:
    Dtk::Widget::DTableWidget* tableWidget_ = nullptr;
    DeviceInfo* titleInfo_ = nullptr;
    QList<DeviceInfo> deviceInfos_;

    QVBoxLayout* vLayout_ = nullptr;
    Dtk::Widget::DScrollArea* downWidgetScrollArea_ = nullptr;
    Dtk::Widget::DWidget*   downWidget_  = nullptr;
    QVBoxLayout* downWidgetLayout = nullptr;
    Dtk::Widget::DMenu* contextMenu_ = nullptr;

    Dtk::Widget::DWidget* infoWidget_ = nullptr;
    QList<Dtk::Widget::DWidget*> subinfoWidgetList_;

    static bool isTitleFontInit_;
    static QFont titleFont_;
    static bool isSubTitleFontInit_;
    static QFont subTitleFont_;

    int currentRow_ = -1;   //-1 for whole info

    ArticleStruct overviewInfo_;
};
