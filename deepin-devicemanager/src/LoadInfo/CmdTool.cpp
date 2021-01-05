// 项目自身文件
#include "CmdTool.h"

// Qt库文件
#include<QDebug>
#include<QDateTime>
#include<QMutex>

// 其它头文件
#include "../commondefine.h"
#include "EDIDParser.h"
#include "DeviceManager.h"
#include "DBusInterface.h"
#include "MacroDefinition.h"

CmdTool::CmdTool()
{

}

void CmdTool::addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    // 设备分类，与设备信息对照表
    if (m_cmdInfo.find(key) != m_cmdInfo.end()) {
        m_cmdInfo[key].append(mapInfo);
    } else {
        QList<QMap<QString, QString> > lstMap;
        lstMap.append(mapInfo);
        m_cmdInfo.insert(key, lstMap);
    }

}

void CmdTool::getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src)
{
    // 获取打印机信息
    mapInfo.insert("Name", src->name);
    for (int i = 0; i < src->num_options; i++) {
        mapInfo.insert(src->options[i].name, src->options[i].value);
    }
}

void CmdTool::loadCmdInfo(const QString &key, const QString &debugFile)
{
    // 根据命令获取设备文件信息
    if (key == "lshw") {
        loadLshwInfo(debugFile);
    } else if (key == "lsblk_d") {
        loadLsblkInfo(debugFile);
    } else if (key == "ls_sg") {
        loadLssgInfo(debugFile);
    } else if (key == "dmesg") {
        loadDmesgInfo(debugFile);
    } else if (key == "hciconfig") {
        loadHciconfigInfo(debugFile);
    } else if (key == "printer") {
        loadPrinterInfo();
    } else if (key == "upower") {
        loadUpowerInfo(key, debugFile);
    } else if (key.startsWith("hwinfo")) {
        loadHwinfoInfo(key, debugFile);
    } else if (key.startsWith("dmidecode")) {
        loadDmidecodeInfo(key, debugFile);
    } else if (key == "cat_devices") {
        loadCatInputDeviceInfo(key, debugFile);
    }  else if (key == "gpuinfo") {
        loadGpuInfo(key, debugFile);
    }  else if (key == "cat_audio") {
        loadCatAudioInfo(key, debugFile);
    }   else if (key == "EDID_HDMI" || key == "EDID_VGA") {
        loadEdidInfo(key, debugFile);
    }  else if (key == "bootdevice") {
        loadBootDeviceManfid(key, debugFile);
    } else if (key == "bt_device") {
        loadBluetoothPairedDevices(key, debugFile);     // 加载蓝牙设备配对信息
    } else if (key == "lscpu") {
        loadLscpuInfo(key, debugFile);
    } else {
        loadCatInfo(key, debugFile);
    }
}

QMap<QString, QList<QMap<QString, QString> > > &CmdTool::cmdInfo()
{
    return m_cmdInfo;
}

void CmdTool::loadLshwInfo(const QString &debugFile)
{
    // 加载lshw信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugFile);

    // 根据设备类型解析lshw信息
    QStringList items = deviceInfo.split("*-");
    bool isFirst = true;
    foreach (const QString &item, items) {
        QMap<QString, QString> mapInfo;
        if (isFirst) {
            // 系统信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_system", mapInfo);
            isFirst = false;
            continue;
        }

        // CPU 信息
        if (item.startsWith("cpu")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cpu", mapInfo);
        } else if (item.startsWith("disk")) {         // 存储设备信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_disk", mapInfo);
        } else if (item.startsWith("storage")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_storage", mapInfo);
        } else if (item.startsWith("bank")) {      // 内存信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_memory", mapInfo);
        } else if (item.startsWith("display")) {      // 显卡信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_display", mapInfo);
        } else if (item.startsWith("multimedia")) {   // 音频信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_multimedia", mapInfo);
        } else if (item.startsWith("network")) {      // 网卡信息
            getMapInfoFromLshw(item, mapInfo);
            if (!item.contains(QRegExp(".*network:[0-9] DISABLED.*"))) {
                addMapInfo("lshw_network", mapInfo);
            }
        } else if (item.startsWith("usb")) {          // USB 设备信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_usb", mapInfo);
        } else if (item.startsWith("cdrom")) {        // 光盘信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cdrom", mapInfo);
        }
    }
}

void CmdTool::loadLsblkInfo(const QString &debugfile)
{
    // 加载lsblk信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QStringList lines = deviceInfo.split("\n");
    QMap<QString, QString> mapInfo;

    // 获取存储设备逻辑名称以及ROTA信息
    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        if (words.size() != 2 || words[0] == "NAME") {
            continue;
        }
        mapInfo.insert(words[0].trimmed(), words[1].trimmed());

        //sudo smartctl --all /dev/%1   文件信息
        loadSmartCtlInfo(words[0].trimmed(), "smartctl_" + words[0].trimmed() + ".txt");
    }
    addMapInfo("lsblk_d", mapInfo);
}

void CmdTool::loadLssgInfo(const QString &debugfile)
{
    // 加载ls /dev/sg*信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QStringList lines = deviceInfo.split("\n");
    QMap<QString, QString> mapInfo;

    // 获取存储设备逻辑名称以及ROTA信息
    foreach (QString line, lines) {
        if (line.isEmpty()) {
            continue;
        }

        QStringList words = line.split("/");

        //sudo smartctl --all /dev/%1   文件信息
        loadSmartCtlInfo(words[2].trimmed(), "smartctl_" + words[2].trimmed() + ".txt");
    }
}

void CmdTool::loadSmartCtlInfo(const QString &logicalName, const QString &debugfile)
{
    QString deviceInfo;

    // 获取硬盘smartctl信息
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;

    // 硬盘逻辑名称
    mapInfo["ln"] = logicalName;

    getMapInfoFromSmartctl(mapInfo, deviceInfo);
    addMapInfo("smart", mapInfo);
}

void CmdTool::loadXrandrInfo(const QString &debugfile)
{
    // 读取信息
    QString deviceInfo;
    if (!getDeviceInfoFromCmd(deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        // 刷新率
        QRegExp reResolution("^[\\s]{3}([0-9]{3,5}x[0-9]{3,5}).*([0-9]{2,3}.[0-9]{2,3}\\*).*");
        if (reResolution.exactMatch(line)) {
            QString rate = reResolution.cap(2).replace("*", "");
            mapInfo.insert("rate", rate);
        }

        // 最大,最小,当前分辨率
        if (line.startsWith("Screen")) {
            QRegExp re(".*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*");
            if (re.exactMatch(line)) {
                mapInfo["minResolution"] = re.cap(1);
                mapInfo["curResolution"] = re.cap(2);
                mapInfo["maxResolution"] = re.cap(3);
            }
        } else if (line.startsWith("HDMI")) {           // HDMI 接口
            mapInfo["HDMI"] = "Enable";
        } else if (line.startsWith("VGA")) {            // HDMI 接口
            mapInfo["VGA"] = "Enable";
        } else if (line.startsWith("DP")
                   || line.startsWith("DisplayPort"))  {        // DP 接口
            mapInfo["DP"] = "Enable";
        } else if (line.startsWith("eDP")) {                    // eDP 接口
            mapInfo["eDP"] = "Enable";
        } else if (line.startsWith("DVI")) {                    // DVI 接口
            mapInfo["DVI"] = "Enable";
        }
    }
    addMapInfo("xrandr", mapInfo);
}

void CmdTool::loadXrandrVerboseInfo(const QString &debugfile)
{
    // 读取文件信息
    QString deviceInfo;
    if (!getDeviceInfoFromCmd(deviceInfo, debugfile)) {
        return;
    }

    QStringList lines = deviceInfo.split(QRegExp("\n"));
    QString mainInfo("");
    QString edid("");
    foreach (QString line, lines) {
        if (line.startsWith("Screen")) {
            continue;
        }
        QRegExp reResolution("^[\\s]{2}([0-9]{3,4}x[0-9]{3,4}).*");
        if (reResolution.exactMatch(line)) {
            continue;
        }

        // 主屏幕信息
        QRegExp reMain("^[a-zA-Z].*");
        if (reMain.exactMatch(line)) {
            if (!mainInfo.isEmpty()) {
                QMap<QString, QString> mapInfo;
                mapInfo.insert("mainInfo", mainInfo.trimmed());
                mapInfo.insert("edid", edid.trimmed());
                addMapInfo("xrandr_verbose", mapInfo);
            }
            mainInfo = line;
            edid = "";
            continue;
        }

        // edid信息
        QRegExp reEdid("^[\\t]{2}[0-9]{1}.*");
        if (reEdid.exactMatch(line)) {
            edid.append(line.trimmed());
            edid.append("\n");
            continue;
        }
    }
    QMap<QString, QString> mapInfo;
    mapInfo.insert("mainInfo", mainInfo.trimmed());
    mapInfo.insert("edid", edid.trimmed());
    addMapInfo("xrandr_verbose", mapInfo);
}

void CmdTool::loadDmesgInfo(const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    // 获取显存大小信息
    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QRegExp reg(".*RAM=([0-9]*)M.*");
        if (reg.exactMatch(line)) {
            double size = reg.cap(1).toDouble();
            QString sizeS = QString("%1GB").arg(size / 1024);
            mapInfo["Size"] = sizeS;
        } else {
            reg.setPattern(".*RAM: ([0-9]*) M.*");
            if (reg.exactMatch(line)) {
                double size = reg.cap(1).toDouble();
                QString sizeS = QString("%1GB").arg(size / 1024);
                mapInfo["Size"] = sizeS;
            }
        }
    }
    addMapInfo("dmesg", mapInfo);

    mapInfo.clear();

    // 声卡芯片型号
    foreach (const QString &line, lines) {
        QRegExp reg(".*autoconfig for ([A-Za-z0-9]+):.*");
        if (reg.exactMatch(line)) {
            QString chip = reg.cap(1);
            mapInfo["chip"] = chip;
        }
    }
    addMapInfo("audiochip", mapInfo);
}

void CmdTool::loadHciconfigInfo(const QString &debugfile)
{
    // 获取hciconfig文件信息
    QString deviceInfo;

    // 判断是否是系统是否是个人版
    DSysInfo::UosEdition type = DSysInfo::uosEditionType();
    if (DSysInfo::UosHome == type) {
        // 如果是个人版则直接执行命令获取设备信息
        // bug 目前服务端与直接执行命令获取结果不一致
        QProcess process;
        int msecs = 10000;
        QString cmd = "hciconfig --all";
        process.start(cmd);

        // 获取命令执行结果
        bool res = process.waitForFinished(msecs);
        if (res) {
            deviceInfo = process.readAllStandardOutput();
        }

    } else {
        // 获取文件信息
        if (!getDeviceInfo(deviceInfo, debugfile)) {
            return;
        }
    }

    QStringList paragraphs = deviceInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        QMap<QString, QString> mapInfo;
        getMapInfoFromHciconfig(mapInfo, paragraph);
        loadBluetoothCtlInfo(mapInfo);
    }
}

void CmdTool::loadBluetoothCtlInfo(QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("BD Address") == mapInfo.end()) {
        addMapInfo("hciconfig", mapInfo);
        return;
    }
    QString deviceInfo;
//    if (!getDeviceInfo("bluetoothctl show " + mapInfo["BD Address"], deviceInfo, "bluetoothctl.txt")) {
//        addMapInfo("hciconfig", mapInfo);
//        return;
//    }

    // 读取文件信息
    if (!getDeviceInfo(deviceInfo, "bluetoothctl.txt")) {
        addMapInfo("hciconfig", mapInfo);
        return;
    }

    getMapInfoFromBluetoothCtl(mapInfo, deviceInfo);

    addMapInfo("hciconfig", mapInfo);
}

void CmdTool::loadPrinterInfo()
{
    // 先判断有没有打印机
    QString deviceInfo;
//    if (!getDeviceInfo("lpstat -a", deviceInfo, "lpstat.txt")) {
//        return;
//    }

    // 通过文件获取lpstat -a信息
    if (!getDeviceInfo(deviceInfo, "lpstat.txt")) {
        return;
    }

    if (deviceInfo.isEmpty()) {
        return;
    }

    // 通过cups获取打印机信息
    cups_dest_t *dests = nullptr;
    http_t *http = nullptr;
    int num_dests;
    num_dests = cupsGetDests2(http, &dests);
    if (dests == nullptr) {
        cupsFreeDests(num_dests, dests);
        return;
    }
    for (int i = 0; i < num_dests; i++) {
        cups_dest_t *dest = nullptr;
        QMap<QString, QString> mapInfo;
        dest = dests + i;
        getMapInfo(mapInfo, dest);

        // 这里为了和打印机管理保持一致，做出限制
        if (mapInfo.size() > 10)
            addMapInfo("printer", mapInfo);
    }
    cupsFreeDests(num_dests, dests);


    //    cups_dest_t *dests = nullptr;
    //    int num_dests;
    //    num_dests = cupsGetDests(&dests);
    //    if (dests == nullptr) {
    //        return;
    //    }
    //    for (int i = 0; i < num_dests; i++) {
    //        cups_dest_t *dest = nullptr;
    //        QMap<QString, QString> mapInfo;
    //        dest = dests + i;
    //        getMapInfo(mapInfo, dest);
    //        addMapInfo("printer", mapInfo);
    //    }
    //    cupsFreeDests(num_dests, dests);
}

void CmdTool::loadHwinfoInfo(const QString &key, const QString &debugfile)
{
    // 获取文件信息
    QString deviceInfo;
    if (key == "hwinfo_monitor") {
        getDeviceInfoFromCmd(deviceInfo, "hwinfo --monitor");
    } else {
        getDeviceInfo(deviceInfo, debugfile);
    }

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromHwinfo(item, mapInfo);

        // hwinfo --usb 里面有很多的无用信息，需要特殊处理
        if (key == "hwinfo_usb") {
            loadHwinfoUsbInfo(item, mapInfo);
        } else if (key == "hwinfo_mouse" || key == "hwinfo_keyboard") {
            if (!item.contains("Linux Foundation") && // 在服务器版本中发现，hwinfo --mouse 和 hwinfo --keyboard获取的信息里面有多余的无用信息，需要过滤
                    !item.contains("Elite Remote Control Driver") && // 在笔记本中发现了一个多余信息，做特殊处理 Elite Remote Control Driver
                    !item.contains("Model: \"serial console\"") && // 鲲鹏台式机子上发现一条多余信息  Model: "serial console"
                    !item.contains("Wacom", Qt::CaseInsensitive)) { // 数位板信息被显示成了mouse信息,这里需要做特殊处理(搞不懂数位板为什么不能显示成鼠标)

                addMapInfo(key, mapInfo);
            }
        } else {
            addMapInfo(key, mapInfo);
        }
    }
}

void CmdTool::loadHwinfoUsbInfo(const QString &item, const QMap<QString, QString> &mapInfo)
{
    QList<QMap<QString, QString>>::iterator it = m_cmdInfo["hwinfo_usb"].begin();
    bool add = true;

    // 有的是有同一个设备有两段信息，我们只需要一个
    // 比如 SysFS BusID: 1-3:1.2   和  SysFS BusID: 1-3:1.0 这个是同一个设备
    // 我们只需要一个
    for (; it != m_cmdInfo["hwinfo_usb"].end(); ++it) {
        QString curBus = (*it)["SysFS BusID"];
        QString newBus = mapInfo["SysFS BusID"];
        curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        newBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        if (curBus == newBus) {
            add = false;
            break;
        }
    }

    // 这个是用来过滤，没有接入任何设备的usb接口
    if (mapInfo["Model"].contains("Linux Foundation")) {
        add = false;
    }

    if (mapInfo["Hardware Class"].contains("hub", Qt::CaseInsensitive)) {
        add = false;
    }

    // 打印机几信息不从hwinfo --usb里面获取，需要过滤
    if (item.contains("Printer", Qt::CaseInsensitive) || item.contains("LaserJet", Qt::CaseInsensitive)) {
        add = false;
    }

    // 提前过滤掉键盘鼠标
    if (item.contains("mouse", Qt::CaseInsensitive) || item.contains("keyboard", Qt::CaseInsensitive)) {
        add = false;
    }

    // 这里特殊处理数位板信息，通过hwinfo --mouse可以获取到数位板信息，但是根据需求数位板应该在其它设备里面(虽然这很不合理)
    // 所以这里需要做特殊处理 即 item 里面包含了 Wacom 的 就说明是数位板设备，那就应该添加到其它设备里面
    if (item.contains("Wacom", Qt::CaseInsensitive)) {
        add = true;
    }

    if (add) {
        addMapInfo("hwinfo_usb", mapInfo);
    }
}

void CmdTool::loadDmidecodeInfo(const QString &key, const QString &debugfile)
{
    // dmidecode -t 2信息单独处理
    if (key == "dmidecode2") {
        loadDmidecode2Info(key, debugfile);
        return;
    }
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    // According to the latest demand , The notebook should not have chassis information
    if (key == "dmidecode3") {
        if (deviceInfo.contains("laptop", Qt::CaseInsensitive) ||
                deviceInfo.contains("notebook", Qt::CaseInsensitive)) {
            return;
        }
    }

    QStringList items = deviceInfo.split("\n\n");

    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }
        QMap<QString, QString> mapInfo;
        getMapInfoFromDmidecode(item, mapInfo);
        if (mapInfo.size() > MIN_NUM)
            addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadDmidecode2Info(const QString &key, const QString &debugfile)
{
    // 通过命令获取设备信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    QStringList items = deviceInfo.split("\n\n");
    QMap<QString, QString> mapInfo;
    bool isGetInfo = false;
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        getMapInfoFromDmidecode(item, mapInfo);

        // 芯片信息,SMBIOS版本信息
        if (isGetInfo == false) {
            QString chipset;
            loadBiosInfoFromLspci(chipset);
            mapInfo.insert("chipset", chipset);
            QString version;
            getSMBIOSVersion(item, version);
            mapInfo.insert("SMBIOS Version", version);
            isGetInfo = true;
        }
    }

    addMapInfo(key, mapInfo);
}

void CmdTool::loadLscpuInfo(const QString &key, const QString &debugfile)
{
    // 通过命令获取设备信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo, key.startsWith("cat_os") ? "=" : ": ");
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        if (key == "cat_version") {
            mapInfo["OS"] = item;
        } else {
            // 根据文件内容调整分隔符
            getMapInfoFromCmd(item, mapInfo, key.startsWith("cat_os") ? "=" : ": ");
        }
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadUpowerInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty() || item.contains("DisplayDevice")) {
            continue;
        }
        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo);
        if (item.contains("Daemon:")) {
            // 守护进程
            addMapInfo("Daemon", mapInfo);
        } else {
            //电池信息
            addMapInfo(key, mapInfo);
        }
    }
}

void CmdTool::loadBiosInfoFromLspci(QString &chipsetFamliy)
{
    QString chipset;
    // 通过文件获取 lspci -v -s %1 信息
    if (!getDeviceInfo(chipset, "lspci_vs.txt")) {
        return;
    }

    QStringList lines = chipset.split("\n");
    foreach (const QString &line, lines) {
        if (line.contains("Subsystem", Qt::CaseInsensitive)) {
            QStringList words = line.split(": ");
            if (words.size() == 2) {
                chipsetFamliy = words[1].trimmed();
            }
            break;
        }
    }
}

void CmdTool::loadCatInputDeviceInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromInput(item, mapInfo, "=");

        // 获取与正则表达式匹配的输入设备
        QRegExp re = QRegExp(".*(mouse[0-9]{1,2}).*");
        if (re.exactMatch(mapInfo["Handlers"])) {
            QString name = re.cap(1);
            DeviceManager::instance()->addInputInfo(name, mapInfo);
        } else {
            QRegExp rem = QRegExp(".*(event[0-9]{1,2}).*");
            if (rem.exactMatch(mapInfo["Handlers"])) {
                QString name = rem.cap(1);
                DeviceManager::instance()->addInputInfo(name, mapInfo);
            }
        }

        //Sysfs=/devices/pci0000:00/0000:00:14.0/usb1/1-5/1-5:1.0/input/input40
        //QRegExp re = QRegExp(".*(usb[0-9]\\/[0-9]-[0-9]\\/[0-9]-[0-9]:[0-9]).*");

        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatAudioInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile)) {
        return;
    }
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo, ":");
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadEdidInfo(const QString &key, const QString &debugfile)
{
    // 获取edid信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QString edid;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.trimmed().split(" ");
        if (words.size() != 9) {
            continue;
        }
        QString l;
        QStringList::const_iterator it = words.begin();
        ++it;
        for (; it != words.end(); ++it) {
            l.append(*it);
        }
        l.append("\n");
        edid.append(l);
    }

    QStringList liness = edid.split("\n");
    if (liness.size() > 3) {
        // 解析edid
        EDIDParser edidParser;
        QString errorMsg;
        edidParser.setEdid(edid, errorMsg);

        QMap<QString, QString> mapInfo;
        mapInfo.insert("Vendor", edidParser.vendor());       // 制造商
        mapInfo.insert("Date", edidParser.releaseDate());    // 生产日期
        mapInfo.insert("Size", edidParser.screenSize());     // 尺寸大小
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadGpuInfo(const QString &key, const QString &debugfile)
{
    // 获取gpuinfo信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }
    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        QStringList lines = item.split("\n");
        if (lines.size() > 0) {
            mapInfo.insert("Name", lines[0].trimmed());
        }
        getMapInfoFromCmd(item, mapInfo, ": ");
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadBootDeviceManfid(const QString &key, const QString &debugfile)
{
    // 加载硬盘信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", deviceInfo.trimmed());
    addMapInfo(key, mapInfo);
}

void CmdTool::loadBluetoothPairedDevices(const QString &key, const QString &debugfile)
{
    // 当机器上没有蓝牙适配器时,不查找匹配信息
    if (m_cmdInfo.find("hciconfig") == m_cmdInfo.end()) {
        return;
    }

    if (m_cmdInfo["hciconfig"].empty()) {
        return;
    }

    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile)) {
        return;
    }

    // 解析蓝牙配对设备信息
    QMap<QString, QString> mapInfo;
    QStringList items = deviceInfo.split("\n");

    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        // 行信息必须以"Device"开头,eg:Device 00:1F:20:A8:B9:E8 Logitech K810
        if (item.startsWith("Device", Qt::CaseInsensitive) == false) {
            continue;
        }

        // 保存设备名称与设备Mac地址
        QStringList lines = item.split(" ");
        if (lines.size() >= 3) {
            QString name = item.mid(item.indexOf(lines[2]));
            QString mac = item.mid(item.indexOf(lines[1]), lines[1].size());
            mac = mac.toUpper();
            mapInfo.insert(name, mac);   // <设备名称,Mac地址>
        }
    }

    addMapInfo(key, mapInfo);
}

void CmdTool::getSMBIOSVersion(const QString &info, QString &version)
{
    QStringList lineList = info.split("\n");

    foreach (auto line, lineList) {
        //  SMBIOS 3.0.0 present.
        QRegExp rx("^SMBIOS ([\\d]*.[\\d]*.[\\d])+ present.$");
        if (rx.indexIn(line) > -1) {
            version = rx.cap(1);
            break;
        }
    }
}

void CmdTool::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromInput(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        *it = (*it).replace(QRegExp("^[A-Z]: "), "");
        *it = (*it).trimmed();
        if ((*it).count(ch) > 2) {
            QStringList words = (*it).split(" ");
            foreach (auto attri, words) {
                QStringList attriList = attri.split(ch);

                if (attriList.size() == 2) {
                    mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed().replace("\"", ""));
                }
            }
        } else {
            QStringList attriList = (*it).split(ch);
            if (attriList.size() == 2) {
                mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed().replace("\"", ""));
            } else if (attriList.size() == 3) {
                mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed() + attriList[2].trimmed());
            }
        }
    }
}

void CmdTool::getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        // && words[0].contains("configuration") == false && words[0].contains("resources") == false
        // 将configuration的内容进行拆分
        if (words[0].contains("configuration") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split("=");
                if (attr.size() != 2) {
                    continue;
                }
                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        } else if (words[0].contains("resources") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split(":");
                if (attr.size() != 2) {
                    continue;
                }
                if (mapInfo.find(attr[0].trimmed()) != mapInfo.end()) {
                    mapInfo[attr[0].trimmed()] += QString("  ");
                }
                mapInfo[attr[0].trimmed()] += attr[1].trimmed();
            }
        } else {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        if (mapInfo.find(words[0].trimmed()) != mapInfo.end()) {
            mapInfo[words[0].trimmed()] += QString(" ");
        }
        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            QString key = words[0].trimmed();
            QString value = re.cap(1);

            //这里是为了防止  "usb-storage", "sr"  -》 usb-storage", "sr
            if (key == "Driver") {
                value.replace("\"", "");
            }
            mapInfo[key] += value;

        } else {
            if (words[0].trimmed() == "Resolution") {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
    }
}

void CmdTool::getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList lines = info.split("\n");
    QString lasKey;
    foreach (const QString &line, lines) {
        if (line.isEmpty()) {
            continue;
        }
        QStringList words = line.split(ch);
        if (words.size() == 1 && words[0].endsWith(":")) {
            lasKey = words[0].replace(QRegExp(":$"), "");
            mapInfo.insert(lasKey.trimmed(), " ");
        } else if (words.size() == 1 && !lasKey.isEmpty()) {
            mapInfo[lasKey.trimmed()] += words[0];
            mapInfo[lasKey.trimmed()] += "  /  ";
        } else if (words.size() == 2) {
            lasKey = "";
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QString indexName;
    int startIndex = 0;

    QRegExp reg("^[\\s\\S]*[\\d]:[\\d][\\s\\S]*$");//time 08:00

    for (int i = 0; i < info.size(); ++i) {
        if (info[i] != '\n' && i != info.size() - 1) {
            continue;
        }

        QString line = info.mid(startIndex, i - startIndex);
        startIndex = i + 1;


        int index = line.indexOf(ch);
        if (index > 0 && reg.exactMatch(line) == false && false == line.contains("Error") && false == line.contains("hh:mm:SS")) {
            if (line.indexOf("(") < index && line.indexOf(")") > index) {
                continue;
            }

            if (line.indexOf("[") < index && line.indexOf("]") > index) {
                continue;
            }

            indexName = line.mid(0, index).trimmed().remove(" is");
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.mid(index + 1).trimmed();
            } else {
                mapInfo[indexName] = line.mid(index + 1).trimmed();
            }
            continue;
        }

        if (indexName.isEmpty() == false && (line.startsWith("\t\t") || line.startsWith("    ")) && line.contains(":") == false) {
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.trimmed();
            } else {
                mapInfo[indexName] = line.trimmed();
            }

            continue;
        }

        indexName = "";

        QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
        if (rx.indexIn(line) > -1) {
            mapInfo[rx.cap(1)] = rx.cap(2);
            continue;
        }

        QStringList strList;

        if (line.endsWith(")")) {
            int leftBracket = line.indexOf("(");
            if (leftBracket > 0) {
                QString str = line.left(leftBracket).trimmed();
                strList = str.trimmed().split(" ");
                if (strList.size() > 2) {
                    strList.last() += line.mid(leftBracket);
                }
            }
        } else if (strList.size() == 0) {
            strList = line.trimmed().split(" ");
        }

        if (strList.size() < 5) {
            continue;
        }

        if (line.contains("Power_On_Hours")) {
            mapInfo["Power_On_Hours"] = strList.last();
            continue;
        }

        if (line.contains("Power_Cycle_Count")) {
            mapInfo["Power_Cycle_Count"] = strList.last();
            continue;
        }

        if (line.contains("Raw_Read_Error_Rate")) {
            mapInfo["Raw_Read_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Up_Time")) {
            mapInfo["Spin_Up_Time"] = strList.last();
            continue;
        }

        if (line.contains("Start_Stop_Count")) {
            mapInfo["Start_Stop_Count"] = strList.last();
            continue;
        }

        if (line.contains("Reallocated_Sector_Ct")) {
            mapInfo["Reallocated_Sector_Ct"] = strList.last();
            continue;
        }

        if (line.contains("Seek_Error_Rate")) {
            mapInfo["Seek_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Retry_Count")) {
            mapInfo["Spin_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("Calibration_Retry_Count")) {
            mapInfo["Calibration_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("G-Sense_Error_Rate")) {
            mapInfo["G-Sense_Error_Rate"] = strList.last();
            continue;
        }
        if (line.contains("Power-Off_Retract_Count")) {
            mapInfo["Power-Off_Retract_Count"] = strList.last();
            continue;
        }
        if (line.contains("Load_Cycle_Count")) {
            mapInfo["Load_Cycle_Count"] = strList.last();
            continue;
        }
        if (line.contains("Temperature_Celsius")) {
            mapInfo["Temperature_Celsius"] = strList.last();
            continue;
        }
        if (line.contains("Reallocated_Event_Count")) {
            mapInfo["Reallocated_Event_Count"] = strList.last();
            continue;
        }
        if (line.contains("Current_Pending_Sector")) {
            mapInfo["Current_Pending_Sector"] = strList.last();
            continue;
        }
        if (line.contains("Offline_Uncorrectable")) {
            mapInfo["Offline_Uncorrectable"] = strList.last();
            continue;
        }
        if (line.contains("UDMA_CRC_Error_Count")) {
            mapInfo["UDMA_CRC_Error_Count"] = strList.last();
            continue;
        }
        if (line.contains("Multi_Zone_Error_Rate")) {
            mapInfo["Multi_Zone_Error_Rate"] = strList.last();
            continue;
        }
    }
}

void CmdTool::getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList pairs = line.trimmed().split("  ");
        if (pairs.size() < 1) {
            continue;
        }
        foreach (const QString &pair, pairs) {
            QStringList keyValue = pair.trimmed().split(": ");
            if (keyValue.size() == 2) {
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
    }
}

void CmdTool::getMapInfoFromBluetoothCtl(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    QString uuid = "";
    foreach (const QString &line, lines) {
        QStringList keyValue = line.trimmed().split(": ");
        if (keyValue.size() == 2) {
            if (keyValue[0] == "UUID") {
                uuid.append(keyValue[1]);
                uuid.append("\n");
            } else {
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
    }
    if (uuid != "") {
        mapInfo["UUID"] = uuid;
    }
}

bool CmdTool::getDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    QString key = debugFile;
    key.replace(".txt", "");
    if (DBusInterface::getInstance()->getInfo(key, deviceInfo)) {
        return true;
    }

    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty()) {
        return true;
    }

    // 从文件中获取设备信息
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();

    return true;
}

bool CmdTool::getDeviceInfoFromCmd(QString &deviceInfo, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    deviceInfo = process.readAllStandardOutput();
    return true;
}

bool CmdTool::getCatDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty()) {
        return true;
    }

    // 从文件中获取设备信息
    QFile inputDeviceFile(debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();

    return true;
}
