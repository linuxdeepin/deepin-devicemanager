cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 5.5.10.1)
endif ()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
#common resource names
set(APP_RES_DIR "${CMAKE_SOURCE_DIR}/src/deepin-devicemanager/assets")
set(APP_BIN_NAME_TEST "deepin-devicemanager-test")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-devicemanager.desktop")
set(APP_QRC "${APP_RES_DIR}/appimage.qrc")
set(APP_ICON_FILE "${APP_RES_DIR}/icons/deepin/builtin/deepin-devicemanager.svg")

project(${APP_BIN_NAME_TEST})

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage")

#Generate the qm file
execute_process(COMMAND bash ${APP_TRANS_SCRIPT}
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

configure_file(${CMAKE_SOURCE_DIR}/src/deepin-devicemanager/src/environments.h.in environments.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/src/deepin-devicemanager/src/config.h.in config.h @ONLY)
add_definitions(-DUSE_POLKIT -DENABLE_INACTIVE_DISPLAY)

# Find the library

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DFrameworkdbus REQUIRED)
set(LINK_LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-devicemanager/src/)

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-devicemanager/src/)

#Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-devicemanager/src/${subdir})
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-devicemanager/3rdparty/QtXlsxWriter/include/QtXlsx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/deepin-devicemanager/3rdparty/docx)

#add cups
include_directories("/usr/include/cups/")
link_libraries("cups")
link_libraries("zmq")

file(GLOB_RECURSE APP_SRCS
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/3rdparty/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/application.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/DebugTimeManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/DBus/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/DeviceManager/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/LoadInfo/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/Page/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/ThreadPool/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/Widget/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager/src/ZeroMQ/*.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager-server/src/DebugTimeManager.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager-server/src/MainJob.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager-server/src/DBus/*.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager-server/src/LoadInfo/*.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/../src/deepin-devicemanager-server/src/ZeroMQ/*.cpp
    )
FILE(GLOB allTestSource
  src/deepin-devicemanager/*.cpp
  src/deepin-devicemanager/DBus/*.cpp
  src/deepin-devicemanager/DeviceManager/*.cpp
  src/deepin-devicemanager/LoadInfo/*.cpp
  src/deepin-devicemanager/Page/*.cpp
  src/deepin-devicemanager/ThreadPool/*.cpp
  src/deepin-devicemanager/Widget/*.cpp
  src/deepin-devicemanager/ZeroMQ/*.cpp
#  src/deepin-devicemanager-server/*.cpp
#  src/deepin-devicemanager-server/ZeroMQ/*.cpp
#  src/deepin-devicemanager-server/LoadInfo/*.cpp
#  src/deepin-devicemanager-server/DBus/*.cpp
#  src/deepin-devicemanager-server/MainJob.cpp
)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the executable
add_executable(${APP_BIN_NAME_TEST} ${APP_SRCS} ${APP_QRC} ${allTestSource}  ${TEST_ACCESSIBLE_HEADERS_FILES})

target_include_directories(${APP_BIN_NAME_TEST}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME_TEST}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Xml
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

set(CMAKE_INSTALL_PREFIX /usr)

add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/tests/${APP_BIN_NAME_TEST}
    COMMAND echo " =================== TEST END ==================== "
)
