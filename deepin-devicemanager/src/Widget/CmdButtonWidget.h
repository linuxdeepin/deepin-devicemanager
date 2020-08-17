#ifndef CMDBUTTONWIDGET_H
#define CMDBUTTONWIDGET_H

#include <QObject>

#include <DWidget>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class DetailTreeView;

class CmdButtonWidget : public DWidget
{
    Q_OBJECT
public:
    CmdButtonWidget(DetailTreeView *parent = nullptr);

    void setButtonText(const QString &str);

signals:
    void cmdButtonClicked();

public slots:
    /**
     * @brief expandCommandLinkClicked:展开button点击槽函数
     */
    void expandCommandLinkClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();

private:
    DCommandLinkButton *mp_cmdButton;

};

#endif // CMDBUTTONWIDGET_H
