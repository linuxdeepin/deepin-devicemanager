#ifndef PAGEDRIVERMANAGER_H
#define PAGEDRIVERMANAGER_H

#include "MacroDefinition.h"

#include <DWidget>
#include <DFrame>
#include <DLabel>
#include <DStackedWidget>

#include <QObject>
#include <QThread>

class PageDriverTableView;
class DetectedStatusWidget;
class DriverScanWidget;
class QHBoxLayout;
class DriverScanner;

//class DFrame;

DWIDGET_USE_NAMESPACE

class PageDriverManager : public DWidget
{
    Q_OBJECT
public:
    explicit PageDriverManager(DWidget *parent = nullptr);
    virtual ~PageDriverManager();

    /**
     * @brief addDriverInfo
     * @param info
     */
    void addDriverInfo(DriverInfo *info);

    /**
     * @brief hasScanned 是否是第一次扫描
     * @return
     */
    bool isFirstScan();

    /**
     * @brief isInstalling：判断当前是否是安装界面
     * @return
     */
    bool isInstalling();

public slots:

    /**
     * @brief scanDriverInfo 开始扫描驱动
     */
    void scanDriverInfo();

private slots:
    /**
     * @brief slotDriverOperationClicked 处理点击信号安装更新操作处理
     */
    void slotDriverOperationClicked(int index);

    /**
     * @brief slotItemCheckedClicked 处理选中按钮
     * @param index
     * @param checked
     */
    void slotItemCheckedClicked(int index, bool checked);

    /**
     * @brief slotDownloadProgressChanged 驱动下载时回调，返回驱动下载进度、速度、已下载大小信息
     */
    void slotDownloadProgressChanged(QStringList msg);

    /**
     * @brief slotDownloadFinished 驱动下载完成
     */
    void slotDownloadFinished();

    /**
     * @brief slotInstallProgressChanged 驱动安装时回调，返回安装进度信息
     */
    void slotInstallProgressChanged(int progress);

    /**
     * @brief slotInstallProgressFinished 驱动安装完成回调，返回错误信息
     */
    void slotInstallProgressFinished(bool bsuccess, int err);

    /**
     * @brief slotInstallAllDrivers
     */
    void slotInstallAllDrivers();

    /**
     * @brief slotScanInfo 时时更新扫描信息
     */
    void slotScanInfo(const QString &info, int progress);

    /**
     * @brief slotScanFinished 扫描结束
     */
    void slotScanFinished(ScanResult sr);

    /**
     * @brief slotUndoInstall 取消安装
     */
    void slotUndoInstall();

signals:
    void startScanning(QList<DriverInfo *> lstInfo);

private:
    /**
     * @brief initWidget 初始化界面
     */
    void initWidget();

    /**
     * @brief initTable 初始化表格信息
     */
    void initTable();

    /**
     * @brief initMainFrame
     */
    void initMainFrame(DFrame *mainFrame);

    /**
     * @brief initHeadWidget 初始化表头编辑
     * @param hLayout
     */
    void initHeadWidget(QHBoxLayout *hLayout);

    /**
     * @brief initScrollArea 在frame中添加scrollarea
     * @param frame
     */
    void initScrollArea(DScrollArea *area);

    /**
     * @brief addDriverInfo 添加可以安装和可以更新的驱动
     * @param info 驱动信息
     */
    void addDriverInfoToTableView(DriverInfo *info, int index);

    /**
     * @brief addCurDriverInfo 当驱动为最新时，不更新
     * @param info
     */
    void addCurDriverInfo(DriverInfo *info);

    /**
     * @brief installNextDriver
     */
    void installNextDriver();

    /**
     * @brief scanDevices 从硬件信息中扫描信息
     */
    void scanDevices();
    void testScanDevices();

    /**
     * @brief scanDevices 扫描某个类型的硬件的驱动信息
     * @param deviceType 设备类型
     */
    void scanDevicesInfo(const QString &deviceType, DriverType driverType);

    /**
     * @brief showTables 决定显示哪些列表，可安装，可更新，无需安装
     */
    void showTables();

    /**
     * @brief clearAllData 清理数据
     */
    void clearAllData();

    /**
     * @brief addToDriverIndex
     * @param index
     */
    void addToDriverIndex(int index);

    /**
     * @brief removeFromDriverIndex
     * @param index
     */
    void removeFromDriverIndex(int index);

private:
    DStackedWidget       *mp_StackWidget;
    PageDriverTableView  *mp_ViewNotInstall; //没有安装驱动的列表
    PageDriverTableView  *mp_ViewCanUpdate;  //可以更新的驱动列表
    PageDriverTableView  *mp_AllDriverIsNew; //当所有驱动为最新时，显示此page
    DetectedStatusWidget *mp_HeadWidget;
    DriverScanWidget     *mp_ScanWidget;
    DLabel               *mp_InstallLabel;
    DLabel               *mp_UpdateLabel;
    DLabel               *mp_LabelIsNew;
    QList<DriverInfo *>   m_ListDriverInfo;  // 保存所有驱动信息
    DriverInfo          *mp_CurDriverInfo;
    int                  m_CurIndex;
    int                  m_CancelIndex;
    QList<int>           m_ListDriverIndex;
    QList<int>           m_ListInstallIndex;
    QList<int>           m_ListUpdateIndex;
    QList<int>           m_ListNewIndex;

    DriverScanner       *mp_scanner;     // 扫描驱动线程

};

#endif // PAGEDRIVERMANAGER_H
