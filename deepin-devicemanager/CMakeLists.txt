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
set(APP_RES_DIR "assets")
set(APP_BIN_NAME "deepin-devicemanager")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-devicemanager.desktop")
set(APP_QRC "${APP_RES_DIR}/appimage.qrc")
set(APP_ICON_FILE "${APP_RES_DIR}/icons/deepin/builtin/deepin-devicemanager.svg")

project(${APP_BIN_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
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

#Generate the qm file
#message(${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND bash ${APP_TRANS_SCRIPT} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
configure_file(src/environments.h.in environments.h @ONLY)
configure_file(src/config.h.in config.h @ONLY)

macro(SUBDIRLIST result curdir)
#    message(************************************************SUBDIRLIST:${SUBDIRLIST})
#    message(************************************************result:${result})
#    message(************************************************curdir:${curdir})
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)

#Include all src
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()

#Include all third
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QtXlsxWriter/include/QtXlsx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/docx)
#add cups
include_directories("/usr/include/cups/")
#link_directories("/usr/lib/cups/")
link_libraries("cups")
link_libraries("zmq")

file(GLOB_RECURSE DEVICE_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/3rdparty/*.cpp)


# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DFrameworkdbus REQUIRED)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${DEVICE_SRC} ${THIRD_PARTY} ${APP_QRC})
target_include_directories(${APP_BIN_NAME}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}

    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
)

set(CMAKE_INSTALL_PREFIX /usr)


# Install files
install(TARGETS deepin-devicemanager DESTINATION bin)

file(GLOB APP_QM_FILES "translations/*.qm")
install(FILES ${APP_QM_FILES} DESTINATION share/deepin-devicemanager/translations)
install(FILES ${APP_DESKTOP} DESTINATION share/applications)
install(FILES ${APP_ICON_FILE} DESTINATION /usr/share/icons/hicolor/scalable/apps)
install(DIRECTORY ${APP_RES_DIR}/deepin-devicemanager DESTINATION /usr/share/deepin-manual/manual-assets/application/)
