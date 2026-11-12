project(deepin-devicemanager-server C CXX)

add_subdirectory(deepin-deviceinfo)
add_subdirectory(deepin-devicecontrol)
add_subdirectory(customgpuinfo)

#TEST--------------------------------------------------
# 注：单元测试已外移至顶层 test/deepin-devicemanager-server/，由根 CMakeLists 统一挂载
