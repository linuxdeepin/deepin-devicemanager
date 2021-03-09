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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${subdir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

# 引用ZeroMQ的库 begin ****************************************************************
link_libraries("zmq")
# 引用ZeroMQ的库 end  *****************************************************************

find_package(Qt5 COMPONENTS
    Core
    DBus REQUIRED)

file(GLOB_RECURSE DEVICE_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${DEVICE_SRC})

target_link_libraries(${APP_BIN_NAME} Qt5::Core Qt5::DBus)

# Install files
install(TARGETS ${APP_BIN_NAME} DESTINATION bin)
install(FILES ${SYSTEMD_SERVER} DESTINATION /lib/systemd/system)
install(FILES ${DBUS_CONFIG} DESTINATION /etc/dbus-1/system.d)

