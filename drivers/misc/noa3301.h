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
#define NOACHIP_PART		0x90  /* needs to be set to the correct part id */
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

enum noa_regs {
	 NOACHIP_PART_ID,
	 NOACHIP_RESET,
	 NOACHIP_INT_CONFIG,
	 NOACHIP_PS_LED_CURRENT,
	 NOACHIP_PS_FILTER_CONFIG,
	 NOACHIP_PS_CONFIG,
	 NOACHIP_TH_UP_MSB,
	 NOACHIP_TH_UP_LSB,
	 NOACHIP_TH_LO_MSB,
	 NOACHIP_TH_LO_LSB,
	 NOACHIP_PS_MEAS_INTERVAL,
	 NOACHIP_PS_CONTROL,
	 NOACHIP_ALS_TH_UP_MSB,
	 NOACHIP_ALS_TH_UP_LSB,
	 NOACHIP_ALS_TH_LO_MSB,
	 NOACHIP_ALS_TH_LO_LSB,
	 NOACHIP_ALS_FILTER_CONFIG,
	 NOACHIP_ALS_CONFIG,
	 NOACHIP_ALS_MEAS_INTERVAL,
	 NOACHIP_ALS_CONTROL,
	 NOACHIP_INTERRUPT,
	 NOACHIP_PS_DATA_MSB,
	 NOACHIP_PS_DATA_LSB,
	 NOACHIP_ALS_DATA_MSB,
	 NOACHIP_ALS_DATA_LSB
};

struct noa_reg {
	 u8 value;
	 bool update;  
};

static const char noa_regmap[]={
	 [NOACHIP_PART_ID] = 0x00, /* Part number and revision ID */
	 [NOACHIP_RESET] = 0x01, /* Reset */
	 [NOACHIP_INT_CONFIG] = 0x02, /* Interrupt pin function control settings */
	 [NOACHIP_PS_LED_CURRENT] = 0x0F, /* Set PS LED Current */
	 [NOACHIP_PS_FILTER_CONFIG] = 0x14, /* Sets N and M setting for PS fileter */
	 [NOACHIP_PS_CONFIG] = 0x15, /* PS Range and intergration time */
	 [NOACHIP_TH_UP_MSB] = 0x10, /* PS interrupt upper threshold, MSB */
	 [NOACHIP_TH_UP_LSB] = 0x11, /* PS interrupt upper threshold, LSB */
	 [NOACHIP_TH_LO_MSB] = 0x12, /* PS interrupt lower threshold, MSB */
	 [NOACHIP_TH_LO_LSB] = 0x13, /* PS interrupt lower threshold, LSB */
	 [NOACHIP_PS_MEAS_INTERVAL] = 0x16, /* PS meas. interval at stand alone mode */
	 [NOACHIP_PS_CONTROL] = 0x17, /* PS operation mode control 0x02 = PS_Repeat, 0x01=PS_OneShot */
	 [NOACHIP_ALS_TH_UP_MSB] = 0x20, /* ALS upper threshold MSB */
	 [NOACHIP_ALS_TH_UP_LSB] = 0x21, /* ALS upper threshold LSB */
	 [NOACHIP_ALS_TH_LO_MSB] = 0x22, /* ALS lower threshold MSB */
	 [NOACHIP_ALS_TH_LO_LSB] = 0x23, /* ALS lower threshold LSB */
	 [NOACHIP_ALS_FILTER_CONFIG] = 0x24, /* ALS filter config */
	 [NOACHIP_ALS_CONFIG] = 0x25, /* ALS intergration time */
	 [NOACHIP_ALS_MEAS_INTERVAL] = 0x26, /* ALS meas. interval at stand alone mode */
	 [NOACHIP_ALS_CONTROL] = 0x27, /* ALS operation mode control */
	 [NOACHIP_INTERRUPT] = 0x40, /* Interrupt status */
	 [NOACHIP_PS_DATA_MSB] = 0x41, /* PS DATA MSB */
	 [NOACHIP_PS_DATA_LSB] = 0x42, /* PS DATA LSB */
	 [NOACHIP_ALS_DATA_MSB] = 0x43, /* ALS DATA high byte */
	 [NOACHIP_ALS_DATA_LSB] = 0x44 /* ALS DATA low byte */
};
        


	 


struct noa3301_chip {
	 struct i2c_client		*client;
	 struct noa3301_platform_data	*pdata;
	 struct input_dev                *input_dev;
	 struct mutex			mutex; /* avoid parallel access */
	 u8				int_config;
	 struct noa_reg              regs[NAOCHIP_REG_NUM];
	 struct work_struct         update_work;  
	 struct work_struct		ps_read_work; /* For ps low threshold */
	 struct work_struct          ps_mode_work;
	 struct work_struct          als_read_work;

	 char				chipname[10];
	 u8				revision;
        int                             interrupt;
        unsigned int active:1;

	
        int	  als_interval;
	 int	  als_mode;
	 u16	  als_data;
	 u16	  als_threshold_up;
	 u16	  als_threshold_lo;
        u16	  ps_data;
        u16	  ps_threshold_lo;
        u16	  ps_threshold_up;
        u8	  ps_led;


        int     work_error; // error inside work
        
};
        
extern struct noa3301_chip *noa3301;

#endif
