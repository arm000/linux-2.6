//#ifndef __NOA3301_H__
#define __NOA3301_H__


static void noa3301_hw_config(int hw_init, int hw_active, int hw_standby, int hw_shutdown )
{
  
}

static int noa3301_gpio_setup (int request)
{
	 return 0;
}


struct noa3301_platform_data {
 

/* led_max_curr is a safetylimit for IR leds */
#define NOA3301_LED_5mA   0
#define NOA3301_LED_10mA  1
#define NOA3301_LED_15mA  2
#define NOA3301_LED_20mA  3
#define NOA3301_LED_25mA  4
#define NOA3301_LED_30mA  5
#define NOA3301_LED_35mA  6
#define NOA3301_LED_40mA  7
#define NOA3301_LED_45mA  8
#define NOA3301_LED_50mA  9
#define NOA3301_LED_55mA  10
#define NOA3301_LED_60mA  11
#define NOA3301_LED_65mA  12
#define NOA3301_LED_70mA  13
#define NOA3301_LED_75mA  14
#define NOA3301_LED_80mA  15
#define NOA3301_LED_85mA  16
#define NOA3301_LED_90mA  17
#define NOA3301_LED_95mA  18
#define NOA3301_LED_100mA 19
#define NOA3301_LED_105mA 20
#define NOA3301_LED_110mA 21
#define NOA3301_LED_115mA 22
#define NOA3301_LED_120mA 23
#define NOA3301_LED_125mA 24
#define NOA3301_LED_130mA 25
#define NOA3301_LED_135mA 26
#define NOA3301_LED_140mA 27
#define NOA3301_LED_145mA 28
#define NOA3301_LED_150mA 29
#define NOA3301_LED_155mA 30
#define NOA3301_LED_160mA 31
    //#define NOA3301_INT_CONFIG    0x01 // interrupt pin active till cleared, active high
#define NOA3301_ALS_HYST_TRIGGER 0x01 // Upper: 0x00=Lowwer, 0x01=Upper
#define NOA3301_ALS_HYST_ENABLE  0x00 // Disabled 0x00=Disabled, 0x01=Enabled
#define NOA3301_ALS_INTEGRATION_TIME  0X04 //100ms: 0x000=6.25ms, 0x001=12.5ms,0x010=25ms,0x011=50ms,0x100=200ms,0x110=400,0x111=800ms
#define NOA3301_ALS_THRES_LO	0
#define NOA3301_ALS_THRES_UP	65535
//#define NOA3301_ALS_CONTROL   0x02  // Repeat Mode 0x01=One Shot
//#define NOA3301_ALS_INTERVAL  500 // Valid values 0 - 3150 in steps of 50.

#define NOA3301_PS_FILTER_M 1 // 1 - 15
#define NOA3301_PS_FILTER_N 1 // 1 - 15
#define NOA3301_PS_INTEGRATION_TIME 0x20 // 300us: 0x00=75us, 0x01=150us, 0x10=300us, 0x11=600us
#define NOA3301_PS_HYST_TRIGGER 0x01 // Upper: 0x00=Lowwer, 0x01=Upper
#define NOA3301_PS_HYST_ENABLE  0x00 // Disabled 0x00=Disabled, 0x01=Enabled
#define NOA3301_PS_THRES_LO	1200
#define NOA3301_PS_THRES_UP	1500
//#define NOA3301_PS_INTERVAL  100 // 100ms: Valid values 0 - 3150 in steps of 50.
//#define NOA3301_PS_LED_CURRENT 0x10 // 85ma  NOA3301_PS_LED_CURRENT * 5 + 5
#define NOA3301_PS_MODE NOACHIP_PS_REPEAT 

	 unsigned int is_irq_wakeup;
	 char *phys_dev_path;

	 void (*hw_config)(int hw_init, int hw_active, int hw_standby, int hw_shutdown);
	 int (*gpio_setup)(int request);
};

#define NOA3301_NAME "noa3301"


#if 0
struct noa3301_platform_data noa3301_pdata = {
      .hw_config      = noa3301_hw_config,
      .gpio_setup     = noa3301_gpio_setup,
      .is_irq_wakeup  = 1,
      .phys_dev_path = " ",
};

static struct i2c_board_info noa3301_i2c_board_info[] = {
      {
             I2C_BOARD_INFO(NOA3301_NAME, 0xA8 >> 1),
             .platform_data = &noa3301_pdata,
             .type = NOA3301_NAME,
       },
};
#endif

//#endif
