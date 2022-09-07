// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MainJob.h"
#include "DriverManager.h"

#include <QCoreApplication>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainJob job;
    job.working();

    return a.exec();
}
