#ifndef LOADDEVICEINFO_H
#define LOADDEVICEINFO_H

#include <QObject>

class LoadDeviceInfo : public QObject
{
    Q_OBJECT
public:
    explicit LoadDeviceInfo(QObject *parent = nullptr);

signals:

public slots:
};

#endif // LOADDEVICEINFO_H