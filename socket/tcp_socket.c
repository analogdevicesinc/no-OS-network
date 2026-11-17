/***************************************************************************//**
 *   @file   tcp_socket.c
 *   @brief  TCP Socket functions
 *   @author Mihail Chindris (mihail.chindris@analog.com)
********************************************************************************
 *   @copyright
 * Copyright 2020(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <stdlib.h>
#include "no_os_error.h"
#include "tcp_socket.h"
#include "no_os_util.h"
#include "no_os_alloc.h"

/* NOTE: TCP socket communication is not secured */

#define DEFAULT_CONNECTION_BUFFER_SIZE 256

/**
 * @brief Allocate resources and initializes the socket descriptor
 * @param desc - Address where to store the socket descriptor
 * @param param - Initializing data
 * @return
 *  - 0 : On success
 *  - -1 : Otherwise
 */
int32_t socket_init(struct tcp_socket_desc **desc,
		    struct tcp_socket_init_param *param)
{
	struct tcp_socket_desc	*ldesc;
	int32_t			ret;
	uint32_t		buff_size;

	if (!desc || !param)
		return -1;

	ldesc = (typeof(ldesc))no_os_calloc(1, sizeof(*ldesc));
	if (!ldesc)
		return -1;

	ldesc->net = param->net;

	if (param->max_buff_size != 0)
		buff_size = param->max_buff_size;
	else
		buff_size = DEFAULT_CONNECTION_BUFFER_SIZE;

	ret = ldesc->net->socket_open(ldesc->net->net, &ldesc->id, PROTOCOL_TCP,
				      buff_size);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		no_os_free(ldesc);
		return ret;
	}

	*desc = ldesc;

	return 0;
}

/**
 * @brief Deallocate resources from the socket descriptor
 * @param desc - Socket descriptor
 * @return
 *  - 0 : On success
 *  - -1 : Otherwise
 */
int32_t socket_remove(struct tcp_socket_desc *desc)
{
	int32_t ret;

	if (!desc)
		return -1;

	ret = desc->net->socket_close(desc->net->net, desc->id);
	if (NO_OS_IS_ERR_VALUE(ret))
		return ret;
	no_os_free(desc);

	return 0;
}

/** @brief See \ref network_interface.socket_connect */
int32_t socket_connect(struct tcp_socket_desc *desc,
		       struct socket_address *addr)
{
	int32_t ret;

	if (!desc || !addr)
		return -1;

	ret = desc->net->socket_connect(desc->net->net,
					desc->id, addr);
	if (NO_OS_IS_ERR_VALUE(ret))
		return ret;

	return 0;
}

/** @brief See \ref network_interface.socket_disconnect */
int32_t socket_disconnect(struct tcp_socket_desc *desc)
{
	if (!desc)
		return -1;

	return desc->net->socket_disconnect(desc->net->net, desc->id);
}

/** @brief See \ref network_interface.socket_send */
int32_t socket_send(struct tcp_socket_desc *desc, const void *data,
		    uint32_t len)
{

	if (!desc)
		return -1;

	return desc->net->socket_send(desc->net->net, desc->id,
				      data, len);
}

/** @brief See \ref network_interface.socket_recv */
int32_t socket_recv(struct tcp_socket_desc *desc, void *data, uint32_t len)
{
	if (!desc)
		return -1;

	return desc->net->socket_recv(desc->net->net, desc->id, data,
				      len);
}

/** @brief See \ref network_interface.socket_bind */
int32_t socket_bind(struct tcp_socket_desc *desc, uint16_t port)
{

	return desc->net->socket_bind(desc->net->net, desc->id, port);
}

/** @brief See \ref network_interface.socket_listen */
int32_t socket_listen(struct tcp_socket_desc *desc, uint32_t back_log)
{

	return desc->net->socket_listen(desc->net->net, desc->id, back_log);
}

/** @brief See \ref network_interface.socket_accept */
int32_t socket_accept(struct tcp_socket_desc *desc,
		      struct tcp_socket_desc **new_client)
{
	uint32_t		new_cli_id;
	int32_t			ret;

	ret = desc->net->socket_accept(desc->net->net, desc->id, &new_cli_id);
	if (NO_OS_IS_ERR_VALUE(ret))
		return ret;

	*new_client = (typeof(*new_client))no_os_calloc(1, sizeof(**new_client));
	if (!*new_client) {
		desc->net->socket_close(desc->net->net, desc->id);
		return -ENOMEM;
	}
	(*new_client)->net = desc->net;
	(*new_client)->id = new_cli_id;

	return 0;
}

