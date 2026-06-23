// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceGenerator 未测的纯逻辑方法 pciPath/uniqueID + 桩 cmdInfo 的 getMouse/othersInfoFromLshw。

#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QString>
#include <QList>

#include <gtest/gtest.h>

static QList<QMap<QString, QString> > ut_dge_lst;
static const QList<QMap<QString, QString> > &ut_dge_cmdInfo()
{
    return ut_dge_lst;
}

class UT_DeviceGeneratorExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        dg = new DeviceGenerator;
    }
    void TearDown() override
    {
        delete dg;
    }
    DeviceGenerator *dg = nullptr;
};

// pciPath: 5 分支
TEST_F(UT_DeviceGeneratorExtra, pciPath)
{
    QMap<QString, QString> m1;
    m1["path"] = "/devices/pci0000:00";
    EXPECT_EQ(dg->pciPath(m1), QString("/devices/pci0000:00"));

    QMap<QString, QString> m2;
    m2["SysFS Device Link"] = "/sys/link";
    EXPECT_EQ(dg->pciPath(m2), QString("/sys/link"));

    QMap<QString, QString> m3;
    m3["SysFS ID"] = "/class/net/eth0";
    EXPECT_EQ(dg->pciPath(m3), QString("/class/net/eth0"));

    QMap<QString, QString> m4;
    m4["Device Files"] = "/dev/platform-foo";
    EXPECT_EQ(dg->pciPath(m4), QString("platform"));

    QMap<QString, QString> m5;
    EXPECT_EQ(dg->pciPath(m5), QString(""));
}

// uniqueID: 5 分支
TEST_F(UT_DeviceGeneratorExtra, uniqueID)
{
    QMap<QString, QString> m1;
    m1["unique_id"] = "uid123";
    EXPECT_EQ(dg->uniqueID(m1), QString("uid123"));

    QMap<QString, QString> m2;
    m2["Unique ID"] = "UID456";
    EXPECT_EQ(dg->uniqueID(m2), QString("UID456"));

    QMap<QString, QString> m3;
    m3["Permanent HW Address"] = "aa:bb:cc:dd";
    EXPECT_EQ(dg->uniqueID(m3), QString("aa:bb:cc:dd"));

    QMap<QString, QString> m4;
    m4["Serial ID"] = "SER789";
    EXPECT_EQ(dg->uniqueID(m4), QString("SER789"));

    QMap<QString, QString> m5;
    EXPECT_EQ(dg->uniqueID(m5), QString(""));
}

// getMouseInfoFromLshw: 桩 cmdInfo(空)
TEST_F(UT_DeviceGeneratorExtra, getMouseInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_dge_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(dg->getMouseInfoFromLshw());
}

// getOthersInfoFromLshw: 桩 cmdInfo(空)
TEST_F(UT_DeviceGeneratorExtra, getOthersInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_dge_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(dg->getOthersInfoFromLshw());
}

// generatorInfoFromToml: 桩 tomlDeviceSet
static void ut_dge_tomlDeviceSet(DeviceManager * /*self*/, DeviceType /*t*/)
{
    return;
}
TEST_F(UT_DeviceGeneratorExtra, generatorInfoFromToml)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, tomlDeviceSet), ut_dge_tomlDeviceSet);
    EXPECT_NO_FATAL_FAILURE(dg->generatorInfoFromToml(DT_Others));
}
