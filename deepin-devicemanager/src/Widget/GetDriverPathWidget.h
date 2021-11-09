/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef GETDRIVERLSTDIALOG_H
#define GETDRIVERLSTDIALOG_H

#include <DCheckBox>
#include <DLabel>
#include <DPushButton>
#include <DWidget>

#include <QObject>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

class UrlChooserEdit;
class GetDriverPathWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GetDriverPathWidget(QWidget *parent = nullptr);
    ~GetDriverPathWidget();

public:
    /**
     * @brief path 返回用户选择的驱动文件所在目录
     * @return 返回该目录
     */
    QString path();

    /**
     * @brief includeSubdir 返回是否包含子目录中的驱动文件
     * @return
     */
    bool includeSubdir();

    /**
     * @brief updateTipLabelText 更新tipLabel
     * @param text
     */
    void updateTipLabelText(const QString &text);

    /**
     * @brief clearTipLabelText
     */
    void clearTipLabelText();

private:
    /**
     * @brief init 初始化界面
     */
    void init();

signals:
    /**
     * @brief signalNotLocalFolder
     */
    void signalNotLocalFolder(bool isLocal);

private:
    UrlChooserEdit *mp_fileChooseEdit;
    DCheckBox      *mp_includeCheckBox;
    DLabel         *mp_tipLabel;
};
DWIDGET_END_NAMESPACE
#endif // GETDRIVERLSTDIALOG_H
