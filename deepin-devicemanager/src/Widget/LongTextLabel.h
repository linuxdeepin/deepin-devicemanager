#ifndef LONGTEXTLABEL_H
#define LONGTEXTLABEL_H

#include <QObject>

#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class LongTextLabel : public DLabel
{
    Q_OBJECT

public:
    LongTextLabel(DWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;

private:
    int m_width;

};

#endif // LONGTEXTLABEL_H
