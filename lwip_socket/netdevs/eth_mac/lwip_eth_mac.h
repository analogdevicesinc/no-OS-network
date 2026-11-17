/***************************************************************************//**
 *   @file   lwip_eth_mac.h
 *   @brief  Header file for the lwIP netdev on a CAPI Ethernet MAC.
 *   @author Ciprian Regus (ciprian.regus@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef LWIP_ETH_MAC_H_
#define LWIP_ETH_MAC_H_

#ifdef NO_OS_LWIP_NETWORKING

#define ETH_MAC_LWIP_BUFF_SIZE 2000

/* How long a frame to send waits for a MAC whose TX queue is full */
#define ETH_MAC_LWIP_TX_TIMEOUT_MS 20

extern const struct no_os_lwip_ops eth_mac_lwip_ops;

#endif /* NO_OS_LWIP_NETWORKING */
#endif /* LWIP_ETH_MAC_H_ */
