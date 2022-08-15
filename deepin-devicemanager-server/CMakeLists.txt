cmake_minimum_required(VERSION 3.7)

set(APP_BIN_NAME "deepin-devicemanager-server")
set(SYSTEMD_SERVER "${CMAKE_CURRENT_SOURCE_DIR}/deepin-devicemanager-server.service")
set(DBUS_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.devicemanager.conf")
project(${APP_BIN_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

# Find the library
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(QApt REQUIRED)
find_package(PolkitQt5-1 REQUIRED)
find_package(DFrameworkdbus REQUIRED)

PKG_SEARCH_MODULE(kmod REQUIRED libkmod IMPORTED_TARGET)

# 设置包含头文件的时候不用包含路径 begin ****************************************************************************************
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

#定义社区版宏begin
message("begin")
execute_process(COMMAND cat "/etc/os-version"
                OUTPUT_VARIABLE output_var)

if(output_var MATCHES "Community")
    option(COMMUNITY "OS community" ON)
    if(COMMUNITY)
        add_definitions(-DCOMMUNITY)
    endif()
endif()

message("end")
#定义社区版宏end

# 引用ZeroMQ的库 begin ****************************************************************
include_directories("/usr/include/cups/")
link_libraries("cups")
link_libraries("udev")
# 引用ZeroMQ的库 end  *****************************************************************

find_package(Qt5 COMPONENTS
    Core
    DBus
    Sql 
    Network REQUIRED)
find_package(DtkCore REQUIRED)

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H})

target_link_libraries(${APP_BIN_NAME}
    ${DtkCore_LIBRARIES}
    ${DFrameworkdbus_LIBRARIES}
    Qt5::Core Qt5::DBus Qt5::Sql Qt5::Network PolkitQt5-1::Agent kmod QApt)

# Install files
install(TARGETS ${APP_BIN_NAME} DESTINATION bin)
install(FILES ${SYSTEMD_SERVER} DESTINATION /lib/systemd/system)
install(FILES ${DBUS_CONFIG} DESTINATION /etc/dbus-1/system.d)

#TEST--------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_subdirectory(./tests)
endif()
