
/***************************************************************************//**
 *   @file   lwip_eth_mac.c
 *   @brief  Source file for the lwIP netdev on a CAPI Ethernet MAC.
 *   @author Ciprian Regus (ciprian.regus@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifdef NO_OS_LWIP_NETWORKING

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpbase.h"
#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/api.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "lwip/inet.h"
#include "lwip_socket.h"
#include "capi_time.h"
#include "tcp_socket.h"

#include "capi_eth_mac.h"
#include "lwip_eth_mac.h"

static uint8_t lwip_buff[ETH_MAC_LWIP_BUFF_SIZE];

/**
 * @brief Read a frame from the RX FIFO.
 * @param desc - Ethernet MAC handle.
 * @param p - the received pbuf.
 * @param len - length of the frame.
 * @return 0 in case of success, negative error otherwise.
 */
static int eth_mac_read_frames(struct capi_eth_mac_handle *desc, struct pbuf **p,
				uint32_t *len)
{
	struct capi_eth_frame_buffer mac_buff;
	uint32_t size;
	int ret;

	ret = capi_eth_mac_get_rx_frame_size(desc, &size);
	if (ret)
		return ret;

	*len = 0;
	if (!size)
		return 0;

	ret = capi_eth_fb_init(&mac_buff, lwip_buff, sizeof(lwip_buff), 0);
	if (ret)
		return ret;

	ret = capi_eth_mac_read_frame(desc, &mac_buff);
	if (ret < 0)
		return ret;

	*len = mac_buff.data_length;
	if (!mac_buff.data_length)
		return 0;

	*p = pbuf_alloc(PBUF_RAW, mac_buff.data_length, PBUF_POOL);
	if (!*p)
		return -ENOMEM;

	pbuf_take(*p, lwip_buff, mac_buff.data_length);

	return 0;
}

/**
 * @brief Read all the frames from the RX FIFO.
 * @param desc - lwip sockets layer specific descriptor.
 * @param data - netif to RX data.
 * @return 0 in case of success, negative error otherwise.
 */
static int32_t eth_mac_step(struct lwip_network_desc *desc, void *data)
{
	struct capi_eth_mac_handle *mac_desc;
	struct netif *netif_desc;
	struct pbuf *p;
	uint32_t len;
	int ret;

	netif_desc = desc->lwip_netif;
	mac_desc = desc->mac_desc;

	do {
		ret = eth_mac_read_frames(mac_desc, &p, &len);
		if (ret)
			return ret;

		if (len) {
			LINK_STATS_INC(link.recv);
			ret = netif_desc->input(p, netif_desc);
			if (ret) {
				if (p->ref)
					pbuf_free(p);
			}
		}
	} while (len);

	return 0;
}

/**
 * @brief Write the data inside a pbuf on the wire.
 * @param net - lwip network descriptor to send data to.
 * @param p - pbuf to be sent.
 * @return 0 in case of success, negative error otherwise.
 */
static int32_t eth_mac_netif_output(struct netif *net, struct pbuf *p)
{
	struct lwip_network_desc *lwip_desc;
	struct capi_eth_mac_handle *mac_desc;
	struct capi_eth_frame_buffer buff;
	uint32_t frame_len;
	int ret;

	lwip_desc = net->state;
	mac_desc = lwip_desc->mac_desc;

	LINK_STATS_INC(link.xmit);
	frame_len = pbuf_copy_partial(p, lwip_buff, p->tot_len, 0);

	ret = capi_eth_fb_init(&buff, lwip_buff, sizeof(lwip_buff), 0);
	if (ret)
		return ret;

	buff.end = frame_len;
	buff.data_length = frame_len;

	return capi_eth_mac_send_frame(mac_desc, &buff);
}

/**
 * @brief Intialize and configure filters for the Ethernet MAC.
 * @param desc - Ethernet MAC handle.
 * @param param - initialization parameters for the Ethernet MAC
 *		  (struct capi_eth_mac_init_config).
 * @return 0 in case of success, negative error otherwise.
 */
static int32_t eth_mac_lwip_init(void **desc, void *param)
{
	struct capi_eth_mac_handle *mac = NULL;
	struct capi_eth_mac_init_config *mac_param = param;
	uint8_t mdns_mcast_addr[NETIF_MAX_HWADDR_LEN] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB};
	uint8_t slot;
	int32_t ret;

	ret = capi_eth_mac_init(&mac, mac_param);
	if (ret)
		return ret;

	/*
	 * DEVIATION: the CAPI MAC takes its station address from its init
	 * parameters and accepts it from init, so it gets no filter slot
	 * here.
	 */

	ret = capi_eth_mac_set_broadcast(mac, true);
	if (ret)
		goto free_descriptor;

	ret = capi_eth_mac_set_address_filter(mac, mdns_mcast_addr, &slot);
	if (ret)
		goto free_descriptor;

	/* DEVIATION: a CAPI MAC passes no frames until it is started. */
	ret = capi_eth_mac_start(mac);
	if (ret)
		goto free_descriptor;

	*desc = mac;

	return 0;

free_descriptor:
	capi_eth_mac_deinit(mac);

	return ret;
}

/**
 * @brief Remove the Ethernet MAC handle.
 * @param desc - Ethernet MAC handle.
 * @return 0 in case of success, negative error otherwise.
 */
static int32_t eth_mac_lwip_remove(void *desc)
{
	struct capi_eth_mac_handle *mac = desc;

	return capi_eth_mac_deinit(mac);
}

const struct no_os_lwip_ops eth_mac_lwip_ops = {
	.init = eth_mac_lwip_init,
	.remove = eth_mac_lwip_remove,
	.netif_output = eth_mac_netif_output,
	.step = eth_mac_step,
};

#endif /* NO_OS_LWIP_NETWORKING */
