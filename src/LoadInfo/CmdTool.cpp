#include "CmdTool.h"

CmdTool::CmdTool()
{

}

void CmdTool::loadCmdInfo(const QString &key, SplitType st, QMap<QString, QMap<QString, QString> > &cmdInfo)
{

}

void CmdTool::getMapInfo(SplitType st, const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    switch (st) {
    case ST_Null:
        getMapInfoFromCmd(info, mapInfo);
        break;
    case ST_lshw:
        getMapInfoFromLshw(info, mapInfo);
        break;
    case ST_Hwinfo:
        getMapInfoFromHwinfo(info, mapInfo);
        break;
    case ST_dmidecode:
        getMapInfoFromDmidecode(info, mapInfo);
        break;
    }
}
void CmdTool::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{

}
void CmdTool::getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{

}
void CmdTool::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{

}
void CmdTool::getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{

}
