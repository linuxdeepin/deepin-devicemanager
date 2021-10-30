# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#
# Author:     V4fr3e <V4fr3e@deepin.io>
#
# Maintainer: V4fr3e <liujinli@uniontech.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 5.5.10.1)
endif ()


#common resource names

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-need -fPIE  -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

option (PERF_ON "Use provided math implementation" ON)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")

#Generate the qm file
#message(${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND bash ${APP_TRANS_SCRIPT} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
configure_file(../src/environments.h.in environments.h @ONLY)
configure_file(../src/config.h.in config.h @ONLY)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DFrameworkdbus REQUIRED)


#add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager/tests/)
# Test--------deepin-devicemanager
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

ADD_COMPILE_OPTIONS(-fno-access-control)

#安全测试的开关
set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")

if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")

#Include all third
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QtXlsxWriter/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/docx/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

#add cups
include_directories("/usr/include/cups/")
link_libraries("cups")

#src
file(GLOB_RECURSE SRC_CPP
     ${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp
     ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/*.cpp
    )
# remove src main.cpp or will multi define
list(REMOVE_ITEM SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/../src/main.cpp)

file(GLOB_RECURSE TEST_SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE TEST_SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
add_executable(${PROJECT_NAME_TEST} ${SRC_CPP} ${TEST_SRC_CPP} ${TEST_SRC_H})

target_include_directories(${PROJECT_NAME_TEST}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME_TEST}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
    Qt5::Test
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/deepin-devicemanager/tests/${PROJECT_NAME_TEST}
    COMMAND echo " =================== TEST END ==================== "
)
#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")

if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
endif()
