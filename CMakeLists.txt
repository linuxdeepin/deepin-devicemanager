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

# Auto-detect Qt version (tries Qt6 first, falls back to Qt5)
# 注: cmake 3.31+ 的 find_package(NAMES) 在部分环境会误检为 Qt5,故改为显式探测
find_package(Qt6 QUIET COMPONENTS Core)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core)
    set(QT_VERSION_MAJOR 5)
endif()
message(STATUS "Found Qt version: ${QT_VERSION_MAJOR}")

# Map to DTK version (Qt6→DTK6, Qt5→DTK5)
if (QT_VERSION_MAJOR MATCHES 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()
message(STATUS "Build with DTK: ${DTK_VERSION_MAJOR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,relro -Wl,-z,now")

add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager)
add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager-server)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()
