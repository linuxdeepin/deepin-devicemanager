#ifndef DETECTJOB_H
#define DETECTJOB_H

#include <QObject>

class DetectJob : public QObject
{
    Q_OBJECT
public:
    explicit DetectJob(QObject *parent = nullptr);

    void working();
    void setUSleepElapse(unsigned int elapse);

signals:
    void usbChanged();

private:
    qint64          m_LaskTime;
    unsigned int    m_Elapse;
};

#endif // DETECTJOB_H
