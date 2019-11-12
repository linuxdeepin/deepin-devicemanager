/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QList>
#include "DTableWidget"
#include "DScrollArea"
#include "DMenu"
#include "DLabel"
#include "deviceattributedefine.h"
#include "xlsxdocument.h"
#include "document.h"
#include "DPalette"

class QLabel;
class QVBoxLayout;
class TableWidgetAlwaysFocus;
class QGridLayout;
class QAction;
class ColumnWidget;

struct TableHeader
{
    QString head;
    int length;
};

struct DeviceInfo
{
    Dtk::Widget::DLabel* title = nullptr;
    QList<ColumnWidget*>  columnWidgets;

    DeviceInfo();
    void changeTheme();

//    static bool isPaletteInit_;
//    static Dtk::Gui::DPalette defaultPa_;
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

    //void addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles, const QFont& font , const QPalette& pa);
    void addCloumnToLayout(DeviceInfo* di, QVBoxLayout* vly, const QList<ArticleStruct>& articles, const QFont& font , int columnHeight, const QPalette& pa);

    void setCentralInfo(const QString& info);

    //void addInfo(const QString& title, const QList<ArticleStruct>& articles);
    // Html version
    void addHtmlInfo(const QString& title, const QList<ArticleStruct>& articles);
    // cloumnwidgeth version
    void addInfo(const QString& title, const QList<ArticleStruct>& articles, bool main = true);

    //void addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles);
    // cloumnwidget version
    void addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles);

    void addTable(const QStringList& headers, const QList<QStringList>& contentsList);

    void addDevice( const QString& subTitle, const QList<ArticleStruct>& articles, int deviceNumber, bool showTitle = false );

    void initDownWidget();

    QString getDeviceName();

    virtual void deviceListClicked();

    static int maxDeviceSize(const QStringList& list1, const QStringList& list2, const QStringList& list3);

    void selectColumnWidget(ColumnWidget* sw);

    void getContextMenu(Dtk::Widget::DMenu** contextMenu);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void OnCurrentItemClicked(/*QTableWidgetItem *item*/);
    void onScroll(int value);
    bool onExportToFile();
    void changeTheme();

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

public:
    static bool isFontInit_;
    static QFont titleFont_;
    static QFont subTitleFont_;
    static QFont infoFont_;
    static QFont labelFont_;
    static QFont tableHeaderFont_;
    static QFont tableContentFont_;

protected:
    ArticleStruct overviewInfo_;

    int verticalScrollBarMaxValue = 0;

    //static int currentXlsRow_;
    ColumnWidget* selectColumnWidget_ = nullptr;

    static bool isPaletteInit_;
    static Dtk::Gui::DPalette defaultPa_;
};
