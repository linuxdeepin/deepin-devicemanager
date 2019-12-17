#! /bin/bash

#pkexec /bin/bash -c "apt-get install lshw"

pkexec deepin-devicemanager-authenticateProxy "sudo apt-get install smartmontools"
pkexec deepin-devicemanager-authenticateProxy "sudo apt-get install dmidecode"
pkexec deepin-devicemanager-authenticateProxy "sudo apt-get install hwinfo"
pkexec deepin-devicemanager-authenticateProxy "sudo apt-get install upower"

chmod +x smartctl

mkdir deviceInfo

cat /proc/version > deviceInfo/osinfo.txt
pkexec deepin-devicemanager-authenticateProxy dmidecode > deviceInfo/dmidecode.txt

pkexec deepin-devicemanager-authenticateProxy lshw > deviceInfo/lshw.txt

pkexec deepin-devicemanager-authenticateProxy lscpu > deviceInfo/lscpu.txt
cat /proc/cpuinfo > deviceInfo/catcpu.txt

pkexec deepin-devicemanager-authenticateProxy smartctl --all /dev/sda > deviceInfo/smartctl.txt
#pkexec /bin/bash -c "sudo ./smartctl --all /dev/sda" > deviceInfo/smartctl.txt

cat /proc/bus/input/devices > deviceInfo/input.txt
pkexec deepin-devicemanager-authenticateProxy "xrandr --verbose" > deviceInfo/xrandr.txt
pkexec deepin-devicemanager-authenticateProxy "lspci -v" > deviceInfo/lspci.txt
pkexec deepin-devicemanager-authenticateProxy "hciconfig -a" > deviceInfo/hciconfig.txt
pkexec deepin-devicemanager-authenticateProxy "hwinfo --monitor" > deviceInfo/hwinfo.txt

cat /etc/os-release > deviceInfo/os-release.txt
lsb_release -a > deviceInfo/lsb_release.txt

upower --dump > deviceInfo/upower.txt

tar zcvf deviceInfo.tar.gz deviceInfo