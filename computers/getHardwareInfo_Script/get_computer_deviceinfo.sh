#! /bin/bash

#pkexec /bin/bash -c "apt-get install lshw"

pkexec /bin/bash -c "apt-get install smartmontools"
pkexec /bin/bash -c "apt-get install dmidecode"
pkexec /bin/bash -c "apt-get install hwinfo"
pkexec /bin/bash -c "apt-get install upower"

chmod +x smartctl

mkdir deviceInfo

cat /proc/version > deviceInfo/osinfo.txt
pkexec /bin/bash -c dmidecode > deviceInfo/dmidecode.txt

pkexec /bin/bash -c lshw > deviceInfo/lshw.txt

pkexec /bin/bash -c lscpu > deviceInfo/lscpu.txt
cat /proc/cpuinfo > deviceInfo/catcpu.txt

pkexec /bin/bash -c "sudo smartctl --all /dev/sda" > deviceInfo/smartctl.txt
#pkexec /bin/bash -c "sudo ./smartctl --all /dev/sda" > deviceInfo/smartctl.txt

cat /proc/bus/input/devices > deviceInfo/input.txt
pkexec /bin/bash -c "sudo xrandr --verbose" > deviceInfo/xrandr.txt
pkexec /bin/bash -c "sudo lspci -v" > deviceInfo/lspci.txt
pkexec /bin/bash -c "sudo hciconfig -a" > deviceInfo/hciconfig.txt
pkexec /bin/bash -c "sudo hwinfo --monitor" > deviceInfo/hwinfo.txt

cat /etc/os-release > deviceInfo/os-release.txt
lsb_release -a > deviceInfo/lsb_release.txt

upower --dump > deviceInfo/upower.txt

tar zcvf deviceInfo.tar.gz deviceInfo