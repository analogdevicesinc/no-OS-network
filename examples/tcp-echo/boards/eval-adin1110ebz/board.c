/*
 * Copyright (c) 2026 Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * EVAL-ADIN1110EBZ: the ADIN1110 on SPI2, with CS on PB12, INT_N on PB11,
 * RESET_N on PC7 and the SPI_CFG0 strap readable on PC8.
 */
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

static struct capi_spi_device spi_dev = {
	.max_speed_hz = ADIN1110_SPI_HZ,
	.mode = CAPI_SPI_MODE_0,
	.cs_gpio = &cs_n,
};

static struct adin1110_config adin_cfg = {
	.spi = &spi_dev,
	.reset = &reset_n,
	.irq = &int_n,
	.mac_address = {0x00, 0xE0, 0x22, 0x66, 0xAA, 0x01},
};

int board_eth_mac_config(struct capi_eth_mac_init_config *cfg, uint8_t hwaddr[6])
{
	struct capi_gpio_port_handle *gpiob = NULL, *gpioc = NULL;
	struct capi_spi_controller_handle *spi = NULL;
	uint8_t strap = 0;
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

	ret = capi_spi_init(&spi, &bsp_spi_cfg);
	if (ret)
		return ret;
	spi_dev.controller = spi;

	/* The driver refuses a protection mode that does not match SPI_CFG0. */
	ret = capi_gpio_pin_set_direction(&cfg0, CAPI_GPIO_INPUT);
	if (!ret)
		ret = capi_gpio_pin_get_raw_value(&cfg0, &strap);
	if (ret)
		return ret;
	adin_cfg.protection = strap ? ADIN1110_OA_UNPROTECTED : ADIN1110_OA_PROTECTED;

	cfg->ops = &adin1110_eth_mac_ops;
	cfg->extra = &adin_cfg;
	memcpy(hwaddr, adin_cfg.mac_address, 6);

	return 0;
}
