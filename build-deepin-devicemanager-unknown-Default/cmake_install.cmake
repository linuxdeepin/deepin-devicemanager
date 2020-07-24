# Install script for directory: /data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/build-deepin-devicemanager-unknown-Default/deepin-devicemanager")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/deepin-devicemanager")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/deepin-devicemanager/translations" TYPE FILE FILES
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_ca.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_cs.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_es.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_fi.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_fr.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_it.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_ko.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_nl.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_pt.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_pt_BR.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_sr.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_tr.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_uk.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_zh_CN.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_zh_HK.qm"
    "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/translations/deepin-devicemanager_zh_TW.qm"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" TYPE FILE FILES "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/assets/deepin-devicemanager.desktop")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/icons/hicolor/scalable/apps/deepin-devicemanager.svg")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/icons/hicolor/scalable/apps" TYPE FILE FILES "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/deepin-devicemanager/assets/icons/deepin/builtin/deepin-devicemanager.svg")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/data/home/liujun/Work/Project/DeviceManager/sp2/dde_devicemanager/build-deepin-devicemanager-unknown-Default/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
