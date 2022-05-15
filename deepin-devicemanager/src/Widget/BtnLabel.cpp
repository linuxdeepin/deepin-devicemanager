#include "BtnLabel.h"

#include <DDialog>

BtnLabel::BtnLabel(DWidget *parent)
    : DLabel(parent)
    , m_Desc("")
{
    connect(this, &BtnLabel::clicked, this, [this]() {
        if (m_Desc.isEmpty())
            return;
        DDialog dialog;
        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.setTitle(m_Desc);
        dialog.addButton(tr("OK","button"));
        dialog.exec();
    });
}

void BtnLabel::setDesc(const QString &txt)
{
    m_Desc = txt;
}

void BtnLabel::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    return DLabel::mousePressEvent(event);
}

