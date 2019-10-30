#include "devicelistview.h"
#include <QStandardItemModel>
#include <QMargins>
#include <QVariant>
#include <QPainter>
#include <qdrawutil.h>
#include "DApplicationHelper"
#include "DMenu"
#include "DApplication"
#include "mainwindow.h"
#include <QDir>
#include "DFileDialog"
#include <QDateTime>
#include <QContextMenuEvent>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return QSize(option.rect.width(), 5);;
    }
    else
    {
        return DStyledItemDelegate::sizeHint(option, index);
    }
}

void DeviceListviewDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 8, yPoint, option.rect.width() - 8, yPoint, option.palette);

    painter->restore();
}

DeviceListView::DeviceListView(DWidget* parent):DListView(parent)
{
//    setResizeMode(QListView::Adjust);
//    setOrientation(QListView::LeftToRight, true);
//    setTextElideMode(Qt::ElideMiddle);
//    setSelectionBehavior(QAbstractItemView::SelectRows);

    //DListView::setSelectionRectVisible(false);

    navModel_ = new QStandardItemModel(navModel_);

    setVerticalScrollMode(ScrollPerPixel);
    //setIconSize(QSize(24, 24));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    setMouseTracking(true);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);

    setModel(navModel_);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setFrameShape(QFrame::Shape::NoFrame);

    setEditTriggers(QListView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    //setAutoScroll(false);

    //setSpacing(0);
    //setViewMode(QListView::ListMode);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    viewport()->setAutoFillBackground(true);

    setFocus(Qt::FocusReason::NoFocusReason);




    //DPalette pa = DApplicationHelper::instance()->palette(this);
    //auto ba = QPalette::Base;
    //ba.setBrush(DPalette::ItemBackground, palette().base());
    //setBackgroundRole(QPalette::Base);
    //setAutoFillBackground(true);

    setItemDelegate( new DeviceListviewDelegate(this) );

    setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);


    auto modifyTheme = [this](){
//        DPalette pa;
//        pa.setColor(DPalette::Background,QColor(0, 248, 248));
//        setAutoFillBackground(true);
//        setPalette(pa);

        DPalette pa = DApplicationHelper::instance()->palette(this);
        QColor base_color = palette().base().color();

        pa.setBrush(DPalette::ItemBackground, base_color);
        pa.setColor(QPalette::Background, base_color);

        setPalette(pa);

        DApplicationHelper::instance()->setPalette(this, pa);
    };

    modifyTheme();

    this->setAutoFillBackground(true);
    //this->setBackgroundRole(DPalette::Base);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, modifyTheme);
    //setMaximumWidth(150);

    initContextMenu();

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnlvOpRightBtn(const QPoint&)));

    setItemSpacing(0);
}

void DeviceListView::addDevice(const QString& deviceName, const QString& iconFile)
{
    DStandardItem* item = new DStandardItem;
    QFont itemFont = item->font();
    itemFont.setPixelSize(16);
    itemFont.setWeight(QFont::Medium);

    item->setFont(itemFont);

    item->setSizeHint( QSize(20, 40) );

    DGuiApplicationHelper::ColorType ct = DApplicationHelper::instance()->themeType();

    QString icon = ":images/";
    icon += ((ct == DGuiApplicationHelper::LightType) ? "light/normal/":"dark/normal/");
    icon += iconFile;

    item->setIcon(QIcon(icon));
    item->setData(icon, Qt::UserRole+89);

    item->setText(deviceName);

    item->setTextAlignment(Qt::AlignLeft);

    const QMargins ListViweItemMargin(20,8,20,8);
    //const QMargins ListViweItemMargin( 10, 5, 10, 5 );
    const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

    item->setData(VListViewItemMargin, Dtk::MarginsRole);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

    navModel_->appendRow(item);
}

void DeviceListView::addSeperator()
{
    DStandardItem* item = new DStandardItem;
    //item->setBackgroundRole(QPalette::Base);
    //item->setAutoFillBackground(true);
    item->setText("Seperator");
    navModel_->appendRow(item);
}

QString DeviceListView::currentDevice()
{
    return currentDevice_;
}

void DeviceListView::setCurrentDevice(QString& device)
{
    currentDevice_ = device;
}

void DeviceListView::setSelectNull()
{
    QModelIndex index = navModel_->index(0, 0);
    selectionModel()->select(index, QItemSelectionModel::Select);
}

QString DeviceListView::indexAt(int index)
{
    return navModel_->index(index, 0).data().toString();
}

bool DeviceListView::onExportToFile()
{
   QString selectFilter;

   QString saveDir = "./";
   QDir dir( QDir::homePath() + "/Documents/");
   if(dir.exists())
   {
        saveDir = QDir::homePath() + "/Documents/";
   }

   QString exportFile = DFileDialog::getSaveFileName(this,
                                                     tr("Export File"), saveDir + DApplication::translate("Main", "deviceInfo") + \
                                                     QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") .remove(QRegExp("\\s")) + ".txt", \
                                                    tr("Text (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);

   if(exportFile.isEmpty() == true)
   {
       return true;
   }

   MainWindow* mainWindow = dynamic_cast<MainWindow*>(this->parent()->parent());
   if( nullptr == mainWindow )
   {
       return false;
   }

   return mainWindow->exportTo(exportFile, selectFilter);
}

void DeviceListView::OnlvOpRightBtn( const QPoint& point)
{
    contextMenu_->exec(mapToGlobal(point));
}

void DeviceListView::initContextMenu()
{
    contextMenu_ = new DMenu(this);
    QAction* refreshAction = new QAction(DApplication::translate("Main", "Refresh"), this);
    connect(refreshAction, &QAction::triggered, \
            [this]()
            {
                MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent());
                if(mainWindow)
                {
                    mainWindow->refresh();
                }
            }
    );
    contextMenu_->addAction(refreshAction);


    QAction* exportAction = new QAction(DApplication::translate("Main", "Export"));

    connect(exportAction, &QAction::triggered, this, &DeviceListView::onExportToFile);

    contextMenu_->addAction(exportAction);
}

void DeviceListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QStandardItem* currentItem = navModel_->itemFromIndex(current);
    if(currentItem)
    {
        QString icon = currentItem->data(Qt::UserRole+89).toString();
        if(icon.isEmpty() == false)
        {
            icon.replace("normal", "press");
            currentItem->setIcon(QIcon(icon));
            currentItem->setData(icon, Qt::UserRole+89);
        }
    }

    QStandardItem* previousItem = navModel_->itemFromIndex(previous);
    if(previousItem)
    {
        QString icon = previousItem->data(Qt::UserRole+89).toString();
        if(icon.isEmpty() == false)
        {
            icon.replace("press", "normal");
            previousItem->setIcon(QIcon(icon));
            previousItem->setData(icon, Qt::UserRole+89);
        }
    }


    DListView::currentChanged(current, previous);
}

//void DeviceListView::contextMenuEvent(QContextMenuEvent *event)
//{
//    contextMenu_->exec(event->globalPos());
//}


