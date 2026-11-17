/*
 * Copyright (c) 2026 Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * lwIP's lwiperf on CAPI: lwip_socket brings up lwIP on the board's
 * Ethernet MAC, and lwiperf serves iperf2 TCP tests on port 5001. Built with
 * IPERF_CLIENT_TO, it also sends one test to an iperf2 server at that
 * address. Each finished test is reported on the console. The MAC's
 * parameters come from boards/<BOARD>/.
 */
#include <stdio.h>

#include "capi_platform.h"
#include "capi_time.h"
#include "capi_uart.h"
#include "parameters.h"

#include "lwip/apps/lwiperf.h"
#include "lwip/ip_addr.h"

#include "lwip_socket.h"
#include "lwip_eth_mac.h"

#include "board.h"

static const char *const report_names[] = {
	[LWIPERF_TCP_DONE_SERVER] = "done",
	[LWIPERF_TCP_DONE_CLIENT] = "done (client)",
	[LWIPERF_TCP_ABORTED_LOCAL] = "aborted locally",
	[LWIPERF_TCP_ABORTED_LOCAL_DATAERROR] = "aborted: data error",
	[LWIPERF_TCP_ABORTED_LOCAL_TXERROR] = "aborted: TX error",
	[LWIPERF_TCP_ABORTED_REMOTE] = "aborted by the client",
};

static int fail(const char *what, int ret)
{
	printf("%s failed: %d\r\n", what, ret);
	for (;;)
		capi_wait_ms(1000);
	return ret;
}

static void report(void *arg, enum lwiperf_report_type type,
		   const ip_addr_t *local_addr, u16_t local_port,
		   const ip_addr_t *remote_addr, u16_t remote_port,
		   u32_t bytes, u32_t ms, u32_t kbitps)
{
	char remote[IPADDR_STRLEN_MAX];

	(void)arg;
	(void)local_addr;
	(void)local_port;

	ipaddr_ntoa_r(remote_addr, remote, sizeof(remote));
	printf("iperf %s, %s:%u: %lu bytes in %lu ms, %lu kbit/s\r\n",
	       type < sizeof(report_names) / sizeof(report_names[0]) ? report_names[type] : "?",
	       remote, remote_port, (unsigned long)bytes, (unsigned long)ms,
	       (unsigned long)kbitps);
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
	int32_t ret;

	capi_platform_init();
	capi_uart_init(&uart, &bsp_uart_cfg);
	capi_stdio_init(uart);
	printf("\r\n*** lwIP iperf TCP server example ***\r\n\r\n");

	ret = board_eth_mac_config(&mac_cfg, lwip_param.hwaddr);
	if (ret)
		return fail("Board setup", ret);

	ret = no_os_lwip_init(&lwip, &lwip_param);
	if (ret)
		return fail("no_os_lwip_init", ret);

	if (!lwiperf_start_tcp_server_default(report, NULL))
		return fail("lwiperf_start_tcp_server_default", -1);
	printf("iperf server on TCP port %d\r\n", LWIPERF_TCP_PORT_DEFAULT);

#ifdef IPERF_CLIENT_TO
	{
		ip_addr_t server;

		if (!ipaddr_aton(IPERF_CLIENT_TO, &server) ||
		    !lwiperf_start_tcp_client_default(&server, report, NULL))
			return fail("lwiperf_start_tcp_client_default", -1);
		printf("iperf client to %s:%d\r\n", IPERF_CLIENT_TO, LWIPERF_TCP_PORT_DEFAULT);
	}
#endif

	for (;;) {
		ret = no_os_lwip_step(lwip, NULL);
		if (ret)
			printf("no_os_lwip_step: %ld\r\n", (long)ret);
	}

	return 0;
}
