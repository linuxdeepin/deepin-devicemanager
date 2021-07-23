#ifndef RRSERVER_H
#define RRSERVER_H

#include <QThread>

class MainJob;

class RRServer : public QThread
{
    Q_OBJECT

public:
    RRServer(QObject *parent);
    ~RRServer();

    bool initTo(char *_end);
    void connectToDealer(char *_endpoint);
    void connectToDealer(char *_endpoint, void *_context);
    virtual void run();
    void sendMsg(const QString &info);

private:
    class ZMQBase            *mpRep;                 //<!
    void                     *mpContext;
    MainJob                  *mp_MainJob;
};
#endif // RRSERVER_H
