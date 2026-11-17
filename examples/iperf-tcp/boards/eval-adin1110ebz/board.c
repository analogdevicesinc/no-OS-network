/*
 * Copyright (c) 2026 Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * EVAL-ADIN1110EBZ: the ADIN1110 on SPI2, with CS on PB12, INT_N on PB11,
 * RESET_N on PC7 and the SPI_CFG0/SPI_CFG1 straps readable on PC8/PC9. The
 * MAC is the no-OS ADIN1110 driver, as a CAPI Ethernet MAC.
 */
#include <errno.h>
#include <string.h>

#include "capi_gpio.h"
#include "capi_spi.h"
#include "parameters.h"

#include "adin1110.h"
#include "board.h"

static struct stm32_gpio_extra_config gpiob_extra = {.port = GPIOB};
static struct stm32_gpio_extra_config gpioc_extra = {.port = GPIOC};

static struct capi_gpio_port_config gpiob_cfg = {
	.ops = &stm32_capi_gpio_ops,
	.identifier = (uint64_t)(uintptr_t)GPIOB,
	.num_pins = 16,
	.extra = &gpiob_extra,
};

static struct capi_gpio_port_config gpioc_cfg = {
	.ops = &stm32_capi_gpio_ops,
	.identifier = (uint64_t)(uintptr_t)GPIOC,
	.num_pins = 16,
	.extra = &gpioc_extra,
};

static struct capi_gpio_pin cs_n = {.number = ADIN1110_CS_PIN};
static struct capi_gpio_pin int_n = {.number = ADIN1110_INT_PIN};
static struct capi_gpio_pin reset_n = {.number = ADIN1110_RESET_PIN};
static struct capi_gpio_pin cfg0 = {.number = ADIN1110_CFG0_PIN};
static struct capi_gpio_pin cfg1 = {.number = ADIN1110_CFG1_PIN};

static struct capi_spi_device spi_dev = {
	.max_speed_hz = ADIN1110_SPI_HZ,
	.mode = CAPI_SPI_MODE_0,
	.cs_gpio = &cs_n,
};

static struct adin1110_init_param adin_param = {
	.chip_type = ADIN1110,
	.comm_desc = &spi_dev,
	.reset_gpio = &reset_n,
	.int_gpio = &int_n,
	.mac_address = {0x00, 0xE0, 0x22, 0x66, 0xAA, 0x01},
};

static int read_strap(struct capi_gpio_pin *pin, uint8_t *value)
{
	int ret;

	ret = capi_gpio_pin_set_direction(pin, CAPI_GPIO_INPUT);
	if (ret)
		return ret;

	return capi_gpio_pin_get_raw_value(pin, value);
}

int board_eth_mac_config(struct capi_eth_mac_init_config *cfg, uint8_t hwaddr[6])
{
	struct capi_gpio_port_handle *gpiob = NULL, *gpioc = NULL;
	struct capi_spi_controller_handle *spi = NULL;
	uint8_t spi_cfg0 = 0, spi_cfg1 = 0;
	int ret;

	ret = capi_gpio_port_init(&gpiob, &gpiob_cfg);
	if (ret)
		return ret;
	ret = capi_gpio_port_init(&gpioc, &gpioc_cfg);
	if (ret)
		return ret;
	cs_n.port_handle = gpiob;
	int_n.port_handle = gpiob;
	reset_n.port_handle = gpioc;
	cfg0.port_handle = gpioc;
	cfg1.port_handle = gpioc;

	ret = capi_spi_init(&spi, &bsp_spi_cfg);
	if (ret)
		return ret;
	spi_dev.controller = spi;

	/*
	 * The SPI_CFG1/SPI_CFG0 straps select the SPI protocol: OPEN Alliance
	 * with (0/0) or without (0/1) protection, or generic SPI with (1/0) or
	 * without (1/1) CRC. The driver cannot select protected OPEN Alliance.
	 */
	ret = read_strap(&cfg0, &spi_cfg0);
	if (ret)
		return ret;
	ret = read_strap(&cfg1, &spi_cfg1);
	if (ret)
		return ret;
	if (!spi_cfg1 && !spi_cfg0)
		return -ENOTSUP;
	adin_param.oa_tc6_spi = !spi_cfg1;
	adin_param.append_crc = spi_cfg1 && !spi_cfg0;

	cfg->ops = &adin1110_eth_mac_ops;
	cfg->extra = &adin_param;
	memcpy(hwaddr, adin_param.mac_address, 6);

	return 0;
}
