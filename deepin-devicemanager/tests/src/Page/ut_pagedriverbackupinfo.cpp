// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "stub.h"

#define private public
#include "PageDriverBackupInfo.h"
#include "PageDriverTableView.h"
#include "driveritem.h"
#undef private

class UT_PageDriverBackupInfo : public ::testing::Test
{
public:
    void SetUp()
    {
        m_page = new PageDriverBackupInfo;
    }
    void TearDown()
    {
        delete m_page;
    }
    PageDriverBackupInfo *m_page = nullptr;
};

TEST_F(UT_PageDriverBackupInfo, Constructor_Created_ReturnsValidPointers)
{
    // Arrange
    // (page created in SetUp)

    // Act
    bool headValid = (m_page->mp_HeadWidget != nullptr);
    bool viewValid = (m_page->mp_ViewBackable != nullptr);

    // Assert
    EXPECT_NE(nullptr, m_page);
    EXPECT_TRUE(headValid);
    EXPECT_TRUE(viewValid);
    EXPECT_NE(nullptr, m_page->mp_ViewBackedUp);
    EXPECT_NE(nullptr, m_page->mp_BackableDriverLabel);
    EXPECT_NE(nullptr, m_page->mp_BackedUpDriverLabel);
}

TEST_F(UT_PageDriverBackupInfo, ShowTables_BothZero_AllHidden)
{
    // Arrange
    // Act
    m_page->showTables(0, 0);

    // Assert
    EXPECT_TRUE(m_page->mp_BackableDriverLabel->isHidden());
    EXPECT_TRUE(m_page->mp_NotBackupWidget->isHidden());
    EXPECT_TRUE(m_page->mp_BackedUpDriverLabel->isHidden());
    EXPECT_TRUE(m_page->mp_BackedUpWidget->isHidden());
}

TEST_F(UT_PageDriverBackupInfo, ShowTables_BackableOnly_BackableVisible)
{
    // Arrange
    // Act
    m_page->showTables(3, 0);

    // Assert
    EXPECT_FALSE(m_page->mp_BackableDriverLabel->isHidden());
    EXPECT_FALSE(m_page->mp_NotBackupWidget->isHidden());
    EXPECT_TRUE(m_page->mp_BackedUpDriverLabel->isHidden());
    EXPECT_TRUE(m_page->mp_BackedUpWidget->isHidden());
}

TEST_F(UT_PageDriverBackupInfo, ShowTables_BackedUpOnly_BackedUpVisible)
{
    // Arrange
    // Act
    m_page->showTables(0, 2);

    // Assert
    EXPECT_TRUE(m_page->mp_BackableDriverLabel->isHidden());
    EXPECT_TRUE(m_page->mp_NotBackupWidget->isHidden());
    EXPECT_FALSE(m_page->mp_BackedUpDriverLabel->isHidden());
    EXPECT_FALSE(m_page->mp_BackedUpWidget->isHidden());
}

TEST_F(UT_PageDriverBackupInfo, ShowTables_BothPresent_AllVisible)
{
    // Arrange
    // Act
    m_page->showTables(2, 1);

    // Assert
    EXPECT_FALSE(m_page->mp_BackableDriverLabel->isHidden());
    EXPECT_FALSE(m_page->mp_NotBackupWidget->isHidden());
    EXPECT_FALSE(m_page->mp_BackedUpDriverLabel->isHidden());
    EXPECT_FALSE(m_page->mp_BackedUpWidget->isHidden());
}

TEST_F(UT_PageDriverBackupInfo, ClearAllData_AfterAdd_RowsCleared)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());

    delete info;
}

TEST_F(UT_PageDriverBackupInfo, UpdateItemStatus_BackupFailed_NoCrash)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->updateItemStatus(0, ST_DRIVER_BACKUP_FAILED);

    // Assert
    EXPECT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);

    delete info;
}

TEST_F(UT_PageDriverBackupInfo, UpdateItemStatus_Normal_NoCrash)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->updateItemStatus(0, ST_DRIVER_BACKUP_SUCCESS);

    // Assert
    EXPECT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);

    delete info;
}

TEST_F(UT_PageDriverBackupInfo, SetCheckedCBDisnable_Called_NoCrash)
{
    // Arrange
    // Act
    m_page->setCheckedCBDisnable();

    // Assert
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);
    EXPECT_NE(nullptr, m_page);
}

TEST_F(UT_PageDriverBackupInfo, SetHeaderCbEnable_TrueFalse_NoCrash)
{
    // Arrange
    // Act
    m_page->setHeaderCbEnable(true);
    m_page->setHeaderCbEnable(false);

    // Assert
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);
    EXPECT_NE(nullptr, m_page);
}

TEST_F(UT_PageDriverBackupInfo, GetCheckedDriverIndex_Empty_ListEmpty)
{
    // Arrange
    QList<int> list;

    // Act
    m_page->getCheckedDriverIndex(list);

    // Assert
    EXPECT_EQ(0, list.size());
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(UT_PageDriverBackupInfo, GetCheckedDriverIndex_WithData_ListNonNegative)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;
    m_page->addDriverInfoToTableView(info, 0);
    QList<int> list;

    // Act
    m_page->getCheckedDriverIndex(list);

    // Assert
    EXPECT_GE(list.size(), 0);
    EXPECT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());

    delete info;
}

TEST_F(UT_PageDriverBackupInfo, AddDriverInfo_Backable_RowIncremented)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "0.9.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;
    int before = m_page->mp_ViewBackable->model()->rowCount();

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    int after = m_page->mp_ViewBackable->model()->rowCount();
    EXPECT_EQ(before + 1, after);
    EXPECT_GT(after, before);

    delete info;
}

TEST_F(UT_PageDriverBackupInfo, AddDriverInfo_BackedUp_RowIncremented)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "0.9.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_CAN_UPDATE;
    int before = m_page->mp_ViewBackedUp->model()->rowCount();

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    int after = m_page->mp_ViewBackedUp->model()->rowCount();
    EXPECT_EQ(before + 1, after);
    EXPECT_GT(after, before);

    delete info;
}
