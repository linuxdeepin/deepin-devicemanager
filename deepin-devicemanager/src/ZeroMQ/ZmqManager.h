#ifndef ZMQMANAGER_H
#define ZMQMANAGER_H

#include <QObject>
class ZMQBase;

class ZmqManager : public QObject
{
    Q_OBJECT
public:
    static ZmqManager* getInstance(){
        if(!s_Instance)
            s_Instance = new ZmqManager;
        return s_Instance;
    }

    /**
     * @brief connectTo
     * @param end
     * @return
     */
    bool connectTo(char *end);

    /**
     * @brief updateData
     * @return
     */
    int updateData();

protected:
    explicit ZmqManager(QObject *parent = nullptr);

private:
    static ZmqManager*        s_Instance;
    ZMQBase                   *mp_Req;
};

#endif // ZMQMANAGER_H
