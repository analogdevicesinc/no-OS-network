# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause
#
# lwIP for bare-metal CAPI applications: lwIP's lwipcore and lwipallapps
# targets, built against the port and configuration in this directory. A
# target that links lwipcore gets the lwIP headers and this configuration.

cmake_minimum_required(VERSION 3.21)

# lwIP at the commit no-OS selects (CONFIG_LWIP_VERSION in no-OS
# libraries/Kconfig).
include(FetchContent)
FetchContent_Declare(lwip
    GIT_REPOSITORY https://github.com/lwip-tcpip/lwip
    GIT_TAG e29870c15e8bf28eac9c811dd236c474f3f2008f
    # Only the sources are needed; do not add lwIP's own CMake project.
    SOURCE_SUBDIR do-not-add)
FetchContent_MakeAvailable(lwip)

# As no-OS cmake/libraries/lwip.cmake. This directory comes first:
# contrib/examples/example_app has an lwipopts.h too.
set(LWIP_DIR "${lwip_SOURCE_DIR}")
set(LWIP_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/configs"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib"
    "${LWIP_DIR}/contrib/examples/example_app")
include("${LWIP_DIR}/src/Filelists.cmake")
target_include_directories(lwipcore PUBLIC ${LWIP_INCLUDE_DIRS})
