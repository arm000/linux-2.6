/* This file is part of the NOA3301 sensor driver.
 * Chip is combined proximity and ambient light sensor.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef __NOA3301_HEADER__
#define __NOA3301_HEADER__

// Register Definitions
#define NAOCHIP_REG_NUM 26
#define NOACHIP_VENDOR      0x0001
/* PART_ID */
#define NOACHIP_PART		0x90	/* needs to be set to the correct part id */
#define NOACHIP_PART_MASK	0xf0
#define NOACHIP_REV_MASK	0x0f
#define NOACHIP_REV_SHIFT	0
#define NOACHIP_REV_0		0x00

/* Operating modes for both */
#define NOACHIP_STANDBY		0x00
#define NOACHIP_PS_ONESHOT	0x01
#define NOACHIP_PS_REPEAT	0x02

#define NOACHIP_TRIG_MEAS	0x01

/* Interrupt control */
#define NOACHIP_INT_LEDS_INT_HI	(1 << 1)
#define NOACHIP_INT_LEDS_INT_LO	(1 << 0)
#define NOACHIP_INT_ALS_INT_HI	(1 << 2)
#define NOACHIP_INT_ALS_INT_LO  (1 << 3)

#define NOACHIP_DISABLE		0
#define NOACHIP_ENABLE		3

struct noa3301_chip {
	struct i2c_client *client;
	struct noa3301_platform_data *pdata;

	char chipname[10];
	u8 revision;
};

extern struct noa3301_chip *noa3301;

#endif
