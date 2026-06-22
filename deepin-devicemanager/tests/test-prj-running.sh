#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

SCRIPT_DIR=$(cd $(dirname $0); pwd)
builddir=build
reportdir=build-ut

PROJECT_ROOT=$(cd $SCRIPT_DIR/../..; pwd)
BUILD_DIR=$PROJECT_ROOT/$builddir
REPORT_DIR=$PROJECT_ROOT/$reportdir

rm -rf $BUILD_DIR
rm -rf $REPORT_DIR
mkdir -p $BUILD_DIR
mkdir -p $REPORT_DIR
cd $BUILD_DIR

#编译 (Qt6 + 覆盖率 + 单元测试)
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_COVERAGE_ARG="CMAKE_COVERAGE_ARG_ON" \
      -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" \
      $PROJECT_ROOT
make -k -j$(nproc) || true

mkdir -p report

# 生成asan日志和ut测试xml结果
if [ -f ./deepin-devicemanager/tests/deepin-devicemanager-test ]; then
    echo " =================== CLIENT TEST BEGIN ==================== "
    QT_QPA_PLATFORM=offscreen ./deepin-devicemanager/tests/deepin-devicemanager-test --gtest_output=xml:./report/report_deepin-devicemanager.xml
    echo " =================== CLIENT TEST END ==================== "
else
    echo "WARNING: deepin-devicemanager-test not found, skipping client tests"
fi

# server测试如果存在则运行
if [ -f ./deepin-devicemanager-server/tests/deepin-devicemanager-server-test ]; then
    echo " =================== SERVER TEST BEGIN ==================== "
    QT_QPA_PLATFORM=offscreen ./deepin-devicemanager-server/tests/deepin-devicemanager-server-test --gtest_output=xml:./report/report_deepin-devicemanager-server.xml
    echo " =================== SERVER TEST END ==================== "
else
    echo "WARNING: deepin-devicemanager-server-test not found, skipping server tests"
fi

#统计代码覆盖率并生成html报告
lcov -d $BUILD_DIR -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o ./coverage.info

# 排除第三方 vendored 代码（QtXlsxWriter/docx），其路径中的 /src/ 会被上面的
# extract 模式误匹配，导致项目自身覆盖率被拉低
lcov --remove ./coverage.info '*/3rdparty/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-devicemanager.html 2>/dev/null || true
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html $REPORT_DIR/ 2>/dev/null || true
cp -r report $REPORT_DIR/ 2>/dev/null || true
cp -r asan*.log* $REPORT_DIR/asan_deepin-devicemanager.log 2>/dev/null || true

exit 0
