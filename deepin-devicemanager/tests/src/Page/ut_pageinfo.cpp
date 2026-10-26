// Copyright (C) 2019-2026 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
    PageInfo *m_pageInfo = nullptr;
};

TEST_F(PageInfo_UT, PageInfo_UT_getDeviceInfoNum)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    m_pageInfo->getDeviceInfoNum();
    QMap<QString, QString> map;
    map.insert("/", "/");
    m_pageInfo->updateInfo(map);
    m_pageInfo->setLabel("", "");
    m_pageInfo->clearContent();
    EXPECT_EQ(0,m_pageInfo->m_AllInfoNum);
    delete p;
}

TEST_F(PageInfo_UT, PageInfo_UT_paintEvent)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    QPaintEvent paint(QRect(m_pageInfo->rect()));
    m_pageInfo->paintEvent(&paint);
    delete p;
}

// Parametrized test for PageInfo::packageHasInstalled
// Stub readAll to inject controlled dpkg output without running dpkg
static QByteArray g_readAllData;

static QByteArray ut_QIODevice_readAll(void *self)
{
    Q_UNUSED(self)
    return g_readAllData;
}

struct PackageHasInstalledParam {
    QByteArray dpkgOutput;
    bool expected;
};

class PackageHasInstalled_UT
    : public UT_HEAD
    , public testing::WithParamInterface<PackageHasInstalledParam>
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_P(PackageHasInstalled_UT, packageHasInstalled)
{
    g_readAllData = GetParam().dpkgOutput;

    Stub stub;
    stub.set(ADDR(QIODevice, readAll), ut_QIODevice_readAll);

    PageMultiInfo *p = new PageMultiInfo;
    PageInfo *pageInfo = dynamic_cast<PageInfo *>(p);
    bool result = pageInfo->packageHasInstalled("test-package");
    EXPECT_EQ(GetParam().expected, result);
    delete p;
}

INSTANTIATE_TEST_SUITE_P(PackageHasInstalledCases, PackageHasInstalled_UT,
    testing::Values(
        PackageHasInstalledParam{"Status: install ok installed\n", true},
        PackageHasInstalledParam{"Package: dde-printer\nStatus: install ok installed\nPriority: optional\n", true},
        PackageHasInstalledParam{"", false},
        PackageHasInstalledParam{"package 'x' is not installed\n", false}
    )
);
