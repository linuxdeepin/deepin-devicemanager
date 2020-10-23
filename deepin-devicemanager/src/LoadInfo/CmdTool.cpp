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

CmdTool::CmdTool()
{

}

void CmdTool::addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    // 命令 与 设备信息Map 的 对应关系Map
    if (m_cmdInfo.find(key) != m_cmdInfo.end()) {
        // 该命令已有获取的设备
        m_cmdInfo[key].append(mapInfo);
    } else {
        // 该命令还未插入Map
        QList<QMap<QString, QString> > lstMap;
        lstMap.append(mapInfo);
        m_cmdInfo.insert(key, lstMap);
    }

}

void CmdTool::getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src)
{
    // 打印机名称
    mapInfo.insert("Name", src->name);

    // 打印机的其他属性
    for (int i = 0; i < src->num_options; i++) {
        mapInfo.insert(src->options[i].name, src->options[i].value);
    }
}

void CmdTool::loadCmdInfo(const QString &key, const QString &cmd, const QString &debugFile)
{
    // 根据命令加载设备信息
    if (key == "lshw") {
        loadLshwInfo(cmd, debugFile);
    } else if (key == "lsblk_d") {
        loadLsblkInfo(cmd, debugFile);
    } else if (key == "xrandr") {
        loadXrandrInfo(cmd, debugFile);
    } else if (key == "xrandr_verbose") {
        loadXrandrVerboseInfo(cmd, debugFile);
    } else if (key == "dmesg") {
        loadDmesgInfo(cmd, debugFile);
    } else if (key == "hciconfig") {
        loadHciconfigInfo(cmd, debugFile);
    } else if (key == "printer") {
        loadPrinterInfo();
    } else if (key == "upower") {
        loadUpowerInfo(key, cmd, debugFile);
    } else if (key.startsWith("hwinfo")) {
        loadHwinfoInfo(key, cmd, debugFile);
    } else if (key.startsWith("dmidecode")) {
        loadDmidecodeInfo(key, cmd, debugFile);
    } else if (key == "cat_devices") {
        loadCatInputDeviceInfo(key, cmd, debugFile);
    }  else if (key == "gpuinfo") {
        loadGpuInfo(key, cmd, debugFile);
    }  else if (key == "cat_audio") {
        loadCatAudioInfo(key, cmd, debugFile);
    }   else if (key == "EDID_HDMI" || key == "EDID_VGA") {
        loadEdidInfo(key, cmd, debugFile);
    }  else if (key == "bootdevice") {
        loadBootDeviceManfid(key, cmd, debugFile);
    } else if (key == "bt_device") {
        loadBluetoothPairedDevices(key, cmd, debugFile);     // 加载蓝牙设备配对信息
    } else {
        loadCatInfo(key, cmd, debugFile);
    }

}

QMap<QString, QList<QMap<QString, QString> > > &CmdTool::cmdInfo()
{
    return m_cmdInfo;
}

void CmdTool::loadLshwInfo(const QString &cmd, const QString &debugFile)
{
    // 加载lshw信息
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugFile);
    QStringList items = deviceInfo.split("*-");
    bool isFirst = true;
    foreach (const QString &item, items) {
        QMap<QString, QString> mapInfo;
        if (isFirst) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_system", mapInfo);           // 系统信息
            isFirst = false;
            continue;
        }
        if (item.startsWith("cpu")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cpu", mapInfo);              // cpu信息
        } else if (item.startsWith("disk")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_disk", mapInfo);             // 存储设备信息
        } else if (item.startsWith("storage")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_storage", mapInfo);          // 存储设备信息
        } else if ((item.startsWith("memory") && !item.startsWith("memory UNCLAIMED")) || item.startsWith("bank")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_memory", mapInfo);           // 内存信息
        } else if (item.startsWith("display")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_display", mapInfo);          // GPU信息
        } else if (item.startsWith("multimedia")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_multimedia", mapInfo);       // 音频适配器信息
        } else if (item.startsWith("network")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_network", mapInfo);          // 网络适配器信息
        } else if (item.startsWith("usb")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_usb", mapInfo);              // USB设备信息
        } else if (item.startsWith("cdrom")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cdrom", mapInfo);            // CD-ROM信息
        }
    }
}

void CmdTool::loadLsblkInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }

    QStringList lines = deviceInfo.split("\n");
    QMap<QString, QString> mapInfo;

    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        // NAME ROTA
        if (words.size() != 2 || words[0] == "NAME") {
            continue;
        }

        // sda     0
        mapInfo.insert(words[0].trimmed(), words[1].trimmed());

        //通过smartctl获取存储设备信息
        QString smartCmd = QString("sudo smartctl --all /dev/%1").arg(words[0].trimmed());
        loadSmartCtlInfo(smartCmd, words[0].trimmed(), "smartctl_" + words[0].trimmed() + ".txt");
    }
    addMapInfo("lsblk_d", mapInfo);
}

void CmdTool::loadSmartCtlInfo(const QString &cmd, const QString &logicalName, const QString &debugfile)
{
    //通过smartctl获取存储设备信息
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
    QMap<QString, QString> mapInfo;
    mapInfo["ln"] = logicalName;         // 逻辑名称
    getMapInfoFromSmartctl(mapInfo, deviceInfo);
    addMapInfo("smart", mapInfo);
}

void CmdTool::loadXrandrInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        // 屏幕支持分辨率
        QRegExp reResolution("^[\\s]{3}([0-9]{3,5}x[0-9]{3,5}).*([0-9]{2,3}.[0-9]{2,3}\\*).*");
        if (reResolution.exactMatch(line)) {
            QString rate = reResolution.cap(2).replace("*", "");
            mapInfo.insert("rate", rate);
        }

        // 当前GPU参数
        if (line.startsWith("Screen")) {
            QRegExp re(".*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*");
            if (re.exactMatch(line)) {
                mapInfo["minResolution"] = re.cap(1);         // GPU支持最小分辨率
                mapInfo["curResolution"] = re.cap(2);         // GPU当前分辨率
                mapInfo["maxResolution"] = re.cap(3);         // GPU支持最大分辨率
            }
        } else if (line.startsWith("HDMI")) {            // 计算机支持的屏幕接入方式
            mapInfo["HDMI"] = "Enable";
        } else if (line.startsWith("VGA")) {
            mapInfo["VGA"] = "Enable";
        } else if (line.startsWith("DP") || line.startsWith("DisplayPort"))  {
            mapInfo["DP"] = "Enable";
        } else if (line.startsWith("eDP")) {
            mapInfo["eDP"] = "Enable";
        } else if (line.startsWith("DVI")) {
            mapInfo["DVI"] = "Enable";
        }

    }
    addMapInfo("xrandr", mapInfo);
}

void CmdTool::loadXrandrVerboseInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(QString(cmd), deviceInfo, debugfile)) {
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
        QRegExp reMain("^[a-zA-Z].*");
        if (reMain.exactMatch(line)) {
            // 主屏幕信息
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

        // edid 信息
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

void CmdTool::loadDmesgInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }
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
    foreach (const QString &line, lines) {
        QRegExp reg(".*autoconfig for ([A-Za-z0-9]+):.*");
        if (reg.exactMatch(line)) {
            QString chip = reg.cap(1);
            mapInfo["chip"] = chip;
        }
    }
    addMapInfo("audiochip", mapInfo);
}

void CmdTool::loadHciconfigInfo(const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
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
    if (!getDeviceInfo("bluetoothctl show " + mapInfo["BD Address"], deviceInfo, "bluetoothctl.txt")) {
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
    if (!getDeviceInfo("lpstat -a", deviceInfo, "lpstat.txt")) {
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

void CmdTool::loadHwinfoInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugfile);
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

void CmdTool::loadDmidecodeInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    if (key == "dmidecode2") {
        loadDmidecode2Info(key, cmd, debugfile);
        return;
    }
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugfile);

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
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadDmidecode2Info(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugfile);
    QStringList items = deviceInfo.split("\n\n");
    QMap<QString, QString> mapInfo;
    bool isGetInfo = false;
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        getMapInfoFromDmidecode(item, mapInfo);
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

void CmdTool::loadCatInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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
            getMapInfoFromCmd(item, mapInfo, key.startsWith("cat_os") ? "=" : ": ");
        }
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadUpowerInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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
            addMapInfo("Daemon", mapInfo);
        } else {
            addMapInfo(key, mapInfo);
        }
    }
}

void CmdTool::loadBiosInfoFromLspci(QString &chipsetFamliy)
{
    // 获取设备信息
    QString command;
    QString deviceInfo;
    if (!getDeviceInfo(QString("lspci"), deviceInfo, "lspci.txt")) {
        return;
    }
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.split(" ");
        if (words.size() < 2) {
            continue;
        }
        if (words[1] == QString("ISA")) {
            command = QString("lspci -v -s %1").arg(words[0].trimmed());
            break;
        }
    }

    if (command.isEmpty()) {
        return;
    }
    QString chipset;
    if (!getDeviceInfo(command, chipset, "")) {
        return;
    }
    lines = chipset.split("\n");
    foreach (const QString &line, lines) {
        if (line.contains("Subsystem")) {
            QStringList words = line.split(": ");
            if (words.size() == 2) {
                chipsetFamliy = words[1].trimmed();
            }
            break;
        }
    }

}

void CmdTool::loadCatInputDeviceInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty()) {
            continue;
        }

        QMap<QString, QString> mapInfo;
        getMapInfoFromInput(item, mapInfo, "=");

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

        //addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatAudioInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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

void CmdTool::loadEdidInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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
        EDIDParser edidParser;
        QString errorMsg;
        edidParser.setEdid(edid, errorMsg);

        QMap<QString, QString> mapInfo;
        mapInfo.insert("Vendor", edidParser.vendor());
        mapInfo.insert("Date", edidParser.releaseDate());
        mapInfo.insert("Size", edidParser.screenSize());
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadGpuInfo(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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

void CmdTool::loadBootDeviceManfid(const QString &key, const QString &cmd, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
        return;
    }

    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", deviceInfo.trimmed());
    addMapInfo(key, mapInfo);
}

void CmdTool::loadBluetoothPairedDevices(const QString &key, const QString &cmd, const QString &debugfile)
{
    // 多线程中不能保证hciconfig在该命令之前执行完成，所以不能以命令判断是否执行下一步
    // 当机器上没有蓝牙适配器时,不查找匹配信息
//    if (m_cmdInfo.find("hciconfig") == m_cmdInfo.end()) {
//        return;
//    }

//    if (m_cmdInfo["hciconfig"].empty()) {
//        return;
//    }

    QString deviceInfo;
    if (!getDeviceInfo(cmd, deviceInfo, debugfile)) {
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

bool CmdTool::getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile)
{
    if (!deviceInfo.isEmpty()) {
        return true;
    }
    if (false == executeProcess(command, deviceInfo)) {
        return false;
    }
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    return true;
}

bool CmdTool::executeProcess(const QString &cmd, QString &deviceInfo)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#else
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd, deviceInfo);
    }

    runCmd("id -un", deviceInfo);
    if (deviceInfo.trimmed() == "root") {
        return runCmd(cmd, deviceInfo);
    }

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd, deviceInfo);
#endif
}

bool CmdTool::runCmd(const QString &proxy, QString &deviceInfo)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyy:MM:dd:hh:mm:ss");
    QString dtInt = QString::number(dt.toMSecsSinceEpoch());
    QString key = getPKStr(dtStr, dtInt);

//    用于测试
//    QString str1, str2;
//    getPKStr(str1, str2, key);

    QString cmd = proxy;
    QProcess process_;
    int msecs = 10000;

    // bluetoothctl 命令阻塞时间
    if (cmd.contains("bluetooth")) {
        msecs = 500;
    }
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy")) {
        cmd = proxy + QString(" ") + key;
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    deviceInfo = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126)) {
        return false;
    }

    if (res == false) {
        //dError("Run \'" + cmd + "\' failed\n");
    }

    return res;
}

QString CmdTool::getPKStr(const QString &dtStr, const QString &dtInt)
{
    QString res = "";
    QString str = dtStr;
    str.replace(":", "");

    int year = str.mid(0, 4).toInt() - 253;
    int month = str.mid(4, 2).toInt() * 7;
    int day = str.mid(6, 2).toInt() * 3;
    int hour = str.mid(8, 2).toInt() * 4;
    int minus = str.mid(10, 2).toInt();
    int second = str.mid(12, 2).toInt();

    QString yearStr = QString("%1").arg(year, 4, 10, QLatin1Char('0'));
    QString monthStr = QString("%1").arg(month, 2, 10, QLatin1Char('0'));
    QString dayStr = QString("%1").arg(day, 2, 10, QLatin1Char('0'));
    QString hourStr = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString minusStr = QString("%1").arg(minus, 2, 10, QLatin1Char('0'));
    QString secondStr = QString("%1").arg(second, 2, 10, QLatin1Char('0'));

    str = dtInt;
    QString value1 = str.mid(0, 1);
    QString value2 = str.mid(1, 2);
    QString value3 = str.mid(3, 3);
    QString value4 = str.mid(6, 4);
    QString value5 = str.mid(10);

    QTime time = QTime::currentTime();
    qsrand(uint(time.msec()) + uint(time.second()) * 1000);
    int random = (qrand() % 10000 + 10000) * 3;  //产生随机数
    QString randomS = QString::number(random);

    QString newDtStr = QString("%1%2%3%4%5%6%7%8%9%10%11%12").arg(value4).arg(dayStr).arg(value2).arg(secondStr).arg(value1).arg(hourStr).arg(value3).arg(monthStr).arg(yearStr).arg(minusStr).arg(value5).arg(randomS);

    return newDtStr;
}

void CmdTool::getPKStr(QString &dtStr, QString &dtInt, const QString &cStr)
{
    QString value4 = cStr.mid(0, 4);
    QString dayStr = QString("%1").arg(cStr.mid(4, 2).toInt() / 3, 2, 10, QLatin1Char('0'));
    QString value2 = cStr.mid(6, 2);
    QString secondStr = QString("%1").arg(cStr.mid(8, 2).toInt(), 2, 10, QLatin1Char('0'));
    QString value1 = cStr.mid(10, 1);
    QString hourStr = QString("%1").arg(cStr.mid(11, 2).toInt() / 4, 2, 10, QLatin1Char('0'));
    QString value3 = cStr.mid(13, 3);
    QString monthStr = QString("%1").arg(cStr.mid(16, 2).toInt() / 7, 2, 10, QLatin1Char('0'));
    QString yearStr = QString("%1").arg(cStr.mid(18, 4).toInt() + 253, 4, 10, QLatin1Char('0'));
    QString minuStr = cStr.mid(22, 2);

    QString value5 = cStr.mid(24);
    QString extraS = value5.right(5);
    value5.replace(extraS, "");

    int extraInt = extraS.toInt();
    if (extraInt % 3 != 0) {
        dtStr = "111";
        return;
    }

    dtStr = QString("%1:%2:%3:%4:%5:%6").arg(yearStr, monthStr, dayStr, hourStr, minuStr, secondStr);
    dtInt = QString("%1%2%3%4%5").arg(value1).arg(value2).arg(value3).arg(value4).arg(value5);
}
