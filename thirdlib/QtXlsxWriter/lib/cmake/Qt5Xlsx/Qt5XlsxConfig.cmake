
if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "Qt 5 Xlsx module requires at least CMake version 3.1.0")
endif()

get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
# Use original install prefix when loaded through a
# cross-prefix symbolic link such as /lib -> /usr/lib.
get_filename_component(_realCurr "${_IMPORT_PREFIX}" REALPATH)
get_filename_component(_realOrig "/usr/lib/x86_64-linux-gnu/cmake/Qt5Xlsx" REALPATH)
if(_realCurr STREQUAL _realOrig)
    get_filename_component(_qt5Xlsx_install_prefix "/usr/lib/x86_64-linux-gnu/../../" ABSOLUTE)
else()
    get_filename_component(_qt5Xlsx_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()
unset(_realOrig)
unset(_realCurr)
unset(_IMPORT_PREFIX)

# For backwards compatibility only. Use Qt5Xlsx_VERSION instead.
set(Qt5Xlsx_VERSION_STRING 0.3.0)

set(Qt5Xlsx_LIBRARIES Qt5::Xlsx)

macro(_qt5_Xlsx_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::Xlsx\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    endif()
endmacro()

macro(_populate_Xlsx_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION)
    set_property(TARGET Qt5::Xlsx APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt5Xlsx_install_prefix}/lib/x86_64-linux-gnu/${LIB_LOCATION}")
    _qt5_Xlsx_check_file_exists(${imported_location})
    set_target_properties(Qt5::Xlsx PROPERTIES
        "INTERFACE_LINK_LIBRARIES" "${_Qt5Xlsx_LIB_DEPENDENCIES}"
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        "IMPORTED_SONAME_${Configuration}" "libQt5Xlsx.so.0"
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_Qt5Xlsx_LIB_DEPENDENCIES}"
    )

endmacro()

if (NOT TARGET Qt5::Xlsx)

    set(_Qt5Xlsx_OWN_INCLUDE_DIRS "${_qt5Xlsx_install_prefix}/include/x86_64-linux-gnu/qt5/" "${_qt5Xlsx_install_prefix}/include/x86_64-linux-gnu/qt5/QtXlsx")
    set(Qt5Xlsx_PRIVATE_INCLUDE_DIRS
        "${_qt5Xlsx_install_prefix}/include/x86_64-linux-gnu/qt5/QtXlsx/0.3.0"
        "${_qt5Xlsx_install_prefix}/include/x86_64-linux-gnu/qt5/QtXlsx/0.3.0/QtXlsx"
    )

    foreach(_dir ${_Qt5Xlsx_OWN_INCLUDE_DIRS})
        _qt5_Xlsx_check_file_exists(${_dir})
    endforeach()

    # Only check existence of private includes if the Private component is
    # specified.
    list(FIND Qt5Xlsx_FIND_COMPONENTS Private _check_private)
    if (NOT _check_private STREQUAL -1)
        foreach(_dir ${Qt5Xlsx_PRIVATE_INCLUDE_DIRS})
            _qt5_Xlsx_check_file_exists(${_dir})
        endforeach()
    endif()

    set(Qt5Xlsx_INCLUDE_DIRS ${_Qt5Xlsx_OWN_INCLUDE_DIRS})

    set(Qt5Xlsx_DEFINITIONS -DQT_XLSX_LIB)
    set(Qt5Xlsx_COMPILE_DEFINITIONS QT_XLSX_LIB)
    set(_Qt5Xlsx_MODULE_DEPENDENCIES "Gui;Core")


    set(Qt5Xlsx_OWN_PRIVATE_INCLUDE_DIRS ${Qt5Xlsx_PRIVATE_INCLUDE_DIRS})

    set(_Qt5Xlsx_FIND_DEPENDENCIES_REQUIRED)
    if (Qt5Xlsx_FIND_REQUIRED)
        set(_Qt5Xlsx_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt5Xlsx_FIND_DEPENDENCIES_QUIET)
    if (Qt5Xlsx_FIND_QUIETLY)
        set(_Qt5Xlsx_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt5Xlsx_FIND_VERSION_EXACT)
    if (Qt5Xlsx_FIND_VERSION_EXACT)
        set(_Qt5Xlsx_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt5Xlsx_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt5Xlsx_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                0.3.0 ${_Qt5Xlsx_FIND_VERSION_EXACT}
                ${_Qt5Xlsx_DEPENDENCIES_FIND_QUIET}
                ${_Qt5Xlsx_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt5Xlsx_FOUND False)
            return()
        endif()

        list(APPEND Qt5Xlsx_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt5Xlsx_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5Xlsx_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt5Xlsx_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt5Xlsx_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt5Xlsx_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5Xlsx_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5Xlsx_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5Xlsx_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5Xlsx_EXECUTABLE_COMPILE_FLAGS)

    set(_Qt5Xlsx_LIB_DEPENDENCIES "Qt5::Gui;Qt5::Core")


    add_library(Qt5::Xlsx SHARED IMPORTED)

    set_property(TARGET Qt5::Xlsx PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt5Xlsx_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::Xlsx PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_XLSX_LIB)

    set(_Qt5Xlsx_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt5Xlsx_PRIVATE_DIR ${Qt5Xlsx_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt5Xlsx_PRIVATE_DIR})
            set(_Qt5Xlsx_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt5Xlsx_PRIVATE_DIRS_EXIST)
        add_library(Qt5::XlsxPrivate INTERFACE IMPORTED)
        set_property(TARGET Qt5::XlsxPrivate PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt5Xlsx_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt5Xlsx_PRIVATEDEPS)
        foreach(dep ${_Qt5Xlsx_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt5Xlsx_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::XlsxPrivate PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::Xlsx ${_Qt5Xlsx_PRIVATEDEPS}
        )
    endif()

    _populate_Xlsx_target_properties(RELEASE "libQt5Xlsx.so.0.3.0" "" )




    file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5Xlsx_*Plugin.cmake")

    macro(_populate_Xlsx_plugin_properties Plugin Configuration PLUGIN_LOCATION)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt5Xlsx_install_prefix}/lib/x86_64-linux-gnu/qt5/plugins/${PLUGIN_LOCATION}")
        _qt5_Xlsx_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )
    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()




_qt5_Xlsx_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt5XlsxConfigVersion.cmake")

endif()
