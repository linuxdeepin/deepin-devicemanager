#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <QObject>
#include <DWidget>
#include <DSpinner>
#include <DLabel>

using namespace Dtk::Widget;
class WaitingWidget : public DWidget
{
public:
    WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

    /**@brief:开始转动小圈圈*/
    void start();

    /**@brief:停止转动小圈圈*/
    void stop();

    /**
     * @brief:设置Label文本
     * @param[in] txt : 需要设置成的文本
     */
    void setLabelTxt(const QString &txt);

private:
    DSpinner        *mp_Spinner;       // 小圈圈
    DLabel          *mp_Label;         // 小圈圈下面的label
};

#endif // WAITINGWIDGET_H
