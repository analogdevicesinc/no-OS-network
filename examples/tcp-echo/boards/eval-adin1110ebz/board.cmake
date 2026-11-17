# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause

# The ADIN1110 10BASE-T1L MAC-PHY on SPI2, from the capi-adin1110 package.
# The SDK resolves packages to CAPI_<NAME>_DIR but does not add them.
add_subdirectory("${CAPI_ADIN1110_DIR}" "${CMAKE_BINARY_DIR}/capi-adin1110")
target_sources(tcp_echo PRIVATE "${CMAKE_CURRENT_LIST_DIR}/board.c")
target_link_libraries(tcp_echo PRIVATE capi_adin1110)
