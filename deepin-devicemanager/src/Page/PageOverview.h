#ifndef PAGEOVERVIEW_H
#define PAGEOVERVIEW_H

#include <QObject>
#include <DWidget>

#include "PageInfo.h"

DWIDGET_USE_NAMESPACE

class PageOverview : public PageInfo
{
public:
    PageOverview(DWidget *parent = nullptr);
};

#endif // PAGEOVERVIEW_H
