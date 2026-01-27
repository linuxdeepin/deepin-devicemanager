cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "customgpuinfo")

# Find Qt package with detected version
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${BIN_NAME}
    ${SRC}
)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/deepin-devicemanager)
