cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "deepin-deviceinfo")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../deepin-devicemanager/src/DDLog)
SUBDIRLIST(dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach(dir ${dirs})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endforeach()
# 设置包含头文件的时候不用包含路径 end ****************************************************************************************

file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_LIST_DIR}/src/*.h)

link_libraries("udev")

#qt5
# find_package(Qt5 COMPONENTS Core DBus REQUIRED)
# find_package(DtkCore REQUIRED)
# find_package(PolkitQt5-1 REQUIRED)

#qt6
find_package(Qt6 COMPONENTS Core DBus REQUIRED)
find_package(PolkitQt6-1 REQUIRED)

find_package(Dtk6 COMPONENTS
    Widget
    Core
    REQUIRED
)

if (Qt6_FOUND)
    find_package(QApt-qt6 REQUIRED)
    include_directories(${QApt-qt6_INCLUDE_DIRS})
    set(QAPT_LIB QApt-qt6)
else()
    find_package(QApt REQUIRED)
    include_directories(${QApt_INCLUDE_DIRS})
    set(QAPT_LIB QApt)
endif()

add_library(${BIN_NAME} MODULE
    ${SRC_CPP}
    ${SRC_H}
)

if (Qt6_FOUND)
    # Qt6 environment
    target_include_directories(${BIN_NAME} PUBLIC
    Qt6::Core
    Qt6::DBus
    ${QAPT_LIB}
  )
  
  target_link_libraries(${BIN_NAME} PRIVATE
    Dtk6::Core
    Dtk6::Widget
    Qt6::Core
    Qt6::DBus
    PolkitQt6-1::Agent
    ${Dtk6Core_LIBRARIES}
    ${QAPT_LIB}
  )
else()
    # Qt5 environment
    target_include_directories(${BIN_NAME} PUBLIC
    Qt5::Core
    Qt5::DBus
  )
  
  target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core
    Qt5::DBus
    PolkitQt5-1::Agent
    ${DtkCore_LIBRARIES}
  )
endif()


install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
install(FILES ${BIN_NAME}.json DESTINATION share/deepin-service-manager/system/)
install(FILES org.deepin.deviceinfo.conf DESTINATION share/dbus-1/system.d/ )
