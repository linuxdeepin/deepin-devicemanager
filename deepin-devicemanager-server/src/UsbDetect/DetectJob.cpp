#include "DetectJob.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <QDateTime>
#include <QDebug>

DetectJob::DetectJob(QObject *parent) : QObject(parent)
    , m_LaskTime(QDateTime::currentMSecsSinceEpoch())
    , m_Elapse(10000)
{

}

void DetectJob::working()
{
    int sockfd;
    struct sockaddr_nl sa;
    char buf[4096];
    struct iovec iov;
    struct msghdr msg;
    //    int i;
    memset(buf, 0, sizeof(buf));
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;//getpid(); both is ok
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void *)buf;
    iov.iov_len = sizeof(buf);
    msg.msg_name = (void *)&sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sockfd = socket(AF_NETLINK/*PF_NETLINK*/, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sockfd == -1)
        printf("socket creating failed:%s\n", strerror(errno));
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
        printf("bind error:%s\n", strerror(errno));

    while (true) {
        int len = int(recvmsg(sockfd, &msg, 0)); //MSG_WAITALL
        int midLen = -1;
        QString Buf(buf);
        if (Buf.startsWith("add@/devices/pci")) {
            midLen = 36;
        } else if (Buf.startsWith("remove@/devices/pci")) {
            midLen = 39;
        }

        if (midLen != -1) {
            qint64 curTime = QDateTime::currentMSecsSinceEpoch();
            if (curTime - m_LaskTime > (m_Elapse / 1000 + 2000)) {
//                qInfo() << Buf.mid(0, midLen);
                emit usbChanged(/*Buf.mid(0, midLen)*/);
                m_LaskTime = curTime;
            }
        }

        memset(buf, 0, sizeof(buf));
        usleep(m_Elapse);
    }
}

void DetectJob::setUSleepElapse(unsigned int elapse)
{
    m_Elapse = elapse;
}
