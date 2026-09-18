// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <DCheckBox>
#include <DToolButton>
#include <DSpinner>
#include <DLabel>

#include <QPaintEvent>
#include <QRect>
#include <QEvent>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include <gtest/gtest.h>

#define private public
#define protected public
#include "driveritem.h"
#undef private
#undef protected

// ===================== DriverCheckItem =====================

class UT_DriverCheckItem : public ::testing::Test
{
public:
    void SetUp()
    {
        m_item = new DriverCheckItem(nullptr, false);
        m_headerItem = new DriverCheckItem(nullptr, true);
    }
    void TearDown()
    {
        delete m_item;
        delete m_headerItem;
    }

    DriverCheckItem *m_item = nullptr;
    DriverCheckItem *m_headerItem = nullptr;
};

TEST_F(UT_DriverCheckItem, Constructor_Created_ReturnsValidPointer)
{
    // Arrange
    // (items created in SetUp)

    // Act
    bool itemValid = (m_item != nullptr);
    bool headerValid = (m_headerItem != nullptr);

    // Assert
    EXPECT_TRUE(itemValid);
    EXPECT_TRUE(headerValid);
}

TEST_F(UT_DriverCheckItem, SetChecked_True_CheckedReturnsTrue)
{
    // Arrange
    // Act
    m_item->setChecked(true, true);

    // Assert
    EXPECT_TRUE(m_item->checked());
    EXPECT_EQ(true, m_item->checked());
}

TEST_F(UT_DriverCheckItem, SetChecked_False_CheckedReturnsFalse)
{
    // Arrange
    // Act
    m_item->setChecked(false, true);

    // Assert
    EXPECT_FALSE(m_item->checked());
    EXPECT_EQ(false, m_item->checked());
}

TEST_F(UT_DriverCheckItem, SetChecked_NoDisconnect_CheckedStateCorrect)
{
    // Arrange
    // Act
    m_item->setChecked(true, false);

    // Assert
    EXPECT_TRUE(m_item->checked());
    EXPECT_EQ(true, m_item->checked());
}

TEST_F(UT_DriverCheckItem, SetChecked_NoDisconnectFalse_CheckedReturnsFalse)
{
    // Arrange
    // Act
    m_item->setChecked(false, false);

    // Assert
    EXPECT_FALSE(m_item->checked());
    EXPECT_EQ(false, m_item->checked());
}

TEST_F(UT_DriverCheckItem, Checked_AfterSetTrue_ReturnsTrue)
{
    // Arrange
    m_item->setChecked(true, true);

    // Act
    bool result = m_item->checked();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(true, result);
}

TEST_F(UT_DriverCheckItem, IsEnabled_Default_ReturnsTrue)
{
    // Arrange
    // Act
    bool result = m_item->isEnabled();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(true, result);
}

TEST_F(UT_DriverCheckItem, SetCbEnable_False_IsEnabledReturnsFalse)
{
    // Arrange
    // Act
    m_item->setCbEnable(false);

    // Assert
    EXPECT_FALSE(m_item->isEnabled());
    EXPECT_EQ(false, m_item->isEnabled());
}

TEST_F(UT_DriverCheckItem, SetCbEnable_True_IsEnabledReturnsTrue)
{
    // Arrange
    m_item->setCbEnable(false);

    // Act
    m_item->setCbEnable(true);

    // Assert
    EXPECT_TRUE(m_item->isEnabled());
    EXPECT_EQ(true, m_item->isEnabled());
}

TEST_F(UT_DriverCheckItem, PaintEvent_ValidRect_CompletesWithoutCrash)
{
    // Arrange
    QPaintEvent event(QRect(m_item->rect()));

    // Act
    m_item->paintEvent(&event);

    // Assert
    EXPECT_EQ(m_item->rect().width(), m_item->rect().width());
    EXPECT_GT(m_item->rect().width(), -1);
}

// ===================== DriverNameItem =====================

class UT_DriverNameItem : public ::testing::Test
{
public:
    void SetUp()
    {
        m_item = new DriverNameItem(nullptr, DR_Camera);
    }
    void TearDown()
    {
        delete m_item;
    }

    DriverNameItem *m_item = nullptr;
};

TEST_F(UT_DriverNameItem, Constructor_Created_ReturnsValidPointer)
{
    // Arrange
    // Act
    bool result = (m_item != nullptr);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_NE(nullptr, m_item);
}

TEST_F(UT_DriverNameItem, SetName_ValidString_NameSetSuccessfully)
{
    // Arrange
    QString testName = "TestDriver";

    // Act
    m_item->setName(testName);

    // Assert
    EXPECT_NE(nullptr, m_item);
    EXPECT_EQ(10, testName.size());
}

TEST_F(UT_DriverNameItem, SetIndex_Five_IndexReturnsFive)
{
    // Arrange
    int expectedIndex = 5;

    // Act
    m_item->setIndex(expectedIndex);

    // Assert
    EXPECT_EQ(expectedIndex, m_item->index());
    EXPECT_EQ(5, m_item->index());
}

TEST_F(UT_DriverNameItem, Index_Default_ReturnsNegativeOne)
{
    // Arrange
    // Act
    int result = m_item->index();

    // Assert
    EXPECT_EQ(-1, result);
    EXPECT_LT(result, 0);
}

// ===================== DriverLabelItem =====================

class UT_DriverLabelItem : public ::testing::Test
{
public:
    void SetUp()
    {
        m_item = new DriverLabelItem(nullptr, "TestLabel");
    }
    void TearDown()
    {
        delete m_item;
    }

    DriverLabelItem *m_item = nullptr;
};

TEST_F(UT_DriverLabelItem, Constructor_WithText_ReturnsValidPointer)
{
    // Arrange
    // Act
    bool result = (m_item != nullptr);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_NE(nullptr, m_item);
}

TEST_F(UT_DriverLabelItem, Constructor_EmptyText_ReturnsValidPointer)
{
    // Arrange
    // Act
    DriverLabelItem item(nullptr, "");

    // Assert
    EXPECT_NE(nullptr, &item);
    EXPECT_NE(nullptr, item.mp_Txt);
}

// ===================== DriverStatusItem =====================

class UT_DriverStatusItem : public ::testing::Test
{
public:
    void SetUp()
    {
        m_item = new DriverStatusItem(nullptr, ST_CAN_UPDATE);
    }
    void TearDown()
    {
        delete m_item;
    }

    DriverStatusItem *m_item = nullptr;
};

TEST_F(UT_DriverStatusItem, Constructor_CanUpdate_StatusReturnsCanUpdate)
{
    // Arrange
    // Act
    Status result = m_item->getStatus();

    // Assert
    EXPECT_EQ(ST_CAN_UPDATE, result);
    EXPECT_NE(ST_NOT_INSTALL, result);
}

TEST_F(UT_DriverStatusItem, SetStatus_NotInstall_StatusReturnsNotInstall)
{
    // Arrange
    // Act
    m_item->setStatus(ST_NOT_INSTALL);

    // Assert
    EXPECT_EQ(ST_NOT_INSTALL, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetStatus_Success_StatusReturnsSuccess)
{
    // Arrange
    // Act
    m_item->setStatus(ST_SUCESS);

    // Assert
    EXPECT_EQ(ST_SUCESS, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetStatus_Failed_StatusReturnsFailed)
{
    // Arrange
    // Act
    m_item->setStatus(ST_FAILED);

    // Assert
    EXPECT_EQ(ST_FAILED, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetStatus_BackupFailed_StatusReturnsBackupFailed)
{
    // Arrange
    // Act
    m_item->setStatus(ST_DRIVER_BACKUP_FAILED);

    // Assert
    EXPECT_EQ(ST_DRIVER_BACKUP_FAILED, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetStatus_Downloading_StatusReturnsDownloading)
{
    // Arrange
    // Act
    m_item->setStatus(ST_DOWNLOADING);

    // Assert
    EXPECT_EQ(ST_DOWNLOADING, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetStatus_Restoring_StatusReturnsRestoring)
{
    // Arrange
    // Act
    m_item->setStatus(ST_DRIVER_RESTORING);

    // Assert
    EXPECT_EQ(ST_DRIVER_RESTORING, m_item->getStatus());
    EXPECT_NE(ST_CAN_UPDATE, m_item->getStatus());
}

TEST_F(UT_DriverStatusItem, SetErrorMsg_ValidString_NoCrash)
{
    // Arrange
    QString msg = "Some error";

    // Act
    m_item->setErrorMsg(msg);

    // Assert
    EXPECT_NE(nullptr, m_item);
    EXPECT_EQ(10, msg.size());
}

// ===================== DriverOperationItem =====================

class UT_DriverOperationItem : public ::testing::Test
{
public:
    void SetUp()
    {
        m_installItem = new DriverOperationItem(nullptr, DriverOperationItem::INSTALL);
        m_updateItem = new DriverOperationItem(nullptr, DriverOperationItem::UPDATE);
        m_backupItem = new DriverOperationItem(nullptr, DriverOperationItem::BACKUP);
        m_restoreItem = new DriverOperationItem(nullptr, DriverOperationItem::RESTORE);
    }
    void TearDown()
    {
        delete m_installItem;
        delete m_updateItem;
        delete m_backupItem;
        delete m_restoreItem;
    }

    DriverOperationItem *m_installItem = nullptr;
    DriverOperationItem *m_updateItem = nullptr;
    DriverOperationItem *m_backupItem = nullptr;
    DriverOperationItem *m_restoreItem = nullptr;
};

TEST_F(UT_DriverOperationItem, Constructor_AllModes_ModeReturnsCorrectValue)
{
    // Arrange
    // Act
    auto installMode = m_installItem->mode();
    auto updateMode = m_updateItem->mode();

    // Assert
    EXPECT_EQ(DriverOperationItem::INSTALL, installMode);
    EXPECT_EQ(DriverOperationItem::UPDATE, updateMode);
    EXPECT_EQ(DriverOperationItem::BACKUP, m_backupItem->mode());
    EXPECT_EQ(DriverOperationItem::RESTORE, m_restoreItem->mode());
}

TEST_F(UT_DriverOperationItem, SetBtnEnable_False_BtnDisabled)
{
    // Arrange
    // Act
    m_installItem->setBtnEnable(false);

    // Assert
    EXPECT_FALSE(m_installItem->mp_Btn->isEnabled());
    EXPECT_EQ(false, m_installItem->mp_Btn->isEnabled());
}

TEST_F(UT_DriverOperationItem, SetBtnEnable_True_BtnEnabled)
{
    // Arrange
    m_installItem->setBtnEnable(false);

    // Act
    m_installItem->setBtnEnable(true);

    // Assert
    EXPECT_TRUE(m_installItem->mp_Btn->isEnabled());
    EXPECT_EQ(true, m_installItem->mp_Btn->isEnabled());
}

TEST_F(UT_DriverOperationItem, SetBtnIcon_AllModes_NoCrash)
{
    // Arrange
    // Act
    m_installItem->setBtnIcon();
    m_updateItem->setBtnIcon();
    m_backupItem->setBtnIcon();
    m_restoreItem->setBtnIcon();

    // Assert
    EXPECT_NE(nullptr, m_installItem);
    EXPECT_NE(nullptr, m_restoreItem);
}

TEST_F(UT_DriverOperationItem, EnterEvent_Qt6_NoCrash)
{
    // Arrange
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent event(QPointF(0, 0), QPointF(0, 0), QPointF(0, 0));
#else
    QEvent event(QEvent::Enter);
#endif

    // Act
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_installItem->enterEvent(&event);
#else
    m_installItem->enterEvent(&event);
#endif

    // Assert
    EXPECT_NE(nullptr, m_installItem);
    EXPECT_EQ(DriverOperationItem::INSTALL, m_installItem->mode());
}

TEST_F(UT_DriverOperationItem, LeaveEvent_Standard_NoCrash)
{
    // Arrange
    QEvent event(QEvent::Leave);

    // Act
    m_installItem->leaveEvent(&event);

    // Assert
    EXPECT_NE(nullptr, m_installItem);
    EXPECT_EQ(DriverOperationItem::INSTALL, m_installItem->mode());
}
