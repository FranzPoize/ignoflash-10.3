cmake_minimum_required(VERSION 2.6)

# IgnoFlash
project(IgnoFlash)

# Add a _d to debug binaries
set(CMAKE_DEBUG_POSTFIX "_d")

# Specify build paths
set(IgnoFlash_OUTPUT_DIR "${IgnoFlash_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IgnoFlash_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IgnoFlash_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IgnoFlash_OUTPUT_DIR}")

# Find packages
find_package(Qt4 4.6.2 COMPONENTS QtCore QtGui QtXml QtMain REQUIRED)
include(${QT_USE_FILE})

include_directories(
        ${QT_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
        
SET(UI_HEADER_FILES
    dialog.h
    )
QT4_WRAP_CPP(UI_MOC_FILES ${UI_HEADER_FILES})

SET(UI_FILES
    dialog.ui
    )
QT4_WRAP_UI(UI_HEADER_FILES ${UI_FILES})

set(HEADER_FILES
    dialog.h
    detect.h
    backup.h
    patch.h
    activex.h
    netscape_win32.h
    netscape_linux32.h
    utils.h
    )

set(SOURCE_FILES
    main.cpp
    dialog.cpp
    detect.cpp
    backup.cpp
    patch.cpp
    activex.cpp
    netscape_win32.cpp
    netscape_linux32.cpp
    utils.cpp
    )

set(MANIFEST_FILES
	ignoflash.manifest
	)	

if(WIN32)
	add_executable(IgnoFlash WIN32 ${HEADER_FILES} ${UI_HEADER_FILES} ${SOURCE_FILES} ${UI_MOC_FILES} ${MANIFEST_FILES})
	add_definitions(-D_UNICODE)
	set_target_properties(IgnoFlash PROPERTIES COMPILE_FLAGS "/Zc:wchar_t-")
	set_target_properties(IgnoFlash PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='highestAvailable' uiAccess='false'\" /SUBSYSTEM:WINDOWS")
else()
  add_executable(IgnoFlash ${HEADER_FILES} ${UI_HEADER_FILES} ${SOURCE_FILES} ${UI_MOC_FILES})
endif()

if(WIN32)
  target_link_libraries(IgnoFlash ${QT_LIBRARIES} version.lib)
else()
  target_link_libraries(IgnoFlash ${QT_LIBRARIES})
endif()
