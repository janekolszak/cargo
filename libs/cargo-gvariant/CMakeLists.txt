# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
# @file   CMakeLists.txt
# @author Pawel Kubik (p.kubik@samsung.com)
#

PROJECT(cargo-gvariant)

MESSAGE(STATUS "")
MESSAGE(STATUS "Generating makefile for the lib${PROJECT_NAME}...")

FILE(GLOB HEADERS           *.hpp
                            ${COMMON_FOLDER}/config.hpp)
FILE(GLOB HEADERS_INTERNALS internals/*.hpp)

SET(_LIB_VERSION_ "${VERSION}")
SET(_LIB_SOVERSION_ "0")
SET(PC_FILE "lib${PROJECT_NAME}.pc")

## Setup target ################################################################

## Link libraries ##############################################################
PKG_CHECK_MODULES(CARGO_GVARIANT_DEPS REQUIRED glib-2.0)

INCLUDE_DIRECTORIES(${LIBS_FOLDER})
INCLUDE_DIRECTORIES(SYSTEM ${CARGO_GVARIANT_DEPS_INCLUDE_DIRS})

## Generate the pc file ########################################################
CONFIGURE_FILE(${PC_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE} @ONLY)

## Install #####################################################################
INSTALL(FILES       ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE}
        COMPONENT   gvariant-dev
        DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

INSTALL(DIRECTORY . DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
        COMPONENT   gvariant-dev
        FILES_MATCHING PATTERN "*.hpp"
                       PATTERN "CMakeFiles" EXCLUDE)

INSTALL(FILES       ${COMMON_FOLDER}/config.hpp
        COMPONENT   gvariant-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})
