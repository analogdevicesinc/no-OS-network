/***************************************************************************//**
 *   @file   tcp_socket.h
 *   @brief  TCP Socket functions
 *   @author Mihail Chindris (mihail.chindris@analog.com)
********************************************************************************
 *   @copyright
 * Copyright 2020(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef TCP_SOCKET_H_
# define TCP_SOCKET_H_

#include "network_interface.h"
#include <stdint.h>

#define MAX_BACKLOG 0xFFFFFFFF

/* Socket descriptor */
struct tcp_socket_desc {
	/* Id of the opened socket */
	uint32_t			id;
	/* Reference to the network interface */
	struct network_interface	*net;
};

/**
 * @struct tcp_socket_init_param
 * @brief Parameter to initialize a TCP Socket
 */
struct tcp_socket_init_param {
	/** Reference to the network interface */
	struct network_interface	*net;
	/**
	 *  Max buffer size for incoming data.
	 *  If set to 0, default value will be used:
	 *  DEFAULT_CONNECTION_BUFFER_SIZE from tcp_socket.c
	 */
	uint32_t			max_buff_size;
};

/* Init socket */
int32_t socket_init(struct tcp_socket_desc **desc,
		    struct tcp_socket_init_param *param);

/* Socket remove */
int32_t socket_remove(struct tcp_socket_desc *desc);

/* Socket connect */
int32_t socket_connect(struct tcp_socket_desc *desc,
		       struct socket_address *addr);

/* Socket disconnect */
int32_t socket_disconnect(struct tcp_socket_desc *desc);

/* Socket send */
int32_t socket_send(struct tcp_socket_desc *desc, const void *data,
		    uint32_t len);

/* Socket recv */
int32_t socket_recv(struct tcp_socket_desc *desc, void *data, uint32_t len);

/* Socket bind */
int32_t socket_bind(struct tcp_socket_desc *desc, uint16_t port);

/* Socket listen */
int32_t socket_listen(struct tcp_socket_desc *desc, uint32_t back_log);

/* Socket accept */
int32_t socket_accept(struct tcp_socket_desc *desc,
		      struct tcp_socket_desc **new_client);

#endif
