#ifndef PAGEBOARDINFO_H
#define PAGEBOARDINFO_H

#include <QObject>
#include "PageSingleInfo.h"

class DeviceBios;
class RichTextDelegate;

class PageBoardInfo : public PageSingleInfo
{
    Q_OBJECT
public:
    PageBoardInfo(QWidget *parent = nullptr);

    /**
     * @brief updateInfo
     * @param lst
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief loadDeviceInfo
     * @param lst
     */
    void loadDeviceInfo(const QList<QPair<QString, QString>> &lst, int limiteNum);
    void loadDeviceInfo(const QList<DeviceBaseInfo *> &devices, const QList<QPair<QString, QString>> &lst);

    /**
     * @brief getOtherInfoPair
     * @param lst
     * @param lstPair
     */
    void getOtherInfoPair(const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair);

    /**
     * @brief getKeyValueInfo 将设备信息直接转换为key-value直
     * @param device
     * @param pair
     */
    void getValueInfo(DeviceBaseInfo *device, QPair<QString, QString> &pair);

private:
    RichTextDelegate     *mp_ItemDelegate;
};

#endif // PAGEBOARDINFO_H
