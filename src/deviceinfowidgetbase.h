#pragma once

#include <QWidget>
#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"
#include "DLabel"
#include "deviceattributedefine.h"

#include "document.h"
#include "xlsxdocument.h"

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

class DeviceInfoWidgetBase : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoWidgetBase(Dtk::Widget::DWidget *parent = nullptr, const QString& deviceName = "");

    virtual bool getOverViewInfo(ArticleStruct& info);

    virtual void initWidget() = 0;

    void initContextMenu();

    // version1.0
    void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QStringList& names, const QStringList& contents, const QFont& font);
    void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles, const QFont& font);

    //void setTitle(const QString& title);
    void addInfo(const QString& title, const QStringList& names, const QStringList& contents);
    void addInfo(const QString& title, const QList<ArticleStruct>& articles);
    //void addInfo(const QString& name, const QString& value);

    //void addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents);
    void addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles);

    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);

    void addStrecch();
    void initDownWidget();

    QString getDeviceName();

    virtual void deviceListClicked();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    void OnCurrentItemClicked(QTableWidgetItem *item);
    bool onExportToFile();

public:
    virtual bool exportToTxt(QFile& txtFile);
    virtual bool exportToDoc(Docx::Document& doc);
    virtual bool exportToXls(QXlsx::Document& xlsFile);
    virtual bool exportToHtml(QFile& htmlFile);

    virtual bool exportToTxt(const QString& txtFile);
    virtual bool exportToDoc(const QString& docFile);
    static void resetXlsRowCount();
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

    static bool isFontInit_;
    static QFont titleFont_;
    static QFont subTitleFont_;
    static QFont infoFont_;
    static QFont labelFont_;
    static QFont tableContentFont_;

    int currentRow_ = -1;   //-1 for whole info

    ArticleStruct overviewInfo_;

    int verticalScrollBarMaxValue = 0;

    //static int currentXlsRow_;
};
