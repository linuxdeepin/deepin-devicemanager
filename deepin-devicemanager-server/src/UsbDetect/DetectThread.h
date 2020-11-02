#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>

class DetectJob;
//class MainJob;

class DetectThread : public QThread
{
    Q_OBJECT
public:
    DetectThread(QObject *parent);

    /**
     * @brief run
     */
    void run() override;

    /**
     * @brief setUSleepElapse
     * @param elapse
     */
    void setUSleepElapse(unsigned int elapse);

signals:
    void usbChanged();

private slots:
    void slotUsbChanged();

private:
    DetectJob    *mp_DectectJob;
};

#endif // DETECTTHREAD_H
