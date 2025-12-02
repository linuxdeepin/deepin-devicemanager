cmake_minimum_required(VERSION 3.7)
project(deepin-devicemanager C CXX)

include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(DISABLE_DRIVER false CACHE BOOL "disable driver")
if (DISABLE_DRIVER)
    add_definitions(-DDISABLE_DRIVER)
endif()

set(DISABLE_POLKIT false CACHE BOOL "disable polkit debug")
if(CMAKE_BUILD_TYPE  STREQUAL "Debug")
    add_definitions(-DDISABLE_POLKIT)
endif()

# 自动检测系统中的Qt版本
if(NOT DEFINED QT_VERSION_MAJOR)
    # 首先尝试查找Qt6
    find_package(Qt6 QUIET)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        message("Found Qt6, using Qt version 6")
    else()
        set(QT_VERSION_MAJOR 5)
        message("Found Qt5, using Qt version 5")
    endif()
else()
    message("Using manually specified QT_VERSION_MAJOR: ${QT_VERSION_MAJOR}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,relro -Wl,-z,now")

add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager)
add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager-server)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()
