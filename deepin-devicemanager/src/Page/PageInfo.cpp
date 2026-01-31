// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageInfo.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DApplication>

// Qt库文件
#include <QPainter>
#include <QStyleOptionFrame>
#include <QLoggingCategory>
#include <QPainterPath>
#include <QProcess>

using namespace DDLog;
DWIDGET_USE_NAMESPACE
PageInfo::PageInfo(QWidget *parent)
    : QWidget(parent)
    , m_AllInfoNum(0)
    , m_multiFlag(false)
{
    qCDebug(appLog) << "PageInfo constructor";
}

void PageInfo::updateInfo(const QMap<QString, QString> &)
{
    qCDebug(appLog) << "PageInfo::updateInfo";
}

void PageInfo::setLabel(const QString &, const QString &)
{
    qCDebug(appLog) << "PageInfo::setLabel";
}

void PageInfo::clearContent()
{
    qCDebug(appLog) << "PageInfo::clearContent";
}

void PageInfo::setDeviceInfoNum(int num)
{
    qCDebug(appLog) << "Setting device info count to:" << num;
    // 设置设备信息数目
    m_AllInfoNum = num;
}

int PageInfo::getDeviceInfoNum()
{
    qCDebug(appLog) << "PageInfo::getDeviceInfoNum, num:" << m_AllInfoNum;
    // 获取设备信息数目
    return m_AllInfoNum;
}

void PageInfo::setMultiFlag(const bool &flag)
{
    // qCDebug(appLog) << "PageInfo::setMultiFlag, flag:" << flag;
    m_multiFlag = flag;
}

bool PageInfo::getMultiFlag()
{
    // qCDebug(appLog) << "PageInfo::getMultiFlag, flag:" << m_multiFlag;
    return m_multiFlag;
}

bool PageInfo::packageHasInstalled(const QString &packageName)
{
    qCDebug(appLog) << "Checking if package is installed:" << packageName;
    QProcess p;
    p.start("dpkg", QStringList() << "-s" << packageName);
    p.waitForFinished(-1);

    QByteArray r = p.readAll();
    bool installed = r.contains("installed");
    qCDebug(appLog) << "Package" << packageName << "is" << (installed ? "installed" : "not installed");
    return installed;
}

void PageInfo::paintEvent(QPaintEvent *e)
{
    // qCDebug(appLog) << "PageInfo::paintEvent";
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/) {
        // qCDebug(appLog) << "PageInfo::paintEvent, active window";
        cg = DPalette::Active;
    } else {
        // qCDebug(appLog) << "PageInfo::paintEvent, inactive window";
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QPainterPath paintPath;
    paintPath.addRoundedRect(rect, radius, radius);
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

