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
    void loadCmdInfo(const QString &key, SplitType st, QMap<QString, QMap<QString, QString> > &cmdInfo);

private:
    void getMapInfo(SplitType st, const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
    void getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
};

#endif // CMDTOOL_H
