#ifndef NOTIFYTHREAD_H
#define NOTIFYTHREAD_H

#include <QObject>
#include <QThread>


class NotifyThread : public QThread
{
    Q_OBJECT
public:
    explicit NotifyThread(QObject *parent = nullptr);
    void run() override;

signals:

public slots:
};

#endif // NOTIFYTHREAD_H
