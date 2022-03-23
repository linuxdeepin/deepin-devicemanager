cmake_minimum_required(VERSION 3.5)

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

# 引用ZeroMQ的库 begin ****************************************************************
link_libraries("zmq")
link_libraries("udev")
# 引用ZeroMQ的库 end  *****************************************************************

find_package(Qt5 COMPONENTS
    Core
    DBus REQUIRED)

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H})

target_link_libraries(${APP_BIN_NAME} Qt5::Core Qt5::DBus)

# Install files
install(TARGETS ${APP_BIN_NAME} DESTINATION bin)
install(FILES ${SYSTEMD_SERVER} DESTINATION /lib/systemd/system)
install(FILES ${DBUS_CONFIG} DESTINATION /etc/dbus-1/system.d)

#TEST--------------------------------------------------
add_subdirectory(./tests)
