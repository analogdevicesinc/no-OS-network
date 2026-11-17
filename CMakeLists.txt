# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause
#
# The no-OS network code on CAPI: the socket API (socket/), its lwIP
# backend with the Ethernet MAC netdev (lwip_socket/) and lwIP (lwip/), as
# the no_os_network library. An application adds this directory and links
# no_os_network.
#
# CAPI_DIR is the CAPI checkout, set by the CAPI SDK or the caller. The no-OS
# network Kconfig values are CMake variables here: set CONFIG_NO_OS_IP,
# CONFIG_NO_OS_NETMASK and CONFIG_NO_OS_GATEWAY for a static address (DHCP
# otherwise) and CONFIG_NO_OS_DOMAIN_NAME for the mDNS host name.

cmake_minimum_required(VERSION 3.21)

if(NOT EXISTS "${CAPI_DIR}/inc/capi_eth_mac.h")
    message(FATAL_ERROR "CAPI_DIR must point at a CAPI checkout (got '${CAPI_DIR}')")
endif()

add_subdirectory(lwip)

add_library(no_os_network STATIC
    socket/tcp_socket.c
    lwip_socket/lwip_socket.c
    lwip_socket/netdevs/eth_mac/lwip_eth_mac.c
    util/src/no_os_util.c)
target_include_directories(no_os_network PUBLIC
    socket
    lwip_socket
    lwip_socket/netdevs/eth_mac
    util/include
    "${CAPI_DIR}/inc")
target_compile_definitions(no_os_network PUBLIC
    NO_OS_LWIP_NETWORKING=1)
foreach(_cfg IN ITEMS CONFIG_NO_OS_IP CONFIG_NO_OS_NETMASK CONFIG_NO_OS_GATEWAY
                      CONFIG_NO_OS_DOMAIN_NAME)
    if(DEFINED ${_cfg})
        target_compile_definitions(no_os_network PRIVATE ${_cfg}="${${_cfg}}")
    endif()
endforeach()
target_link_libraries(no_os_network PUBLIC lwipallapps lwipcore)
