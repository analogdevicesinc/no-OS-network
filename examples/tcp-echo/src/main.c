/*
 * Copyright (c) 2026 Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The no-OS network code on CAPI: lwip_socket brings up lwIP on the
 * board's Ethernet MAC, and a tcp_socket server echoes back what a client
 * sends on TCP port 10000. The MAC's parameters come from boards/<BOARD>/.
 */
#include <errno.h>
#include <stdio.h>

#include "capi_platform.h"
#include "capi_time.h"
#include "capi_uart.h"
#include "parameters.h"

#include "lwip_socket.h"
#include "lwip_eth_mac.h"
#include "tcp_socket.h"

#include "board.h"

#define ECHO_PORT	10000
#define ECHO_BUFF_SIZE	1024

static uint8_t buff[ECHO_BUFF_SIZE];

static int fail(const char *what, int ret)
{
	printf("%s failed: %d\r\n", what, ret);
	for (;;)
		capi_wait_ms(1000);
	return ret;
}

int main(void)
{
	struct capi_uart_handle *uart = NULL;
	struct capi_eth_mac_init_config mac_cfg = {0};
	struct lwip_network_param lwip_param = {
		.platform_ops = &eth_mac_lwip_ops,
		.mac_param = &mac_cfg,
	};
	struct lwip_network_desc *lwip;
	struct tcp_socket_init_param sock_param = {
		.max_buff_size = 0,
	};
	struct tcp_socket_desc *server, *client = NULL;
	int32_t ret;

	capi_platform_init();
	capi_uart_init(&uart, &bsp_uart_cfg);
	capi_stdio_init(uart);
	printf("\r\n*** no-OS network TCP echo example ***\r\n\r\n");

	ret = board_eth_mac_config(&mac_cfg, lwip_param.hwaddr);
	if (ret)
		return fail("Board setup", ret);

	ret = no_os_lwip_init(&lwip, &lwip_param);
	if (ret)
		return fail("no_os_lwip_init", ret);

	sock_param.net = &lwip->no_os_net;
	ret = socket_init(&server, &sock_param);
	if (ret)
		return fail("socket_init", ret);
	ret = socket_bind(server, ECHO_PORT);
	if (ret)
		return fail("socket_bind", ret);
	ret = socket_listen(server, 1);
	if (ret)
		return fail("socket_listen", ret);
	printf("TCP echo on port %d\r\n", ECHO_PORT);

	for (;;) {
		ret = no_os_lwip_step(lwip, NULL);
		if (ret)
			printf("no_os_lwip_step: %ld\r\n", (long)ret);

		if (!client) {
			ret = socket_accept(server, &client);
			if (ret == -EAGAIN)
				continue;
			if (ret) {
				printf("socket_accept: %ld\r\n", (long)ret);
				client = NULL;
				continue;
			}
			printf("client connected\r\n");
		}

		ret = socket_recv(client, buff, sizeof(buff));
		for (int32_t len = ret, sent = 0; ret > 0 && sent < len; sent += ret) {
			/*
			 * socket_send() takes what fits in the TCP send buffer;
			 * when that is full, let lwIP send and take ACKs first.
			 */
			while (!(ret = socket_send(client, &buff[sent], len - sent)))
				no_os_lwip_step(lwip, NULL);
		}
		if (ret < 0) {
			printf("client disconnected (%ld)\r\n", (long)ret);
			socket_remove(client);
			client = NULL;
		}
	}

	return 0;
}
