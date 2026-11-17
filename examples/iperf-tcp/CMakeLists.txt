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
project(iperf_tcp LANGUAGES C ASM)
capi_create_application(iperf_tcp SOURCES src/main.c)
target_include_directories(iperf_tcp PRIVATE src)

# The no-OS network Kconfig values; leave CONFIG_NO_OS_IP unset for DHCP.
set(CONFIG_NO_OS_IP "192.168.10.10")
set(CONFIG_NO_OS_NETMASK "255.255.255.0")
set(CONFIG_NO_OS_GATEWAY "192.168.10.1")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." "${CMAKE_BINARY_DIR}/no-os-network")
target_link_libraries(iperf_tcp PRIVATE no_os_network)

# Set IPERF_CLIENT_TO to an iperf2 server's address to also send it one test.
if(IPERF_CLIENT_TO)
    target_compile_definitions(iperf_tcp PRIVATE IPERF_CLIENT_TO="${IPERF_CLIENT_TO}")
endif()

include("${EXAMPLE_BOARD_DIR}/board.cmake")
