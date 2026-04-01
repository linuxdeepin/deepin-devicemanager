// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADERINFOTABLEDELEGATE_H
#define HEADERINFOTABLEDELEGATE_H

#include <QStyledItemDelegate>

class HeaderInfoDelegate : public QStyledItemDelegate
{
public:
    explicit HeaderInfoDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif
