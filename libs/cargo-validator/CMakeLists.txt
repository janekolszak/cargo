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
# @author Maciej Karpiuk (m.karpiuk2@samsung.com)
#

PROJECT(cargo-validator)

MESSAGE(STATUS "")
MESSAGE(STATUS "Generating makefile for the lib${PROJECT_NAME}...")

FILE(GLOB HEADERS           *.hpp)
FILE(GLOB HEADERS_INTERNALS internals/*.hpp)
FILE(GLOB SRCS              *.cpp)
FILE(GLOB SRCS_INTERNALS    internals/*.cpp)

SET(_LIB_VERSION_ "${VERSION}")
SET(_LIB_SOVERSION_ "0")
SET(PC_FILE "lib${PROJECT_NAME}.pc")

## Setup target ################################################################
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS} ${SRCS_INTERNALS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    SOVERSION   ${_LIB_SOVERSION_}
    VERSION     ${_LIB_VERSION_}
)

ADD_DEPENDENCIES(${PROJECT_NAME} cargo-utils)

## Link libraries ##############################################################
INCLUDE_DIRECTORIES(${LIBS_FOLDER} ${COMMON_FOLDER})
INCLUDE_DIRECTORIES(SYSTEM ${CARGO_DEPS_INCLUDE_DIRS})

## Generate the pc file ########################################################
CONFIGURE_FILE(${PC_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE} @ONLY)

## Install #####################################################################
INSTALL(FILES       ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE}
        COMPONENT   validator
        DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

INSTALL(TARGETS     ${PROJECT_NAME}
        COMPONENT   validator
        DESTINATION ${LIB_INSTALL_DIR})

INSTALL(FILES       ${HEADERS}
        COMPONENT   validator-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})

INSTALL(FILES       ${HEADERS_INTERNALS}
        COMPONENT   validator-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/internals)

