/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UrlChooserEdit.h"

#include <DApplicationHelper>
#include <DStyle>
#include <DLineEdit>
#include <DSuggestButton>

#include <QHBoxLayout>
#include <QFileDialog>


UrlChooserEdit::UrlChooserEdit(QWidget *parent) : DWidget(parent)
  ,mp_urlEdit(new DLineEdit)
  ,mp_urlBtn(new DSuggestButton)
{
   initUI();
   initConnections();
}

void UrlChooserEdit::initUI()
{
   setFixedSize(460,36);
   mp_urlEdit->setFixedSize(410,36);
   mp_urlBtn->setFixedSize(40,36);
   mp_urlBtn->setIcon(DStyleHelper(mp_urlEdit->style()).standardIcon(DStyle::SP_SelectElement, nullptr));
   mp_urlBtn->setIconSize(QSize(24,24));
   QHBoxLayout *mainLayout = new QHBoxLayout;
   setContentsMargins(0,0,0,0);
   mainLayout->setContentsMargins(0,0,0,0);
   mainLayout->addWidget(mp_urlEdit);
   mainLayout->addSpacing(10);
   mainLayout->addWidget(mp_urlBtn);
   setLayout(mainLayout);
}

void UrlChooserEdit::initConnections()
{
    connect(mp_urlBtn,&DSuggestButton::clicked, this, &UrlChooserEdit::slotChooseUrl);
}

QString UrlChooserEdit::text() const
{
    return  mp_urlEdit->text();
}

void UrlChooserEdit::slotChooseUrl()
{
    mp_urlEdit->clear();
    QString path = QFileDialog::getExistingDirectory(this,"","",QFileDialog::ReadOnly);
    mp_urlEdit->setText(path);
}
