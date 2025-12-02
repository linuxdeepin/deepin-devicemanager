cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "customgpuinfo")

macro(SET_QT_VERSION)
  if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt6 COMPONENTS Core REQUIRED)
  elseif(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(Qt5 COMPONENTS Core REQUIRED)
  else()
    message(FATAL_ERROR "Unsupported QT_VERSION_MAJOR: ${QT_VERSION_MAJOR}")
  endif()
endmacro()

SET_QT_VERSION()

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
