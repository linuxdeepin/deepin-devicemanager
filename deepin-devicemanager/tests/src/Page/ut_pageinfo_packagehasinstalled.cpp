// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 覆盖率缺口补全：PageInfo::packageHasInstalled 此前未被用例调用。
// 该函数通过 `dpkg -s <pkg>` 判断包是否安装：已安装包 stdout 含 "installed"，
// 不存在的包 stdout 为空（提示信息走 stderr）。PageInfo 为抽象基类，通过其派生类
// PageMultiInfo 实例化后调用。

#include "PageInfo.h"
#include "PageMultiInfo.h"

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

TEST(UT_PageInfo_PackageHasInstalled, InstalledDpkgPackage_ReturnsTrue)
{
    PageMultiInfo *p = new PageMultiInfo;
    PageInfo *info = dynamic_cast<PageInfo *>(p);
    // dpkg 自身必然已安装
    EXPECT_TRUE(info->packageHasInstalled("dpkg"));
    delete p;
}

TEST(UT_PageInfo_PackageHasInstalled, NonExistentPackage_ReturnsFalse)
{
    PageMultiInfo *p = new PageMultiInfo;
    PageInfo *info = dynamic_cast<PageInfo *>(p);
    EXPECT_FALSE(info->packageHasInstalled("zzz-not-a-real-package-20260811"));
    delete p;
}
