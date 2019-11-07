#pragma once

#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"
#include "DLabel"
#include "deviceattributedefine.h"
#include "xlsxdocument.h"
#include "document.h"


class QLabel;
class QVBoxLayout;
class TableWidgetAlwaysFocus;
class QGridLayout;
class QAction;

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

class ColumnWidget: public Dtk::Widget::DWidget
{
private:
    Dtk::Widget::DLabel* l1;
    Dtk::Widget::DLabel* l2;

public:
    ColumnWidget(Dtk::Widget::DWidget* parent = nullptr);
    void mousePressEvent(QMouseEvent *event) override;
};

class DeviceInfoWidgetBase : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoWidgetBase(Dtk::Widget::DWidget *parent = nullptr, const QString& deviceName = "");
    void initFont();

    virtual bool getOverViewInfo(ArticleStruct& info);

    virtual void initWidget() = 0;

    void initContextMenu();

    void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles, const QFont& font , const QPalette& pa);

    void setCentralInfo(const QString& info);

    void addInfo(const QString& title, const QList<ArticleStruct>& articles);
    // Html version
    void addHtmlInfo(const QString& title, const QList<ArticleStruct>& articles);

    void addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles);

    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);

    void addDevice( const QString& subTitle, const QList<ArticleStruct>& articles, int deviceNumber, bool showTitle = false );

    void initDownWidget();

    QString getDeviceName();

    virtual void deviceListClicked();

    static int maxDeviceSize(const QStringList& list1, const QStringList& list2, const QStringList& list3);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

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

    QAction* refreshAction_ = nullptr;

    Dtk::Widget::DWidget* infoWidget_ = nullptr;
    QList<Dtk::Widget::DWidget*> subinfoWidgetList_;

    static bool isFontInit_;
    static QFont titleFont_;
    static QFont subTitleFont_;
    static QFont infoFont_;
    static QFont labelFont_;
    static QFont tableHeaderFont_;
    static QFont tableContentFont_;

    ArticleStruct overviewInfo_;

    int verticalScrollBarMaxValue = 0;

    //static int currentXlsRow_;
};
