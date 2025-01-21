project(deepin-devicemanager-server C CXX)

add_subdirectory(deepin-deviceinfo)
add_subdirectory(deepin-devicecontrol)

set(QT_VERSION_MAJOR 5)

#TEST--------------------------------------------------
if (CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
   add_subdirectory(./tests)
endif()
