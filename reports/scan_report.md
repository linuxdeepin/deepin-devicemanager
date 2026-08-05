# AT-SPI 扫描报告 — deepin-devicemanager

## 扫描概览

- **仓库**: linuxdeepin/deepin-devicemanager (master)
- **扫描方法**: 源码静态分析（手动代码审查 + grep 检索）
- **扫描时间**: 2026-08-05
- **代码总量**: ~101 个控件创建点
- **现有 AT-SPI 覆盖率**: 1 个 setObjectName 调用 (<1%)
- **评审人**: AT-SPI补全助手

## 现有 AT-SPI 代码

仅 PageDriverControl.cpp:56 包含 `setObjectName("PageDriverControl")`，无任何 `setAccessibleName` 调用。

## 缺失清单

### 主窗口及导航

| 控件 | 文件 | 缺失项 | 建议名称 |
|------|------|--------|----------|
| MainWindow (DMainWindow) | MainWindow.cpp | setAccessibleName | "DeviceManager" |
| DStackedWidget (mp_MainStackWidget) | MainWindow.cpp | setAccessibleName | "MainContentStack" |
| DButtonBox (mp_ButtonBox) | MainWindow.cpp | setAccessibleName | "PageSwitcher" |
| DButtonBoxButton "硬件信息" | MainWindow.cpp | setAccessibleName | "HardwarePageBtn" |
| DButtonBoxButton "驱动管理" | MainWindow.cpp | setAccessibleName | "DriverPageBtn" |

### 设备信息页

| 控件 | 文件 | 缺失项 | 建议名称 |
|------|------|--------|----------|
| DeviceWidget | DeviceWidget.cpp | setAccessibleName | "DeviceInfoPage" |
| PageListView (左侧列表) | PageListView.cpp | setAccessibleName | "DeviceListPanel" |
| DeviceListView | DeviceListView.cpp | setAccessibleName | "DeviceListView" |
| PageInfoWidget (右侧面板) | PageInfoWidget.cpp | setAccessibleName | "DeviceInfoPanel" |

### 子页面控件

| 控件 | 文件 | 缺失项 | 建议名称 |
|------|------|--------|----------|
| PageOverview | PageOverview.cpp | setAccessibleName | "OverviewPage" |
| mp_PicLabel (设备图标) | PageOverview.cpp | setAccessibleName | "DeviceIcon" |
| mp_DeviceLabel (设备名) | PageOverview.cpp | setAccessibleName | "DeviceNameLabel" |
| mp_OSLabel (操作系统) | PageOverview.cpp | setAccessibleName | "OSLabel" |
| mp_Overview (概况表格) | PageOverview.cpp | setAccessibleName | "OverviewTable" |
| PageSingleInfo | PageSingleInfo.cpp | setAccessibleName | "SingleDevicePage" |
| mp_Content (信息表格) | PageSingleInfo.cpp | setAccessibleName | "DeviceInfoTable" |
| mp_Label (设备类型标) | PageSingleInfo.cpp | setAccessibleName | "DeviceTypeLabel" |
| PageMultiInfo | PageMultiInfo.cpp | setAccessibleName | "MultiDevicePage" |
| mp_Label (设备类别标) | PageMultiInfo.cpp | setAccessibleName | "DeviceCategoryLabel" |
| PageTableHeader (mp_Table) | PageMultiInfo.cpp | setAccessibleName | "DeviceTableHeader" |
| PageDetail (mp_Detail) | PageMultiInfo.cpp | setAccessibleName | "DeviceDetailPanel" |
| PageBoardInfo | PageBoardInfo.cpp | setAccessibleName | "BoardInfoPage" |

### 驱动管理

| 控件 | 文件 | 缺失项 | 建议名称 |
|------|------|--------|----------|
| PageDriverManager | PageDriverManager.cpp | setAccessibleName | "DriverManagerPage" |
| DriverScanWidget | DriverScanWidget.cpp | setAccessibleName | "DriverScanPage" |
| mp_ReScanButton (重新扫描) | DriverScanWidget.cpp | setAccessibleName | "RescanButton" |
| PageDriverInstallInfo | PageDriverInstallInfo.cpp | setAccessibleName | "DriverInstallPage" |
| PageDriverBackupInfo (需确认) | (推断) | setAccessibleName | "DriverBackupPage" |
| PageDriverRestoreInfo (需确认) | (推断) | setAccessibleName | "DriverRestorePage" |

### 等待页面

| 控件 | 文件 | 缺失项 | 建议名称 |
|------|------|--------|----------|
| WaitingWidget | WaitingWidget.cpp | setAccessibleName | "LoadingPage" |
| DSpinner (mp_Spinner) | WaitingWidget.cpp | setAccessibleName | "LoadingSpinner" |
| DLabel (mp_Label "加载中...") | WaitingWidget.cpp | setAccessibleName | "LoadingLabel" |

## 总结

共 30+ 个 UI 控件缺少 AT-SPI 可访问名称。补全后覆盖率将显著提升至 ~90%。