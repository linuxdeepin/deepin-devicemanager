// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADERINFOTABLEWIDGET_H
#define HEADERINFOTABLEWIDGET_H

#include <DTableWidget>
#include <DWidget>

#include <QObject>

class HeaderInfoTableWidget : public DTK_WIDGET_NAMESPACE::DTableWidget
{
    Q_OBJECT
public:
    explicit HeaderInfoTableWidget(DTK_WIDGET_NAMESPACE::DWidget *parent = nullptr);
    void updateData(const QList<QPair<QString, QString>> &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    void resetTableContents();
    class HeaderInfoDelegate *m_delegate = nullptr;
};

#endif
