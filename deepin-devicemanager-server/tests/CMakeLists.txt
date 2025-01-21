# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 5.5.10.1)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option (PERF_ON "Use provided math implementation" ON)

# 设置包含头文件的时候不用包含路径 begin ****************************************************************************************
macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deepin-deviceinfo/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deepin-devicecontrol/src)
SUBDIRLIST(deviceinfo_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-deviceinfo/src)
SUBDIRLIST(devicecontrol_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-devicecontrol/src)
foreach(subdir ${deviceinfo_dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deepin-deviceinfo/src/${subdir})
endforeach()
foreach(subdir ${devicecontrol_dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deepin-devicecontrol/src/${subdir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 设置 QT_VERSION 变量
set(QT_VERSION_MAJOR 5)

# 定义一个宏来根据QT_VERSION选择性执行代码
macro(SET_QT_VERSION)
  if(${QT_VERSION_MAJOR} EQUAL 6)
    # Qt6 环境
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 COMPONENTS 
        Core
        Gui 
        Widgets 
        DBus 
        Sql 
        Network
        REQUIRED)

    find_package(Dtk6 COMPONENTS
        Widget
        Core
        REQUIRED
    )
    find_package(PolkitQt6-1 REQUIRED)
  elseif(${QT_VERSION_MAJOR} EQUAL 5)
    # Qt5 环境
    find_package(PkgConfig REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5 COMPONENTS DBus REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Sql REQUIRED)
    find_package(DtkWidget REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(DtkCore REQUIRED)
    find_package(PolkitQt5-1 REQUIRED)
    find_package(QApt REQUIRED)
  else()
    message(FATAL_ERROR "Unsupported QT_VERSION: ${QT_VERSION}")
  endif()
endmacro()

# 调用宏
SET_QT_VERSION()

if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(QApt-qt6 REQUIRED)
    include_directories(${QApt-qt6_INCLUDE_DIRS})
    set(QAPT_LIB QApt-qt6)
elseif(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(QApt REQUIRED)
    include_directories(${QApt_INCLUDE_DIRS})
    set(QAPT_LIB QApt)
else()
    message(FATAL_ERROR "Unsupported QT_VERSION: ${QT_VERSION_MAJOR}")
endif()

PKG_SEARCH_MODULE(kmod REQUIRED libkmod IMPORTED_TARGET)

set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

#src
file(GLOB_RECURSE INFO_SRCS
     ${CMAKE_CURRENT_LIST_DIR}/../deepin-deviceinfo/src/*.cpp
    )
file(GLOB_RECURSE CONTROL_SRCS
     ${CMAKE_CURRENT_LIST_DIR}/../deepin-devicecontrol/src/*.cpp
    )
# remove src main.cpp or will multi define
list(REMOVE_ITEM INFO_SRCS ${CMAKE_CURRENT_LIST_DIR}/../deepin-deviceinfo/src/plugin.cpp)
list(REMOVE_ITEM CONTROL_SRCS ${CMAKE_CURRENT_LIST_DIR}/../deepin-devicecontrol/src/main.cpp)

#test src
file(GLOB_RECURSE TEST_SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE TEST_SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

link_libraries("udev")

add_executable(${PROJECT_NAME_TEST} ${INFO_SRCS} ${CONTROL_SRCS} ${TEST_SRC_CPP} ${TEST_SRC_H})

include_directories("/usr/include/cups/")

target_include_directories(${PROJECT_NAME_TEST} PUBLIC 
    ${DtkWidget_INCLUDE_DIRS}
    ${OBJECT_BINARY_DIR}
    )

if(${QT_VERSION_MAJOR} EQUAL 6)
    # Qt6 environment
    target_link_libraries(${PROJECT_NAME_TEST}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Sql
    Qt6::Network
    Dtk6::Core
    Dtk6::Widget
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    PolkitQt6-1::Agent
    pthread
    kmod
    ${QAPT_LIB}
    cups
    )
elseif(${QT_VERSION_MAJOR} EQUAL 5)
    # Qt5 environment
    target_link_libraries(${PROJECT_NAME_TEST}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Sql
    Qt5::Network
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    PolkitQt5-1::Agent
    pthread
    kmod
    ${QAPT_LIB}
    cups
    )
else()
    message(FATAL_ERROR "Unsupported QT_VERSION: ${QT_VERSION_MAJOR}")
endif()

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()
