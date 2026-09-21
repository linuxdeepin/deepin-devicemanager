// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "stub.h"

#define private public
#include "PageDriverInstallInfo.h"
#include "PageDriverTableView.h"
#include "driveritem.h"
#undef private

class UT_PageDriverInstallInfo : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_page = new PageDriverInstallInfo;
    }

    void TearDown() override
    {
        delete m_page;
    }

    PageDriverInstallInfo *m_page = nullptr;
};

// Constructor: internal pointers initialized
TEST_F(UT_PageDriverInstallInfo, Constructor_AfterInit_AllPointersInitialized)
{
    // Arrange
    int initialRowCount = m_page->mp_ViewNotInstall->model()->rowCount();

    // Act
    auto *head = m_page->mp_HeadWidget;
    PageDriverTableView *notInstall = m_page->mp_ViewNotInstall;
    PageDriverTableView *canUpdate = m_page->mp_ViewCanUpdate;

    // Assert
    EXPECT_NE(nullptr, head);
    EXPECT_NE(nullptr, notInstall);
    EXPECT_NE(nullptr, canUpdate);
    EXPECT_NE(nullptr, m_page->mp_AllDriverIsNew);
    EXPECT_NE(nullptr, m_page->mp_InstallLabel);
    EXPECT_NE(nullptr, m_page->mp_UpdateLabel);
    EXPECT_NE(nullptr, m_page->mp_LabelIsNew);
    EXPECT_NE(nullptr, m_page->mp_InstallWidget);
    EXPECT_NE(nullptr, m_page->mp_UpdateWidget);
    EXPECT_EQ(0, initialRowCount);
}

// headWidget accessor returns valid pointer
TEST_F(UT_PageDriverInstallInfo, HeadWidget_AfterInit_ReturnsHeadWidgetPointer)
{
    // Arrange
    auto *expected = m_page->mp_HeadWidget;

    // Act
    auto *result = m_page->headWidget();

    // Assert
    EXPECT_EQ(expected, result);
    EXPECT_NE(nullptr, result);
}

// showTables: all zeros → all hidden
TEST_F(UT_PageDriverInstallInfo, ShowTables_AllZeroCount_AllWidgetsHidden)
{
    // Arrange
    auto *installWidget = m_page->mp_InstallWidget;
    auto *updateWidget = m_page->mp_UpdateWidget;

    // Act
    m_page->showTables(0, 0, 0);

    // Assert
    EXPECT_TRUE(m_page->mp_LabelIsNew->isHidden());
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isHidden());
    EXPECT_TRUE(m_page->mp_InstallWidget->isHidden());
    EXPECT_TRUE(m_page->mp_UpdateWidget->isHidden());
    EXPECT_EQ(installWidget, m_page->mp_InstallWidget);
    EXPECT_EQ(updateWidget, m_page->mp_UpdateWidget);
}

// showTables: install only → install visible, update/new hidden
TEST_F(UT_PageDriverInstallInfo, ShowTables_InstallOnly_InstallVisibleOthersHidden)
{
    // Arrange
    auto *installWidget = m_page->mp_InstallWidget;

    // Act
    m_page->showTables(3, 0, 0);

    // Assert
    EXPECT_FALSE(m_page->mp_InstallWidget->isHidden());
    EXPECT_TRUE(m_page->mp_UpdateWidget->isHidden());
    EXPECT_TRUE(m_page->mp_LabelIsNew->isHidden());
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isHidden());
    EXPECT_EQ(installWidget, m_page->mp_InstallWidget);
}

// showTables: update only → update visible, install/new hidden
TEST_F(UT_PageDriverInstallInfo, ShowTables_UpdateOnly_UpdateVisibleOthersHidden)
{
    // Arrange
    auto *updateWidget = m_page->mp_UpdateWidget;

    // Act
    m_page->showTables(0, 2, 0);

    // Assert
    EXPECT_TRUE(m_page->mp_InstallWidget->isHidden());
    EXPECT_FALSE(m_page->mp_UpdateWidget->isHidden());
    EXPECT_TRUE(m_page->mp_LabelIsNew->isHidden());
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isHidden());
    EXPECT_EQ(updateWidget, m_page->mp_UpdateWidget);
}

// showTables: new only → new visible, install/update hidden
TEST_F(UT_PageDriverInstallInfo, ShowTables_NewOnly_NewVisibleOthersHidden)
{
    // Arrange
    auto *labelIsNew = m_page->mp_LabelIsNew;

    // Act
    m_page->showTables(0, 0, 1);

    // Assert
    EXPECT_TRUE(m_page->mp_InstallWidget->isHidden());
    EXPECT_TRUE(m_page->mp_UpdateWidget->isHidden());
    EXPECT_FALSE(m_page->mp_LabelIsNew->isHidden());
    EXPECT_FALSE(m_page->mp_AllDriverIsNew->isHidden());
    EXPECT_EQ(labelIsNew, m_page->mp_LabelIsNew);
}

// showTables: all present → all visible
TEST_F(UT_PageDriverInstallInfo, ShowTables_AllPresent_AllWidgetsVisible)
{
    // Arrange
    auto *installWidget = m_page->mp_InstallWidget;
    auto *updateWidget = m_page->mp_UpdateWidget;

    // Act
    m_page->showTables(2, 1, 1);

    // Assert
    EXPECT_FALSE(m_page->mp_InstallWidget->isHidden());
    EXPECT_FALSE(m_page->mp_UpdateWidget->isHidden());
    EXPECT_FALSE(m_page->mp_LabelIsNew->isHidden());
    EXPECT_FALSE(m_page->mp_AllDriverIsNew->isHidden());
    EXPECT_EQ(installWidget, m_page->mp_InstallWidget);
    EXPECT_EQ(updateWidget, m_page->mp_UpdateWidget);
}

// addDriverInfoToTableView: ST_NOT_INSTALL → row added to not-install view
TEST_F(UT_PageDriverInstallInfo, AddDriverInfo_NotInstall_RowAddedToNotInstallView)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_NOT_INSTALL;

    // Act
    int before = m_page->mp_ViewNotInstall->model()->rowCount();
    m_page->addDriverInfoToTableView(info, 0);
    int after = m_page->mp_ViewNotInstall->model()->rowCount();

    // Assert
    EXPECT_EQ(before + 1, after);
    EXPECT_EQ(1, after);

    delete info;
}

// addDriverInfoToTableView: ST_CAN_UPDATE → row added to can-update view
TEST_F(UT_PageDriverInstallInfo, AddDriverInfo_CanUpdate_RowAddedToCanUpdateView)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Camera;
    info->m_Packages = "test-pkg";
    info->m_Status = ST_CAN_UPDATE;

    // Act
    int before = m_page->mp_ViewCanUpdate->model()->rowCount();
    m_page->addDriverInfoToTableView(info, 0);
    int after = m_page->mp_ViewCanUpdate->model()->rowCount();

    // Assert
    EXPECT_EQ(before + 1, after);
    EXPECT_EQ(1, after);

    delete info;
}

// addCurDriverInfo: row added to all-driver-is-new view
TEST_F(UT_PageDriverInstallInfo, AddCurDriverInfo_ValidInfo_RowAddedToNewView)
{
    // Arrange
    DriverInfo *info = new DriverInfo();
    info->m_Name = "TestDriver";
    info->m_Version = "1.0.0";
    info->m_Type = DR_Camera;

    // Act
    int before = m_page->mp_AllDriverIsNew->model()->rowCount();
    m_page->addCurDriverInfo(info);
    int after = m_page->mp_AllDriverIsNew->model()->rowCount();

    // Assert
    EXPECT_EQ(before + 1, after);
    EXPECT_EQ(1, after);

    delete info;
}

// clearAllData: after adding data, clear resets to zero
TEST_F(UT_PageDriverInstallInfo, ClearAllData_AfterAdd_AllViewsResetToZero)
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
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());

    delete info;
}

// getCheckedDriverIndex: empty table → empty list
TEST_F(UT_PageDriverInstallInfo, GetCheckedDriverIndex_EmptyTable_ReturnsEmptyList)
{
    // Arrange
    QList<int> list;

    // Act
    m_page->getCheckedDriverIndex(list);

    // Assert
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(0, list.size());
}

// updateItemStatus: no crash after adding data
TEST_F(UT_PageDriverInstallInfo, UpdateItemStatus_AfterAdd_NoCrashAndStatusUpdated)
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
    m_page->updateItemStatus(0, ST_FAILED, "error msg");

    // Assert
    EXPECT_NO_FATAL_FAILURE({
        m_page->updateItemStatus(0, ST_FAILED, "error msg");
    });
    int rows = m_page->mp_ViewNotInstall->model()->rowCount();
    EXPECT_GT(rows, 0);
    EXPECT_EQ(1, rows);

    delete info;
}

// setCheckedCBDisnable: no crash
TEST_F(UT_PageDriverInstallInfo, SetCheckedCBDisnable_WhenCalled_NoCrashAndWidgetsValid)
{
    // Arrange
    PageDriverTableView *notInstall = m_page->mp_ViewNotInstall;
    PageDriverTableView *canUpdate = m_page->mp_ViewCanUpdate;

    // Act
    m_page->setCheckedCBDisnable();

    // Assert
    EXPECT_NO_FATAL_FAILURE({
        m_page->setCheckedCBDisnable();
    });
    EXPECT_EQ(notInstall, m_page->mp_ViewNotInstall);
    EXPECT_EQ(canUpdate, m_page->mp_ViewCanUpdate);
}

// setHeaderCbEnable: true and false, no crash
TEST_F(UT_PageDriverInstallInfo, SetHeaderCbEnable_TrueAndFalse_NoCrashAndStateValid)
{
    // Arrange
    PageDriverTableView *notInstall = m_page->mp_ViewNotInstall;

    // Act
    m_page->setHeaderCbEnable(true);
    m_page->setHeaderCbEnable(false);

    // Assert
    EXPECT_NO_FATAL_FAILURE({
        m_page->setHeaderCbEnable(true);
        m_page->setHeaderCbEnable(false);
    });
    EXPECT_EQ(notInstall, m_page->mp_ViewNotInstall);
    EXPECT_NE(nullptr, m_page->mp_ViewCanUpdate);
}

// slotDownloadProgressChanged: no crash
TEST_F(UT_PageDriverInstallInfo, SlotDownloadProgressChanged_ValidInput_NoCrashAndPageValid)
{
    // Arrange
    QStringList msg;
    msg << "50" << "100MB" << "Downloading";
    auto *head = m_page->mp_HeadWidget;

    // Act
    m_page->slotDownloadProgressChanged(DR_Camera, "100MB", msg);

    // Assert
    EXPECT_NO_FATAL_FAILURE({
        m_page->slotDownloadProgressChanged(DR_Camera, "100MB", msg);
    });
    EXPECT_EQ(head, m_page->mp_HeadWidget);
    EXPECT_NE(nullptr, m_page->mp_HeadWidget);
}

// slotDownloadFinished: no crash
TEST_F(UT_PageDriverInstallInfo, SlotDownloadFinished_AfterAdd_NoCrashAndRowsPreserved)
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
    m_page->slotDownloadFinished(0, ST_SUCESS);

    // Assert
    EXPECT_NO_FATAL_FAILURE({
        m_page->slotDownloadFinished(0, ST_SUCESS);
    });
    int rows = m_page->mp_ViewNotInstall->model()->rowCount();
    EXPECT_GT(rows, 0);
    EXPECT_EQ(1, rows);

    delete info;
}
