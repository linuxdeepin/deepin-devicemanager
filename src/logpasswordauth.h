#ifndef LOGPASSWORDAUTH_H
#define LOGPASSWORDAUTH_H

#include <dabstractdialog.h>
#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DWidget>
#include <QTimer>

class LogPasswordAuth : public Dtk::Widget::DDialog
{
public:
    LogPasswordAuth(Dtk::Widget::DWidget *parent = nullptr);
    void initUI();
    void initConnections();

    QString getPasswd();
    void clearPasswd() {m_passwd = "";}

public slots:
    void slotOkBtnClicked();
    void slotButtonClicked(int index, const QString &text);

private:
    void setPasswd(QString str);
    QString m_passwd;
    Dtk::Widget::DLineEdit *m_edt;
    Dtk::Widget::DPushButton *m_okBtn, *m_cancleBtn;
    QTimer *m_timer;
};

#endif  // LOGPASSWORDAUTH_H
