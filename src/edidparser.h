#ifndef EDIDPARSER_H
#define EDIDPARSER_H

#include <QString>
#include "monitorwidget.h"
class EDIDParser
{
public:
    EDIDParser();
    void setSource(QString source);

    bool isVaild();
    QString getReadableHexString();
    bool getDate(int &week, int &year);
    QString getManufatureName();
    QString getEDIDVersion();
    QString getMaxImageSize();
    QString getSignalType();
    QStringList getSupportResolutionList();
private:

    bool m_sourceValid = false;
    QByteArray m_edidBin;
};

#endif // EDIDPARSER_H
