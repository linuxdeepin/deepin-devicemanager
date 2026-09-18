// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <DStackedWidget>

#include "stub.h"

#define private public
#include "PageDriverRestoreInfo.h"
#include "PageDriverTableView.h"
#include "driveritem.h"
#undef private

class UT_PageDriverRestoreInfo : public ::testing::Test
{
public:
    void SetUp()
    {
        m_page = new PageDriverRestoreInfo;
    }
    void TearDown()
    {
        delete m_page;
    }
    PageDriverRestoreInfo *m_page = nullptr;
};

TEST_F(UT_PageDriverRestoreInfo, Constructor_Created_ReturnsValidPointers)
{
    // Arrange
    // (page created in SetUp)

    // Act
    bool stackValid = (m_page->mp_StackWidget != nullptr);
    bool viewValid = (m_page->mp_ViewBackable != nullptr);

    // Assert
    EXPECT_NE(nullptr, m_page);
    EXPECT_TRUE(stackValid);
    EXPECT_TRUE(viewValid);
    EXPECT_NE(nullptr, m_page->mp_HeadWidget);
    EXPECT_NE(nullptr, m_page->mp_BackableDriverLabel);
    EXPECT_NE(nullptr, m_page->mp_NoRestoreDriverFrame);
    EXPECT_NE(nullptr, m_page->mp_GotoBackupSgButton);
}

TEST_F(UT_PageDriverRestoreInfo, ShowTables_ZeroDrivers_StackIndexZero)
{
    // Arrange
    // Act
    m_page->showTables(0);

    // Assert
    EXPECT_EQ(0, m_page->mp_StackWidget->currentIndex());
    EXPECT_NE(nullptr, m_page->mp_StackWidget);
}

TEST_F(UT_PageDriverRestoreInfo, ShowTables_WithDrivers_StackIndexOne)
{
    // Arrange
    // Act
    m_page->showTables(3);

    // Assert
    EXPECT_EQ(1, m_page->mp_StackWidget->currentIndex());
    EXPECT_NE(nullptr, m_page->mp_StackWidget);
}

TEST_F(UT_PageDriverRestoreInfo, ClearAllData_AfterAdd_RowsCleared)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "0.9.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_DRIVER_CAN_RESTORE;
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);

    delete info;
}

TEST_F(UT_PageDriverRestoreInfo, SetItemOperationEnable_TrueFalse_NoCrash)
{
    // Arrange
    // Act
    m_page->setItemOperationEnable(0, true);
    m_page->setItemOperationEnable(0, false);

    // Assert
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);
    EXPECT_NE(nullptr, m_page);
}

TEST_F(UT_PageDriverRestoreInfo, SlotOperatorClicked_RestoreMode_NoCrash)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "0.9.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_DRIVER_CAN_RESTORE;
    m_page->addDriverInfoToTableView(info, 0);
    int rowsBefore = m_page->mp_ViewBackable->model()->rowCount();

    // Act
    m_page->slotOperatorClicked(0, 0, DriverOperationItem::RESTORE);

    // Assert
    int rowsAfter = m_page->mp_ViewBackable->model()->rowCount();
    EXPECT_EQ(rowsBefore, rowsAfter);
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);

    delete info;
}

TEST_F(UT_PageDriverRestoreInfo, AddDriverInfo_WithBackup_RowIncremented)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "0.9.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_DRIVER_CAN_RESTORE;
    int before = m_page->mp_ViewBackable->model()->rowCount();

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    int after = m_page->mp_ViewBackable->model()->rowCount();
    EXPECT_EQ(before + 1, after);
    EXPECT_GT(after, before);

    delete info;
}

TEST_F(UT_PageDriverRestoreInfo, AddDriverInfo_NoBackup_RowUnchanged)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_DebBackupVersion = "";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_CAN_UPDATE;
    int before = m_page->mp_ViewBackable->model()->rowCount();

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    int after = m_page->mp_ViewBackable->model()->rowCount();
    EXPECT_EQ(before, after);
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);

    delete info;
}
