// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "WaitingWidget.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Qt库文件
#include <QHBoxLayout>

using namespace DDLog;


#define SPINNER_WIDTH  32  // 小圈圈的宽度
#define SPINNER_HEIGHT 32  // 小圈圈的高度


WaitingWidget::WaitingWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Spinner(new DSpinner(this))
    , mp_Label(new DLabel(tr("Loading..."), this))
{
    qCDebug(appLog) << "WaitingWidget constructor start";
    // 设置小圈圈的参数,小圈圈的大小
    mp_Spinner->setFixedSize(SPINNER_WIDTH, SPINNER_HEIGHT);

    // 初始化布局
    // 新建Layout,不需要指定父窗口
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStretch();

    QHBoxLayout *spinnerLayout = new QHBoxLayout();
    spinnerLayout->addStretch();
    spinnerLayout->addWidget(mp_Spinner);
    spinnerLayout->addStretch();
    vLayout->addLayout(spinnerLayout);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addStretch();
    labelLayout->addWidget(mp_Label);
    labelLayout->addStretch();
    vLayout->addLayout(labelLayout);

    vLayout->addStretch();

    hLayout->addLayout(vLayout);
    hLayout->addStretch();
    setLayout(hLayout);
    qCDebug(appLog) << "WaitingWidget constructor end";
}

WaitingWidget::~WaitingWidget()
{
    qCDebug(appLog) << "WaitingWidget destructor start";
    DELETE_PTR(mp_Spinner);
    DELETE_PTR(mp_Label);
}

void WaitingWidget::start()
{
    qCDebug(appLog) << "Starting waiting spinner";
    // 开始转动
    if (mp_Spinner)
        mp_Spinner->start();
}
void WaitingWidget::stop()
{
    qCDebug(appLog) << "Stopping waiting spinner";
    // 停止转动
    if (mp_Spinner)
        mp_Spinner->stop();
}
