#include <QCoreApplication>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include<sys/types.h>
#include<sys/wait.h>

#include "MainJob.h"

int main(int argc, char *argv[])
{
//    int n = fork();
//    fprintf(stdout, "PID:%d, PGID:%d, SID:%d\n", getpid(), getpgrp(), getsid(0));

//    int stat = 0;
//    if (n > 0) {
//        pid_t x = wait(&stat);
//        printf("child pid=%d\n", x);
//        if (WIFEXITED(stat) == 0) {
//            printf("child normal exited!\n");
//        }
//    } else if (n == 0) {

//        if (setsid() < 0) {
//            fprintf(stderr, "setsid error:%s\n", strerror(errno));
//        }

//        QCoreApplication a(argc, argv);
//        MainJob job;
//        job.working();
//        return a.exec();
//    }

//    return 0;


    QCoreApplication a(argc, argv);
    MainJob job;
    job.working();
    return a.exec();
}
