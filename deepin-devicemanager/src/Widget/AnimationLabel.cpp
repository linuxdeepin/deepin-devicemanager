// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AnimationLabel.h"
#include "DDLog.h"

using namespace DDLog;

AnimationLabel::AnimationLabel(QWidget *parent)
    : DLabel(parent)
    , m_CurrentIndex(-1)
{
    qCDebug(appLog) << "AnimationLabel constructor called";
}

void AnimationLabel::setImageList(const QStringList &iconList)
{
    qCDebug(appLog) << "Setting image list with" << iconList.size() << "icons";
    // 设置图片列表
    m_ImageList << iconList;
}

void AnimationLabel::setcurrentIcon(int iconIndex)
{
    qCDebug(appLog) << "Setting current icon to index:" << iconIndex;
    // 设置当前Icon
    if (iconIndex < 0 || iconIndex >= m_ImageList.size()) {
        qCWarning(appLog) << "Icon index" << iconIndex << "is out of bounds.";
        return;
    }
    QIcon icon(QIcon::fromTheme(m_ImageList[iconIndex]));
    QPixmap pix = icon.pixmap(128, 128);
    this->setPixmap(pix);
    m_CurrentIndex = iconIndex;
}

int AnimationLabel::currentIcon()
{
    // qCDebug(appLog) << "Getting current icon index:" << m_CurrentIndex;
    return m_CurrentIndex;
}


