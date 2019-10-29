#ifndef LOGPASSWORDAUTH_H
#define LOGPASSWORDAUTH_H

#include <dabstractdialog.h>
#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DWidget>
#include <QTimer>
#include "DIconButton"
#include "DPasswordEdit"

class LogPasswordAuth : public Dtk::Widget::DAbstractDialog
{
public:
    LogPasswordAuth(Dtk::Widget::DWidget *parent = nullptr);
    ~LogPasswordAuth();
    void initUI();
    void initConnections();

    QString getPasswd();
    void clearPasswd();

    void hideMessage();
    void showMessage(const QString& msg);

public slots:
    void cancelButtonClicked();
    void confimButtonClicked();
    void defaultButtonTriggered();
    void onTimer();

private:
    //void setPasswd(QString str);
    //QString m_passwd;

    QHBoxLayout *iconLayout;
    QVBoxLayout *contentLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *topLayout;

    QLabel* iconLabel;
    QLabel* messageLabel;
    QLabel* titleLabel;

    Dtk::Widget::DPushButton* cancelButton_;
    Dtk::Widget::DPushButton* confimButton_;

    Dtk::Widget::DPasswordEdit *m_edt;

    QTimer* m_timer = nullptr;
};

#endif  // LOGPASSWORDAUTH_H
