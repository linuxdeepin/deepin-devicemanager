# deepin-devicemanager
Device Manager is a gadget for viewing device information.

深度设备管理器是查看、管理硬件设备的工具软件。

## Special Links
* [linux-hardware.org](https://linux-hardware.org/)

## Depend on 
* [dmidecode](http://www.nongnu.org/dmidecode/)
* [lshw](https://ezix.org/project/wiki/HardwareLiSter)
* [lspci](https://github.com/linuxhw/LsPCI)
* lsb_release
* hciconfig
* bluetoothctl
* [hwinfo](https://github.com/linuxhw/HWInfo)
* lsusb
* lscpu
* [smartctl](https://www.smartmontools.org/)
* [xrandr](https://www.x.org/wiki/Projects/XRandR/)
* [CUPS](https://www.cups.org/index.html)
* [upower](https://upower.freedesktop.org/)
* deepin-shortcut-viewer

## Thridparty lib
* [docx](https://github.com/lpxxn/docx)
* [QtXlsx](http://qtxlsx.debao.me)

## Installation
sudo apt-get install deepin-devicemanager

## Build
* mkdir build
* cd build
* qmake ..
* make

## Usage
Caution: deepin-devicemanager is depend on polkit, So if run source build version, 
you have to add policy to usr/share/polkit-1/actions/. 
add deepin-devicemanager-authenticateProxy to /usr/bin.

* sudo cp authenticateProxy/policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy /usr/share/polkit-1/actions/
* sudo cp build-all-unknown-Debug/authenticateProxy/deepin-devicemanager-authenticateProxy /usr/bin

* ./deepin-devicemanager