// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverNameWidget.h"
#include "GetDriverNameModel.h"
#include "WaitingWidget.h"

#include <DFrame>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DStackedWidget>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QThread>

#include "MacroDefinition.h"
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
    if (mp_GetModel) {
        delete mp_GetModel;
        mp_GetModel = nullptr;
    }
    DELETE_PTR(mp_titleLabel);
}

void GetDriverNameWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mp_titleLabel = new DLabel(tr("Select a driver for update"));
    mp_StackWidget->addWidget(mp_WaitingWidget);
    mp_StackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_StackWidget->addWidget(mp_ListView);
    mp_StackWidget->setContentsMargins(0, 0, 0, 0);
    mp_tipLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    mp_tipLabel->setElideMode(Qt::ElideRight);
    mp_tipLabel->setMinimumHeight(25);

    DFontSizeManager::instance()->bind(mp_tipLabel, DFontSizeManager::T8, QFont::Medium);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DPalette pa = DGuiApplicationHelper::instance()->palette(mp_tipLabel);
    pa.setColor(DPalette::WindowText, pa.color(DPalette::TextWarning));
    DGuiApplicationHelper::instance()->setPalette(mp_tipLabel, pa);
#else
    DPalette pa = mp_tipLabel->palette();
    pa.setColor(DPalette::WindowText, pa.color(DPalette::TextWarning));
    mp_tipLabel->setPalette(pa);
#endif

    // 上方布局
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(mp_titleLabel);
    hLayout1->addStretch();
    mainLayout->addLayout(hLayout1);

    // 中间布局
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->setContentsMargins(5, 5, 5, 5);
    hLayout2->addWidget(mp_StackWidget);
    DFrame *frame = new DFrame;
    frame->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hLayout2);
    mainLayout->addWidget(frame);

    // 下面布局
    QHBoxLayout *hLayout3 = new QHBoxLayout;
    hLayout3->addStretch();
    hLayout3->addWidget(mp_tipLabel);
    hLayout3->addStretch();
    mainLayout->addLayout(hLayout3);
    this->setLayout(mainLayout);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GetDriverNameWidget::onUpdateTheme);
}

void GetDriverNameWidget::onUpdateTheme()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
#else
    QPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(QPalette::Window, plt.color(QPalette::Base));
#endif
    mp_titleLabel->setPalette(plt);
}

void GetDriverNameWidget::initConnections()
{
    connect(mp_ListView, &DriverListView::clicked, this, &GetDriverNameWidget::slotSelectedDriver);
    connect(this, &GetDriverNameWidget::startLoadDrivers, mp_GetModel, &GetDriverNameModel::startLoadDrivers);
    connect(mp_GetModel, &GetDriverNameModel::finishLoadDrivers, this, &GetDriverNameWidget::slotFinishLoadDrivers);
}

void GetDriverNameWidget::loadAllDrivers(bool includeSub, const QString &path)
{
    mp_WaitingWidget->start();
    mp_StackWidget->setCurrentIndex(0);
    mp_model = new QStandardItemModel(this);
    emit startLoadDrivers(mp_model, includeSub, path);
}

void GetDriverNameWidget::reloadDriversListPages()
{
    DFrame *frame = this->findChild<DFrame *>();
    DLabel *label = this->findChild<DLabel *>();
    if (!(frame && label))
        return;
    if (mp_model->rowCount() <= 0) {
        frame->hide();
        mp_tipLabel->hide();
        label->setText(tr("No drivers found in this folder"));
        mp_selectedRow = -1;
        emit  signalDriversCount();
    } else {
        frame->show();
        mp_tipLabel->show();
        label->setText(tr("Select a driver for update"));
    }

    //TODO
    DPalette pa = label->palette();
    QColor color = pa.color(QPalette::Active, QPalette::BrightText).darker(130);
    pa.setColor(QPalette::WindowText, color);
    label->setPalette(pa);
}

QString GetDriverNameWidget::selectName()
{
    if (-1 == mp_selectedRow || !(mp_selectedRow < mp_model->rowCount() && mp_selectedRow > -1))
        return "";
    return mp_model->item(mp_selectedRow)->data(Qt::UserRole).toString();
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
    if (mp_model->rowCount() > 0) {
        QModelIndex index = mp_model->index(0, 0);
        mp_ListView->setCurrentIndex(index);
    }

    mp_StackWidget->setCurrentIndex(1);
    mp_WaitingWidget->stop();
}
