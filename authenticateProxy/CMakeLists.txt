cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

set(APP_BIN_NAME "deepin-devicemanager-authenticateProxy")
set(APP_POLICY_FILES "policy/com.deepin.pkexec.deepin-devicemanager-authenticateProxy.policy")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

project(${APP_BIN_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DFrameworkdbus REQUIRED)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# ${PROJECT_NAME}:项目名称，这里用作生成的exe名称
add_executable(${PROJECT_NAME} "main.cpp")

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

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
)

# Install files
install(FILES ${APP_POLICY_FILES} DESTINATION /usr/share/polkit-1/actions)
install(TARGETS deepin-devicemanager-authenticateProxy DESTINATION bin)

