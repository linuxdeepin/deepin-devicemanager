/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QObject>
#include <QWidget>
#include <DTextBrowser>
#include <QDomDocument>
#include <DNotifySender>

#include "DeviceInfo.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DeviceBaseInfo;
/**
 * @brief The TextBrowser class
 * 封装富文本控件
 */
class TextBrowser : public DTextBrowser
{
    Q_OBJECT
public:
    explicit TextBrowser(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param info ：需要显示的设备信息
     */
    void showDeviceInfo(DeviceBaseInfo *info);

    /**
     * @brief updateInfo : 点击详细按钮之后的展开和收起操作
     */
    void updateInfo();

    /**
     * @brief setDeviceEnabled
     * @param enable
     */
    EnableDeviceStatus setDeviceEnabled(bool enable);

    /**
     * @brief updateShowOtherInfo 更新是否需要显示其它信息
     */
    void updateShowOtherInfo();


public slots:
    void fillClipboard();

protected:
    virtual void paintEvent(QPaintEvent *event)override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;


signals:
    void refreshInfo();
    void exportInfo();
    void copyAllInfo();

private slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();
    void slotActionCopy();

private:
    void domTitleInfo(QDomDocument &doc, DeviceBaseInfo *info);
    void domTableInfo(QDomDocument &doc, const QList<QPair<QString, QString>> &info);

private:
    bool                    m_ShowOtherInfo;
    DeviceBaseInfo          *mp_Info;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QAction                 *mp_Copy;        //<! 拷贝
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // TEXTBROWSER_H
