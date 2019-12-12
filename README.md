## deepin-devicemanager
Device Manager is a handy tool for viewing hardware information and managing the devices.

设备管理器是查看、管理硬件设备的工具软件。

#### Core
The primary GUI is based on DTK, Qt (Supported >= 5.12).

Build-Depends: debhelper (>= 11), pkg-config, qt5-qmake, qt5-default, libdtkwidget-dev, libdtkgui-dev, qtbase5-private-dev, libdframeworkdbus-dev, libcups2-dev

Execute-Depends: smartmontools, dmidecode[i386 arm64 amd64], x11-xserver-utils, hwinfo, cups, upower, deepin-shortcut-viewer, lshw

#### Special Link
- [linux-hardware.org](https://linux-hardware.org/)

#### Depend on 
- [dmidecode](http://www.nongnu.org/dmidecode/)
- [lshw](https://ezix.org/project/wiki/HardwareLiSter)
- [lspci](https://github.com/linuxhw/LsPCI)
- lsb_release
- hciconfig
- bluetoothctl
- [hwinfo](https://github.com/linuxhw/HWInfo)
- lsusb
- lscpu
- [smartctl](https://www.smartmontools.org/)
- [xrandr](https://www.x.org/wiki/Projects/XRandR/)
- [CUPS](https://www.cups.org/index.html)
- [upower](https://upower.freedesktop.org/)
- deepin-shortcut-viewer

#### Thridparty lib (source code)
- [docx](https://github.com/lpxxn/docx)
- [QtXlsx](http://qtxlsx.debao.me)

#### Installation
sudo apt-get install deepin-devicemanager

#### Build
- mkdir build
- cd build
- qmake ..
- make

#### Usage
Caution: deepin-devicemanager is depend on polkit, So if run source build version, 
you have to add policy to usr/share/polkit-1/actions/. 
add deepin-devicemanager-authenticateProxy to /usr/bin.
If use Qt Creator, you should set [Project][Run Settings] [Run] [Run configuration] to src from 2 projects authenticateProxy and src.
Of course, you have to install build depends first.

- sudo cp authenticateProxy/policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy /usr/share/polkit-1/actions/
- sudo cp build-all-unknown-Debug/authenticateProxy/deepin-devicemanager-authenticateProxy /usr/bin

- ./deepin-devicemanager