# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.27)

if(NOT DEFINED CAPI_DIR)
    if(NOT DEFINED ENV{CAPI_DIR})
        message(FATAL_ERROR
            "CAPI_DIR not set. Export CAPI_DIR=/path/to/capi or pass -DCAPI_DIR=...")
    endif()
    set(CAPI_DIR "$ENV{CAPI_DIR}")
endif()

# Which MAC the board has and how it is wired is in boards/<BOARD>/.
set(EXAMPLE_BOARD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}")
if(NOT EXISTS "${EXAMPLE_BOARD_DIR}/board.cmake")
    message(FATAL_ERROR "No Ethernet setup for board '${BOARD}' in boards/")
endif()
if(EXISTS "${EXAMPLE_BOARD_DIR}/board.conf")
    list(APPEND CAPI_EXTRA_CONF_FILE "${EXAMPLE_BOARD_DIR}/board.conf")
endif()

include("${CAPI_DIR}/sdk/cmake/capi_create_application.cmake")

capi_resolve_platform()
project(tcp_echo LANGUAGES C ASM)
capi_create_application(tcp_echo SOURCES src/main.c)
target_include_directories(tcp_echo PRIVATE src)

# The no-OS network Kconfig values; leave CONFIG_NO_OS_IP unset for DHCP.
set(CONFIG_NO_OS_IP "192.168.10.10")
set(CONFIG_NO_OS_NETMASK "255.255.255.0")
set(CONFIG_NO_OS_GATEWAY "192.168.10.1")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." "${CMAKE_BINARY_DIR}/no-os-network")
target_link_libraries(tcp_echo PRIVATE no_os_network)

include("${EXAMPLE_BOARD_DIR}/board.cmake")
