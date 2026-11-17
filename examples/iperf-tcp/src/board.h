/*
 * Copyright (c) 2026 Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>
#include "capi_eth_mac.h"

/**
 * Set up what the board's Ethernet MAC is wired to and fill in the
 * parameters lwip_socket's netdev passes to capi_eth_mac_init(), and the
 * MAC address the MAC is given there.
 *
 * @return 0, or a negative error code.
 */
int board_eth_mac_config(struct capi_eth_mac_init_config *cfg, uint8_t hwaddr[6]);

#endif
