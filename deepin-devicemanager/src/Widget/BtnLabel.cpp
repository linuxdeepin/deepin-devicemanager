#include "BtnLabel.h"

#include <DDialog>

BtnLabel::BtnLabel(DWidget *parent)
    : DLabel(parent)
    , m_Desc("")
{
    // bug132075 安装成功状态此button无法点击
    // 仅安装失败可点击
    connect(this, &QLabel::linkActivated, this, [this]() {
        if (m_Desc.isEmpty())
            return;
        DDialog dialog;
        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.setTitle(m_Desc);
        dialog.addButton(tr("OK", "button"));
        dialog.exec();
    });
}

void BtnLabel::setDesc(const QString &txt)
{
    m_Desc = txt;
}


