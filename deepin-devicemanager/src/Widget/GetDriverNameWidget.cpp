#include "GetDriverNameWidget.h"
#include "GetDriverNameModel.h"
#include "WaitingWidget.h"

#include <DFrame>
#include <DLabel>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DStackedWidget>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QThread>

GetDriverNameWidget::GetDriverNameWidget(QWidget *parent)
    : DWidget(parent)
    , mp_StackWidget(new DStackedWidget(this))
    , mp_WaitingWidget(new WaitingWidget(mp_StackWidget))
    , mp_ListView(new DriverListView(mp_StackWidget))
    , mp_tipLabel(new DLabel)
    , mp_GetModel(new GetDriverNameModel())
    , mp_Thread(new QThread(this))
{
    mp_GetModel->moveToThread(mp_Thread);
    mp_Thread->start();
    init();
    initConnections();
}

GetDriverNameWidget::~GetDriverNameWidget()
{
    stopLoadingDrivers();
    mp_Thread->quit();
    mp_Thread->wait();
}

void GetDriverNameWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    DFrame *frame = new DFrame;
    frame->setFixedSize(460, 165);
    frame->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    DLabel *titleLable = new DLabel(tr("Select a driver for update"));
    hLayout->addStretch();
    hLayout->addWidget(titleLable);
    hLayout->addStretch();

    QHBoxLayout *driverLayout = new QHBoxLayout;
    driverLayout->setContentsMargins(0, 0, 0, 0);
    driverLayout->addStretch();
    driverLayout->addWidget(frame);
    driverLayout->addStretch();

    mp_StackWidget->addWidget(mp_WaitingWidget);
    mp_StackWidget->addWidget(mp_ListView);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(mp_StackWidget);
    layout->addStretch();
    frame->setLayout(layout);

    mainLayout->addStretch();
    mainLayout->addLayout(hLayout);
    mainLayout->addLayout(driverLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(mp_tipLabel);

    mp_tipLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    mp_tipLabel->setElideMode(Qt::ElideRight);
    mp_tipLabel->setMinimumHeight(20);

    DFontSizeManager::instance()->bind(mp_tipLabel, DFontSizeManager::T8, QFont::Medium);
    DPalette pa = DApplicationHelper::instance()->palette(mp_tipLabel);
    pa.setColor(DPalette::WindowText, pa.color(DPalette::TextWarning));
    DApplicationHelper::instance()->setPalette(mp_tipLabel, pa);

    this->setLayout(mainLayout);
}

void GetDriverNameWidget::initConnections()
{
    connect(mp_ListView, &DriverListView::clicked, this, &GetDriverNameWidget::slotSelectedDriver);
    connect(this,&GetDriverNameWidget::startLoadDrivers,mp_GetModel,&GetDriverNameModel::startLoadDrivers);
    connect(mp_GetModel, &GetDriverNameModel::finishLoadDrivers,this,&GetDriverNameWidget::slotFinishLoadDrivers);
}

void GetDriverNameWidget::loadAllDrivers(bool includeSub, const QString &path)
{
    mp_WaitingWidget->start();
    mp_StackWidget->setCurrentIndex(0);
    mp_model = new QStandardItemModel(this);
    emit startLoadDrivers(mp_model,includeSub,path);
}

void GetDriverNameWidget::reloadDriversListPages()
{
    DFrame *frame = this->findChild<DFrame *>();
    DLabel *label = this->findChild<DLabel *>();
    if (!(frame && label))
        return;
    if (mp_model->rowCount() <= 0) {
        frame->hide();
        label->setText(tr("No drivers found in this folder"));
        mp_selectedRow = -1;
        emit  signalDriversCount();
    } else {
        frame->show();
        label->setText(tr("Select a driver for update"));
    }

    DPalette pa = DApplicationHelper::instance()->palette(label);
    QColor color = DGuiApplicationHelper::adjustColor(pa.color(QPalette::Active, QPalette::BrightText), 0, 0, 0, 0, 0, 0, -30);
    pa.setColor(QPalette::WindowText, color);
    label->setPalette(pa);
}

QString GetDriverNameWidget::selectName()
{
    if (-1 == mp_selectedRow || !(mp_selectedRow < mp_model->rowCount() && mp_selectedRow > -1))
        return "";
    return mp_model->item(mp_selectedRow,0)->text();
}

void GetDriverNameWidget::updateTipLabelText(const QString &text)
{
    mp_tipLabel->setText(text);
    mp_tipLabel->setToolTip(text);
}

void GetDriverNameWidget::stopLoadingDrivers()
{
    mp_GetModel->stopLoadingDrivers();
}

void GetDriverNameWidget::slotSelectedDriver(const QModelIndex &index)
{
    updateTipLabelText("");
    emit signalItemClicked();
    int row = index.row();
    QStandardItem *item =  mp_model->item(row, 1);
    if (!item)
        return;
    QStandardItem *lastItem = mp_model->item(mp_selectedRow, 1);
    if (lastItem) {
        lastItem->setCheckState(Qt::Unchecked);
        lastItem->setData(QVariant(), Qt::CheckStateRole);
    }
    if (Qt::Unchecked == item->checkState())
        item->setCheckState(Qt::Checked);
    mp_selectedRow = row;
}

void GetDriverNameWidget::slotFinishLoadDrivers()
{
    reloadDriversListPages();
    mp_ListView->setModel(mp_model);
    mp_ListView->setColumnWidth(0, 40);
    updateTipLabelText("");

    mp_StackWidget->setCurrentIndex(1);
    mp_WaitingWidget->stop();
}
