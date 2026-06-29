#************************************************************************************************
#
# This file is part of Crystal Class Library (R)
# Copyright (c) 2025 CCL Software Licensing GmbH.
# All Rights Reserved.
#
# Licensed for use under either:
#  1. a Commercial License provided by CCL Software Licensing GmbH, or
#  2. GNU Affero General Public License v3.0 (AGPLv3).
# 
# You must choose and comply with one of the above licensing options.
# For more information, please visit ccl.dev.
#
# Filename    : CMakeLists.txt
# Description : CCL Demo
#
#************************************************************************************************

cmake_minimum_required (VERSION 3.30 FATAL_ERROR)

project ("CCL Demo")
set (VENDOR_PRODUCT_NAME "CCL Demo")
set (PROJECT_VENDOR ccl)

# By default, CCL Demo is running in a sandbox. This can be disabled in order to use CCL Spy, 
# which doesn't support sandboxing due to required file system access.
option (ccldemo_ENABLE_SANDBOX "CCL Demo will be running in a sandbox" ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND ${ccldemo_ENABLE_SANDBOX})
	set (CCLDEMO_CCLSPY_SUPPORT OFF)
else ()
	set (CCLDEMO_CCLSPY_SUPPORT ON)
endif ()

# Find dependencies
include (vendor OPTIONAL)
find_package (ccl ${CCL_VERSION_REQUIREMENT} REQUIRED COMPONENTS cclapp cclbase ccltext cclsystem cclnet cclgui cclsecurity cclextras-stores)
find_package (modelimporter3d)
find_package (skins)
if (${CCLDEMO_CCLSPY_SUPPORT})
	find_package (cclspy)
endif ()

list (APPEND ccldemo_license_files
	${CCL_IDENTITIES_DIR}/ccl/legal/CCL\ 3rd\ Party\ Licenses.txt
	${CCL_IDENTITIES_DIR}/ccl/eula/EULA.txt
)

# Add targets
ccl_add_app (ccldemo GUI
	VENDOR ccl
	VERSION_FILE "${CMAKE_CURRENT_LIST_DIR}/../source/appversion.h"
	VERSION_PREFIX APP
)
ccl_set_product_name (ccldemo ${ccldemo_NAME})
ccl_set_startup_project (ccldemo)
  
ccl_use_skin_packages (ccldemo neutralbase neutraldesign devtools)
ccl_add_skin_package (ccldemo ${CMAKE_CURRENT_LIST_DIR}/../skin)

ccl_add_assets (ccldemo FILES ${ccldemo_license_files} PATH "license")

list (APPEND ccldemo_resources
	${CMAKE_CURRENT_LIST_DIR}/../resource/cclgui.config
	${CMAKE_CURRENT_LIST_DIR}/../resource/cclplugins.xml
	${CMAKE_CURRENT_LIST_DIR}/../resource/commands.xml
	${CMAKE_CURRENT_LIST_DIR}/../resource/markdowndemo.md
	${CMAKE_CURRENT_LIST_DIR}/../resource/embedded
	${CMAKE_CURRENT_LIST_DIR}/../resource/svg
	${CMAKE_CURRENT_LIST_DIR}/../resource/menubar.xml
	${CMAKE_CURRENT_LIST_DIR}/../resource/appmenu.xml
)

list (APPEND ccldemo_source_files
	${CMAKE_CURRENT_LIST_DIR}/../source/appversion.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demoapp.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demoapp.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demoitem.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demoitem.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/doclinks.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/blockviewdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/buttondemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/coreviewdemo.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/compositiondemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/controlsdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/dialogsdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/embeddeddemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/exampletext.h
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/flexboxdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/focusdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/graphicsdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/graphicsdemo3d.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/layoutdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/networkdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/sliderdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/spritedemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/skindemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/storedemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/svgdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/systemdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/touchdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/treeviewdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/viewsdemo.cpp
	${CMAKE_CURRENT_LIST_DIR}/../source/demos/workspacedemo.cpp
)

list (APPEND ccldemo_ccl_sources
	${CCL_DIR}/main/cclmain.cpp
)

source_group (TREE ${CMAKE_CURRENT_LIST_DIR}/../source PREFIX "source" FILES ${ccldemo_source_files})
source_group ("source\\ccl" FILES ${ccldemo_ccl_sources})

list (APPEND ccldemo_sources
	${ccldemo_source_files}
	${ccldemo_ccl_sources}
)

ccl_add_resources (ccldemo ${ccldemo_resources})
target_sources (ccldemo PRIVATE ${ccldemo_sources})
target_link_libraries (ccldemo PRIVATE cclapp cclbase ccltext cclsystem cclnet cclgui cclsecurity corelib cclextras-stores)

cmake_path (SET applications_root NORMALIZE "${CCL_APPLICATIONS_ROOT}")
cmake_path (SET repo_root NORMALIZE "${REPOSITORY_ROOT}")
cmake_path (RELATIVE_PATH applications_root BASE_DIRECTORY "${repo_root}" OUTPUT_VARIABLE ccl_applications_dir)
target_compile_definitions (ccldemo PRIVATE "CCL_APPLICATIONS_DIRECTORY=\"${ccl_applications_dir}/\"")

ccl_add_dependencies (ccldemo modelimporter3d)

if (${CCLDEMO_CCLSPY_SUPPORT})
	ccl_add_dependencies (ccldemo cclspy)
endif ()
