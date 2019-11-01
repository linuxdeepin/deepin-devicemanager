#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>

int main(int argc, char *argv[])
{
    if ( argc < 2)
    {
        qDebug() << "less than 1";
        return 0;
    }

    QProcess proc;

    proc.start( argv[1] );
    proc.waitForFinished(-1);

    std::cout << proc.readAllStandardOutput().data();

    proc.close();

    return 0;
}
