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
# @author Mateusz Malicki (m.malicki2@samsung.com)
#

PROJECT(cargo-ipc)

MESSAGE(STATUS "")
MESSAGE(STATUS "Generating makefile for the lib${PROJECT_NAME}...")

FILE(GLOB HEADERS           *.hpp
                            ${COMMON_FOLDER}/config.hpp)
FILE(GLOB HEADERS_INTERNALS internals/*.hpp)
FILE(GLOB HEADERS_EPOLL     epoll/*.hpp)
FILE(GLOB HEADERS_UTILS     ${COMMON_FOLDER}/utils/eventfd.hpp
                            ${COMMON_FOLDER}/utils/callback-guard.hpp)

FILE(GLOB SRCS              *.cpp)
FILE(GLOB SRCS_INTERNALS    internals/*.cpp)
FILE(GLOB SRCS_EPOLL        epoll/*.cpp)

SET(_LIB_VERSION_ "${VERSION}")
SET(_LIB_SOVERSION_ "0")
SET(PC_FILE "lib${PROJECT_NAME}.pc")

## Setup target ################################################################
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS} ${SRCS_INTERNALS} ${SRCS_EPOLL} ${SRCS_UTILS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    SOVERSION   ${_LIB_SOVERSION_}
    VERSION     ${_LIB_VERSION_}
    )

ADD_DEPENDENCIES(${PROJECT_NAME} cargo-utils cargo-logger cargo-fd)

## Link libraries ##############################################################
IF(WITHOUT_SYSTEMD)
PKG_CHECK_MODULES(CARGO_IPC_DEPS REQUIRED uuid gio-2.0)
ELSE()
PKG_CHECK_MODULES(CARGO_IPC_DEPS REQUIRED uuid gio-2.0 libsystemd-daemon)
ENDIF(WITHOUT_SYSTEMD)

INCLUDE_DIRECTORIES(${LIBS_FOLDER} ${COMMON_FOLDER})
INCLUDE_DIRECTORIES(SYSTEM ${CARGO_IPC_DEPS_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} cargo-utils ${CARGO_IPC_DEPS_LIBRARIES} cargo-logger cargo-fd)

## Generate the pc file ########################################################
CONFIGURE_FILE(${PC_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE} @ONLY)

## Install #####################################################################
INSTALL(FILES       ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE}
        COMPONENT   ipc
        DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

INSTALL(TARGETS     ${PROJECT_NAME}
        DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT   ipc)

INSTALL(FILES       ${HEADERS}
        COMPONENT   ipc-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})

INSTALL(FILES       ${HEADERS_INTERNALS}
        COMPONENT   ipc-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/internals)

INSTALL(FILES       ${HEADERS_EPOLL}
        COMPONENT   ipc-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/epoll)

INSTALL(FILES       ${HEADERS_UTILS}
        COMPONENT   ipc-dev
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/utils)
