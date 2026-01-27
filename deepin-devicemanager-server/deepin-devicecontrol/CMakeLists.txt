cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "deepin-devicecontrol")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTIFY_SOURCE=2 -z noexecstack -pie -fPIC -z lazy")

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../deepin-devicemanager/src/DDLog)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
  
# Find Qt package with detected version
find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Network Sql REQUIRED)

# Find Dtk package with mapped version
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS
    Widget
    Core
    REQUIRED
)

# Define external dependency package names based on Qt version
if(${QT_VERSION_MAJOR} EQUAL 6)
    set(POLKITQT_NAME "PolkitQt6-1")
    set(QAPT_NAME "QApt-qt6")
elseif(${QT_VERSION_MAJOR} EQUAL 5)
    set(POLKITQT_NAME "PolkitQt5-1")
    set(QAPT_NAME "QApt")
else()
    message(FATAL_ERROR "Unsupported QT_VERSION_MAJOR: ${QT_VERSION_MAJOR}")
endif()

# Find external dependencies
find_package(${POLKITQT_NAME} REQUIRED)
find_package(deepin-qdbus-service REQUIRED)
find_package(${QAPT_NAME} REQUIRED)
include_directories(${${QAPT_NAME}_INCLUDE_DIRS})

if(NOT DISABLE_DRIVER)
    PKG_SEARCH_MODULE(kmod REQUIRED libkmod IMPORTED_TARGET)
endif()

add_executable(${BIN_NAME}
    ${SRC_CPP}
    ${SRC_H}
)

include_directories("/usr/include/cups/")

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")

# Include directories with dynamic Qt/DTK versions
if(${QT_VERSION_MAJOR} EQUAL 6)
    target_include_directories(${BIN_NAME} PUBLIC
        Dtk${DTK_VERSION_MAJOR}::Core
        Dtk${DTK_VERSION_MAJOR}::Widget
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Gui
        ${deepin-qdbus-service_INCLUDE_DIR}
        ${QAPT_NAME}
    )

    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        ${POLKITQT_NAME}::Agent
        Dtk${DTK_VERSION_MAJOR}::Core
        Dtk${DTK_VERSION_MAJOR}::Widget
        ${deepin-qdbus-service_LIBRARIES}
        ${QAPT_NAME}
        cups
    )
elseif(${QT_VERSION_MAJOR} EQUAL 5)
    target_include_directories(${BIN_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        ${deepin-qdbus-service_INCLUDE_DIR}
    )

    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        ${POLKITQT_NAME}::Agent
        ${DtkCore_LIBRARIES}
        ${deepin-qdbus-service_LIBRARIES}
        cups
    )
endif()
    
if(NOT DISABLE_DRIVER)
    target_link_libraries(${BIN_NAME} PRIVATE
        kmod
        ${QAPT_NAME}
        )
endif()

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${BIN_NAME}.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.devicecontrol.conf DESTINATION share/dbus-1/system.d/)
install(FILES org.deepin.DeviceControl.service DESTINATION share/dbus-1/system-services/)
install(FILES deepin-devicecontrol.service DESTINATION lib/systemd/system/)
# 添加安全加固策略
install(DIRECTORY deepin-service-group@.service.d DESTINATION /etc/systemd/system/)
