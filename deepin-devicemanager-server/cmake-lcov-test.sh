# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Release .. 
make -j16

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

app_name=deepin-devicemanager-server-test

mkdir -p report

./test/deepin-devicemanager-server/$app_name --gtest_output=xml:./report/report.xml

lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/test/*' '*/3rdparty/*' -o  ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
