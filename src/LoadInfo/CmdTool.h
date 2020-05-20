#ifndef CMDTOOL_H
#define CMDTOOL_H

#include <QObject>
#include <QMap>
enum SplitType {
    ST_Null,
    ST_lshw,
    ST_dmidecode,
    ST_Hwinfo,
};

class CmdTool
{
public:
    CmdTool();
    void loadCmdInfo(QMap<QString, QMap<QString, QString> > &cmdInfo, const QString &cmd, const QString &key,  SplitType st, const QString &debugFile = QString(""));

private:
    void getMapInfo(SplitType st, const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));
    bool executeProcess(const QString &cmd, QString &deviceInfo);
    bool runCmd(const QString &cmd, QString &deviceInfo);
};

#endif // CMDTOOL_H
