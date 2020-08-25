/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#ifndef RICHTEXTDELEGATE_H
#define RICHTEXTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QDomDocument>

class RichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RichTextDelegate(QObject *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
    void getDocFromLst(QDomDocument &doc, const QStringList &lst)const;
    void addRow(QDomDocument &doc, QDomElement &table, const QPair<QString, QString> &pair)const;
    void addTd1(QDomDocument &doc, QDomElement &tr, const QString &value)const;
    void addTd2(QDomDocument &doc, QDomElement &tr, const QString &value)const;
};

#endif // RICHTEXTDELEGATE_H
