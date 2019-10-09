#include "logpasswordauth.h"
#include <DApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

LogPasswordAuth::LogPasswordAuth(DWidget *parent)
    : DDialog(parent)
{
    this->setFixedSize(300, 100);

    initUI();
    initConnections();
}

void LogPasswordAuth::initUI()
{
    //    this->setTitle("授权验证");
    m_edt = new DLineEdit(this);
    m_edt->setEchoMode(QLineEdit::Password);
    DLabel *label = new DLabel("请输入sudo密码执行该操作！");
    this->addContent(label);
    this->addContent(m_edt);
    this->addButton(DApplication::translate("Button", "Cancel"));
    this->addButton(DApplication::translate("Button", "Comfirm"), true);
}

void LogPasswordAuth::initConnections()
{
    connect(this, &DDialog::buttonClicked, this, &LogPasswordAuth::slotButtonClicked);
}

QString LogPasswordAuth::getPasswd()
{
    return m_passwd;
}

void LogPasswordAuth::slotButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
        case 0:
            this->setPasswd("");
            this->setOnButtonClickedClose(true);
            break;
        case 1:
            if (m_edt->text().isEmpty())
                this->setOnButtonClickedClose(false);
            else {
                this->setOnButtonClickedClose(true);
                this->setPasswd(m_edt->text());
            }
            break;
        default:
            break;
    }
    m_edt->clear();
}

void LogPasswordAuth::setPasswd(QString str)
{
    m_passwd = str;
}
