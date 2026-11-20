# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause

# The ADIN1110 10BASE-T1L MAC-PHY on SPI2, from the no-os-adin package.
# The SDK resolves packages to <NAME>_DIR but does not add them.
# More OA TC6 RX frame buffers than no-OS's 5: while the netdev waits for
# room to send, the frames the ADIN1110 receives wait in them, and with 5
# a full-duplex TCP load leaves RX at 2-3 Mb/s.
set(CONFIG_OA_RX_FRAME_BUFF_NUM 16 CACHE STRING "OA TC6 RX frame buffers")
add_subdirectory("${NO_OS_ADIN_DIR}" "${CMAKE_BINARY_DIR}/no-os-adin")
target_sources(iperf_tcp PRIVATE "${CMAKE_CURRENT_LIST_DIR}/board.c")
target_link_libraries(iperf_tcp PRIVATE no_os_adin)
