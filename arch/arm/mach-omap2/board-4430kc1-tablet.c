/*
 * Board support file for OMAP4430KC1 tablet.
 *
 * Copyright (C) 2009 Texas Instruments
 *
 * Author: Santosh Shilimkar <santosh.shilimkar@ti.com>
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Based on mach-omap2/board-3430sdp.c
 * Based on mach-omap2/board-4430sdp.c
 * Based on mach-omap2/board-44xx-tablet.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/usb/otg.h>
#include <linux/spi/spi.h>
#include <linux/i2c/twl.h>
#include <linux/leds-omap4430sdp-display.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/omapfb.h>
#include <linux/wl12xx.h>
#include <linux/skbuff.h>
#include <linux/reboot.h>
#include <linux/ti_wilink_st.h>
#include <linux/bootmem.h>
#include <linux/module.h>
#include <linux/wakelock.h>
#include <linux/memblock.h>
#include <linux/omap_die_governor.h>

#include <mach/hardware.h>
#include <mach/omap4_ion.h>
#include <mach/omap-secure.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/hardware/gic.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/dmtimer.h>
#include <plat/drm.h>
#include <plat/mmc.h>
#include <plat/omap_apps_brd_id.h>
#include <plat/omap_device.h>
#include <plat/omap_hwmod.h>
#include <plat/omap-pm.h>
#include <plat/omap-serial.h>
#include <plat/remoteproc.h>
#include <plat/usb.h>
#include <plat/vram.h>

#include <video/omapdss.h>

#include <linux/mfd/tlv320aic31xx-registers.h>
#include <linux/mfd/tlv320aic3xxx-registers.h>
#include <linux/mfd/tlv320aic3xxx-core.h>

#include <linux/platform_data/emif_plat.h>
#include <linux/platform_data/lm75_platform_data.h>
#include <linux/input/bma250.h>
#include <linux/power/bq27541_battery.h>
#include <linux/leds-otter-button.h>
#include <linux/power/smb347-otter.h>

#include "omap_ram_console.h"
#include "common.h"
#include "common-board-devices.h"
#include "mux.h"
#include "hsmmc.h"
#include "control.h"
#include "pm.h"
#include "prm-regbits-44xx.h"
#include "prm44xx.h"
#include "voltage.h"

#include "board-4430kc1-tablet.h"

#define WILINK_UART_DEV_NAME		"/dev/ttyO1"
#define BLUETOOTH_UART			(0x1)
#define CONSOLE_UART			(0x2)

#define OMAP4_TOUCH_RESET_GPIO	18

#define OMAP4_TOUCH_IRQ_1		35
#define OMAP4_TOUCH_IRQ_2		36
#define OMAP_UART_GPIO_MUX_MODE_143	143

#define PANEL_IRQ			34


#define GPIO_WIFI_PMENA			54
#define GPIO_WIFI_IRQ			53

#define OMAP4SDP_MDM_PWR_EN_GPIO	157

#define MBID0_GPIO			174
#define MBID1_GPIO			173
#define MBID2_GPIO			178
#define MBID3_GPIO			177
#define PANELID0_GPIO			176
#define PANELID1_GPIO			175
#define TOUCHID0_GPIO			50
#define TOUCHID1_GPIO			51

#define TWL6030_RTC_GPIO		6

//static struct wake_lock uart_lock;

/* Board IDs */
static u8 quanta_mbid;
static u8 quanta_touchid;
static u8 quanta_panelid;
u8 quanta_get_mbid(void)
{
	return quanta_mbid;
}
EXPORT_SYMBOL(quanta_get_mbid);

u8 quanta_get_touchid(void)
{
	return quanta_touchid;
}
EXPORT_SYMBOL(quanta_get_touchid);

u8 quanta_get_panelid(void)
{
	return quanta_panelid;
}
EXPORT_SYMBOL(quanta_get_panelid);

static void __init quanta_boardids(void)
{
    gpio_request(MBID0_GPIO, "MBID0");
    gpio_direction_input(MBID0_GPIO);
    gpio_request(MBID1_GPIO, "MBID1");
    gpio_direction_input(MBID1_GPIO);
    gpio_request(MBID2_GPIO, "MBID2");
    gpio_direction_input(MBID2_GPIO);
    gpio_request(MBID3_GPIO, "MBID3");
    gpio_direction_input(MBID3_GPIO);
    gpio_request(PANELID0_GPIO, "PANELID0");
    gpio_direction_input(PANELID0_GPIO);
    gpio_request(PANELID1_GPIO, "PANELID1");
    gpio_direction_input(PANELID1_GPIO);
    gpio_request(TOUCHID0_GPIO, "TOUCHID0");
    gpio_direction_input(TOUCHID0_GPIO);
    gpio_request(TOUCHID1_GPIO, "TOUCHID1");
    gpio_direction_input(TOUCHID1_GPIO);
    quanta_mbid=gpio_get_value(MBID0_GPIO) | ( gpio_get_value(MBID1_GPIO)<<1)
        | ( gpio_get_value(MBID2_GPIO)<<2) | ( gpio_get_value(MBID3_GPIO)<<3);
    quanta_touchid = gpio_get_value(TOUCHID0_GPIO) | ( gpio_get_value(TOUCHID1_GPIO)<<1);
    quanta_panelid = gpio_get_value(PANELID0_GPIO) | ( gpio_get_value(PANELID1_GPIO)<<1);
}

#ifdef CONFIG_SND_SOC_TLV320AIC31XX
static struct aic3xxx_gpio_setup aic3xxx_gpio[] ={
	{
		.reg = AIC3XXX_MAKE_REG(0, 0, 51),
		.value = 0x14,
	},
};

static struct aic3xxx_pdata aic31xx_codec_pdata ={
	.audio_mclk1 = 19200000,
	.gpio_defaults = aic3xxx_gpio,
	.num_gpios = ARRAY_SIZE(aic3xxx_gpio),
	.irq_base = TWL6040_CODEC_IRQ_BASE,
	.regulator_name = "audio-pwr",
	.regulator_min_uV = 3000000,
	.regulator_max_uV = 3000000,
};

static struct platform_device sdp4430_aic3110 = {
        .name   = "omap4-panda-aic31xx",
        .id     = -1,
};
#endif

#ifdef CONFIG_LEDS_OTTER_BUTTON
#define LED_PWM1ON		0x00
#define LED_PWM1OFF		0x01
#define LED_PWM2ON		0x03
#define LED_PWM2OFF		0x04
#define LED_TOGGLE3		0x92

static void __init orange_led_init(void) {
	twl_i2c_write_u8(TWL_MODULE_PWM, 0xFF, LED_PWM2ON);
	twl_i2c_write_u8(TWL_MODULE_PWM, 0x7F, LED_PWM2OFF);
	twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x08, LED_TOGGLE3);
	twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x38, LED_TOGGLE3);
}

static void orange_led_set(u8 brightness) {
	if (brightness > 1) {
		if (brightness == LED_FULL)
			brightness = 0x7F;
		else
			brightness = (~(brightness/2)) & 0x7F;
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x30, LED_TOGGLE3);
		twl_i2c_write_u8(TWL_MODULE_PWM, brightness, LED_PWM2ON);
	} else if (brightness <= 1) {
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x08, LED_TOGGLE3);
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x38, LED_TOGGLE3);
	}
}

static void __init green_led_init(void) {
	twl_i2c_write_u8(TWL_MODULE_PWM, 0xFF, LED_PWM1ON);
	twl_i2c_write_u8(TWL_MODULE_PWM, 0x7F, LED_PWM1OFF);
	twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x01, LED_TOGGLE3);
	twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x07, LED_TOGGLE3);
};

static void green_led_set(u8 brightness) {
	if (brightness > 1) {
		if (brightness == LED_FULL)
			brightness = 0x7F;
		else
			brightness = (~(brightness/2)) & 0x7F;
		twl_i2c_write_u8(TWL_MODULE_PWM, brightness, LED_PWM1ON);
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x06, LED_TOGGLE3);
	} else {
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x01, LED_TOGGLE3);
		twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x07, LED_TOGGLE3);
	}
};

static struct otter_button_led_platform_data orange_led_data = {
	.name = "led-orange",
	.led_init = orange_led_init,
	.led_set_brightness = orange_led_set,
};

static struct platform_device orange_led = {
	.name	= "leds-otter-button",
	.id	= 0,
	.dev	= {
		.platform_data = &orange_led_data,
	},
};

static struct otter_button_led_platform_data green_led_data = {
	.name = "led-green",
	.led_init = green_led_init,
	.led_set_brightness = green_led_set,
};

static struct platform_device green_led = {
	.name	= "leds-otter-button",
	.id	= 1,
	.dev	= {
		.platform_data = &green_led_data,
	},
};

static void battery_led_callback(u8 green_value, u8 orange_value) {
	green_led_set(green_value);
	orange_led_set(orange_value);
};
#else
static void battery_led_callback(u8 green_value, u8 orange_value) {};
#endif

static struct platform_device __initdata *sdp4430_devices[] = {
#ifdef CONFIG_LEDS_OTTER_BUTTON
	&orange_led,
	&green_led,
#endif
#ifdef CONFIG_SND_SOC_TLV320AIC31XX
	&sdp4430_aic3110,
#endif
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_UTMI,
	.mode			= MUSB_OTG,
	.power			= 200,
};

/* Initial set of thresholds for different thermal zones */
static struct omap_thermal_zone thermal_zones[] = {
	OMAP_THERMAL_ZONE("safe", 0, 25000, 65000, 250, 1000, 400),
	OMAP_THERMAL_ZONE("monitor", 0, 60000, 80000, 250, 250,	250),
	OMAP_THERMAL_ZONE("alert", 0, 75000, 90000, 250, 250, 150),
	OMAP_THERMAL_ZONE("critical", 1, 85000,	115000,	250, 250, 50),
};

static struct omap_die_governor_pdata omap_gov_pdata = {
	.zones = thermal_zones,
	.zones_num = ARRAY_SIZE(thermal_zones),
};

/* MMC / Wifi */

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA | MMC_CAP_1_8V_DDR,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_29_30,
		.nonremovable	= true,
#ifdef CONFIG_PM_RUNTIME
		.power_saving	= true,
#endif
	},
	{
		.mmc		= 5,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
		.pm_caps	= MMC_PM_KEEP_POWER,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_165_195,
		.built_in	= 1,
		.nonremovable	= true,
	},
	{}	/* Terminator */
};

static struct wl12xx_platform_data __initdata omap4_kc1_wlan_data = {
	.irq = OMAP_GPIO_IRQ(GPIO_WIFI_IRQ),
	.board_ref_clock = WL12XX_REFCLOCK_38_XTAL,
};

static struct regulator_consumer_supply omap4_sdp4430_vmmc5_supply = {
	.supply = "vmmc",
	.dev_name = "omap_hsmmc.4",
};
static struct regulator_init_data sdp4430_vmmc5 = {
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &omap4_sdp4430_vmmc5_supply,
};
static struct fixed_voltage_config sdp4430_vwlan = {
	.supply_name		= "vwl1271",
	.microvolts		= 1800000, /* 1.8V */
	.gpio			= GPIO_WIFI_PMENA,
	.startup_delay		= 70000, /* 70msec */
	.enable_high		= 1,
	.enabled_at_boot	= 0,
	.init_data		= &sdp4430_vmmc5,
};
static struct platform_device omap_vwlan_device = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev = { .platform_data = &sdp4430_vwlan, }
};

static int omap4_twl6030_hsmmc_late_init(struct device *dev)
{
	int irq = 0;
	struct platform_device *pdev = container_of(dev,
				struct platform_device, dev);
	struct omap_mmc_platform_data *pdata = dev->platform_data;

	/* Setting MMC1 Card detect Irq */
	if (pdev->id == 0) {
		irq = twl6030_mmc_card_detect_config();
		if (irq < 0) {
			pr_err("Failed configuring MMC1 card detect\n");
			return irq;
		}
		pdata->slots[0].card_detect_irq = irq;
		pdata->slots[0].card_detect = twl6030_mmc_card_detect;
	}
//	if (pdev->id == 4)
//		pdata->slots[0].set_power = wifi_set_power;

	return 0;
}

static __init void omap4_twl6030_hsmmc_set_late_init(struct device *dev)
{
	struct omap_mmc_platform_data *pdata;

	/* dev can be null if CONFIG_MMC_OMAP_HS is not set */
	if (!dev) {
		pr_err("Failed %s\n", __func__);
		return;
	}
	pdata = dev->platform_data;
	pdata->init = omap4_twl6030_hsmmc_late_init;
}

static int __init omap4_twl6030_hsmmc_init(struct omap2_hsmmc_info *controllers)
{
	struct omap2_hsmmc_info *c;

	omap_hsmmc_init(controllers);
	for (c = controllers; c->mmc; c++)
		omap4_twl6030_hsmmc_set_late_init(&c->pdev->dev);

	return 0;
}

/***** I2C BOARD INIT ****/

static struct lm75_platform_data lm75_pdata = {
	.domain = "case",
	.stats_enable = 1,
	.average_period = 2000,
	.average_number = 20,
};

static struct bma250_platform_data bma250_pdata = {
	.regulator_name = "g-sensor-pwr",
	.min_voltage	= 1800000,
	.max_voltage	= 1800000,
	.range		= BMA250_RANGE_2G,
	.shift_adj	= BMA250_SHIFT_RANGE_2G,
	.report_threshold = 5,
	.poll_interval	= 40,
	.max_interval	= 200,
	.min_interval	= 32,
	.axis_map_x	= 0,
	.axis_map_y	= 1,
	.axis_map_z	= 2,
};

static struct bq27541_battery_platform_data bq27541_pdata = {
	.led_callback		= battery_led_callback,
};

#if 0
static int kc1_phydetect_setting[] = {
/*0*/	FCC_2500mA | PCC_150mA | TC_150mA,
/*1*/	DC_ICL_1800mA | USBHC_ICL_1800mA,
/*2*/	SUS_CTRL_BY_REGISTER | BAT_TO_SYS_NORMAL | VFLT_PLUS_200mV | AICL_ENABLE | AIC_TH_4200mV | USB_IN_FIRST | BAT_OV_END_CHARGE,
/*3*/	PRE_CHG_VOLTAGE_THRESHOLD_3_0 | FLOAT_VOLTAGE_4_2_0,
/*detect by omap*/
/*4*/	AUTOMATIC_RECHARGE_ENABLE | CURRENT_TERMINATION_ENABLE | BMD_VIA_THERM_IO | AUTO_RECHARGE_100mV | APSD_DISABLE |
		NC_APSD_DISABLE | SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO,
/*5*/	STAT_ACTIVE_LOW | STAT_CHARGEING_STATE | STAT_DISABLE | NC_INPUT_HC_SETTING | CC_TIMEOUT_DISABLED | PC_TIMEOUT_DISABLED,
/*6*/	LED_BLINK_DISABLE | CHARGE_EN_I2C_0 | USB_HC_CONTROL_BY_REGISTER | USB_HC_TRI_STATE | CHARGER_ERROR_NO_IRQ |
		APSD_DONE_IRQ | DCIN_INPUT_PRE_BIAS_ENABLE,
/*7*/	0X80 | MIN_SYS_3_4_5_V | THERM_MONITOR_VDDCAP | SOFT_COLD_NO_RESPONSE | SOFT_HOT_NO_RESPONSE,
/*8*/	INOK_OPERATION | USB_2 | VFLT_MINUS_60mV | PC_TO_FC_THRESHOLD_ENABLE | HARD_TEMP_CHARGE_SUSPEND | INOK_ACTIVE_LOW ,
/*9*/	RID_DISABLE_OTG_I2C_CONTROL | OTG_PIN_ACTIVE_HIGH | LOW_BAT_VOLTAGE_3_5_8_V,
/*a*/	CCC_700mA | DTRT_130C | OTG_CURRENT_LIMIT_500mA | OTG_BAT_UVLO_THRES_2_7_V,
/*b*/	0x61,
/*c*/	TEMP_OUTSIDE_COLD_HOT_HARD_LIMIITS_TRIGGER_IRQ | TEMP_OUTSIDE_COLD_HOT_SOFT_LIMIITS_TRIGGER_IRQ |
		USB_OVER_VOLTAGE_TRIGGER_IRQ | USB_UNDER_VOLTAGE_TRIGGER_IRQ | AICL_COMPLETE_TRIGGER_IRQ,
/*d*/	CHARGE_TIMEOUT_TRIGGER_IRQ | TERMINATION_OR_TAPER_CHARGING_TRIGGER_IRQ | FAST_CHARGING_TRIGGER_IRQ | INOK_TRIGGER_IRQ |
		MISSING_BATTERY_TRIGGER_IRQ | LOW_BATTERY_TRIGGER_IRQ,
};

static int kc1_chargerdetect_setting[] = {
// [0] FCC_2500mA | PCC_150mA | TC_250mA
	(7<<5) | (1<<3) | (5),
// [1] DC_ICL_1800mA | USBHC_ICL_1800mA
	(6<<4) | (6),
// [2] SUS_CTRL_BY_REGISTER | BAT_TO_SYS_NORMAL | VFLT_PLUS_200mV | AICL_ENABLE | AIC_TH_4200mV | USB_IN_FIRST | BAT_OV_END_CHARGE
	(1<<7) | (0) | (1<<5) | (1<<4) | (0) | (1<<2) | (1<<1),
// [3] PRE_CHG_VOLTAGE_THRESHOLD_3_0 | FLOAT_VOLTAGE_4_2_0
	(3<<6) | (35),
// [4] AUTOMATIC_RECHARGE_ENABLE | CURRENT_TERMINATION_ENABLE | BMD_VIA_THERM_IO | AUTO_RECHARGE_100mV | APSD_ENABLE | NC_APSD_ENABLE | SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO
	(0) | (0) | (3<<4) | (1<<3) | (1<<2) | (1<<1) | (0),
// [5] STAT_ACTIVE_LOW | STAT_CHARGEING_STATE | STAT_ENABLE | NC_INPUT_HC_SETTING | CC_TIMEOUT_764MIN | PC_TIMEOUT_48MIN
	(0) | (0) | (0) | (1<<4) | (1<<2) | (0),
// [6] LED_BLINK_DISABLE | EN_PIN_ACTIVE_LOW | USB_HC_CONTROL_BY_PIN | USB_HC_DUAL_STATE | CHARGER_ERROR_NO_IRQ | APSD_DONE_IRQ | DCIN_INPUT_PRE_BIAS_ENABLE
	(0) | (3<<5) | (1<<4) | (1<<3) | (0) | (1<<1) | (1),
// [7] 0x80 | MIN_SYS_3_4_5_V | THERM_MONITOR_VDDCAP | THERM_MONITOR_ENABLE | SOFT_COLD_CC_FV_COMPENSATION | SOFT_HOT_CC_FV_COMPENSATION
	(0x80) | (0) | (1<<5) | (0) | (3<<2) | (3),
// [8] INOK_OPERATION | USB_2 | VFLT_MINUS_240mV | HARD_TEMP_CHARGE_SUSPEND | PC_TO_FC_THRESHOLD_ENABLE | INOK_ACTIVE_LOW
	(0) | (0) | (3<<3) | (0) | (0) | (0),
// [9] RID_DISABLE_OTG_I2C_CONTROL | OTG_PIN_ACTIVE_HIGH | LOW_BAT_VOLTAGE_3_4_6_V
	(0) | (0) | (14),
// [A] CCC_700mA | DTRT_130C | OTG_CURRENT_LIMIT_500mA | OTG_BAT_UVLO_THRES_2_7_V
	(1<<6) | (3<<4) | (2<<2) | (0), 
// [B]
	(0x61),
// [C] AICL_COMPLETE_TRIGGER_IRQ
	(1<<1),
// [D] INOK_TRIGGER_IRQ | LOW_BATTERY_TRIGGER_IRQ
	(1<<2) | (1),
};
#endif

#ifdef CONFIG_OTTER
static int chargerdetect_setting[] = {
// [0] FCC_2500mA | PCC_150mA | TC_150mA
	(7<<5) | (1<<3) | (3),
// [1] DC_ICL_1800mA | USBHC_ICL_1800mA
	(6<<4) | (6),
// [2] SUS_CTRL_BY_REGISTER | BAT_TO_SYS_NORMAL | VFLT_PLUS_200mV | AICL_ENABLE | AIC_TH_4200mV | USB_IN_FIRST | BAT_OV_END_CHARGE
	(1<<7) | (0) | (1<<5) | (1<<4) | (0) | (1<<2) | (1<<1),
// [3] PRE_CHG_VOLTAGE_THRESHOLD_3_0 | FLOAT_VOLTAGE_4_2_0
	(3<<6) | (35),
// [4] AUTOMATIC_RECHARGE_ENABLE | CURRENT_TERMINATION_ENABLE | BMD_VIA_THERM_IO | AUTO_RECHARGE_100mV | APSD_ENABLE | NC_APSD_ENABLE | SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO
	(0) | (0) | (3<<4) | (1<<3) | (1<<2) | (1<<1) | (0),
// [5] STAT_ACTIVE_LOW | STAT_CHARGEING_STATE | STAT_ENABLE | NC_INPUT_HC_SETTING | CC_TIMEOUT_764MIN | PC_TIMEOUT_48MIN
	(0) | (0) | (0) | (1<<4) | (1<<2) | (0),
// [6] LED_BLINK_DISABLE | EN_PIN_ACTIVE_LOW | USB_HC_CONTROL_BY_PIN | USB_HC_DUAL_STATE | CHARGER_ERROR_NO_IRQ | APSD_DONE_IRQ | DCIN_INPUT_PRE_BIAS_ENABLE
	(0) | (3<<5) | (1<<4) | (1<<3) | (0) | (1<<1) | (1),
// [7] 0x80 | MIN_SYS_3_4_5_V | THERM_MONITOR_VDDCAP | THERM_MONITOR_ENABLE | SOFT_COLD_CC_FV_COMPENSATION | SOFT_HOT_FV_COMPENSATION
	(0x80) | (0) | (1<<5) | (0) | (3<<2) | (3),
// [8] INOK_OPERATION | USB_2 | VFLT_MINUS_240mV | HARD_TEMP_CHARGE_SUSPEND | PC_TO_FC_THRESHOLD_ENABLE | INOK_ACTIVE_LOW
	(0) | (0) | (3<<3) | (0) | (0) | (0),
// [9] RID_DISABLE_OTG_I2C_CONTROL | OTG_PIN_ACTIVE_HIGH | LOW_BAT_VOLTAGE_3_4_6_V
	(0) | (0) | (14),
// [A] CCC_700mA | DTRT_130C | OTG_CURRENT_LIMIT_500mA | OTG_BAT_UVLO_THRES_2_7_V
	(1<<6) | (3<<4) | (2<<2) | (0), 
// [B]
	(0xF5),
// [C] AICL_COMPLETE_TRIGGER_IRQ
	(1<<1),
// [D] INOK_TRIGGER_IRQ | LOW_BATTERY_TRIGGER_IRQ
	(1<<2) | (1),
};
#else
static int chargerdetect_setting[] = {
/*0*/	FCC_2500mA | PCC_150mA | TC_150mA,
/*1*/	DC_ICL_1800mA | USBHC_ICL_1800mA,
/*2*/	SUS_CTRL_BY_REGISTER | BAT_TO_SYS_NORMAL | VFLT_PLUS_200mV | AICL_ENABLE | AIC_TH_4200mV | USB_IN_FIRST | BAT_OV_END_CHARGE,
/*3*/	PRE_CHG_VOLTAGE_THRESHOLD_3_0 | FLOAT_VOLTAGE_4_2_0,
/*4*/	AUTOMATIC_RECHARGE_ENABLE | CURRENT_TERMINATION_ENABLE | BMD_VIA_THERM_IO | AUTO_RECHARGE_100mV | APSD_ENABLE | NC_APSD_ENABLE |
		SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO,
/*5*/	STAT_ACTIVE_LOW | STAT_CHARGEING_STATE | STAT_ENABLE | NC_INPUT_HC_SETTING | CC_TIMEOUT_764MIN | PC_TIMEOUT_48MIN,
/*6*/	LED_BLINK_DISABLE | EN_PIN_ACTIVE_LOW | USB_HC_CONTROL_BY_PIN | USB_HC_DUAL_STATE | CHARGER_ERROR_NO_IRQ | APSD_DONE_IRQ | DCIN_INPUT_PRE_BIAS_ENABLE,
/*7*/	0x80 | MIN_SYS_3_4_5_V | THERM_MONITOR_VDDCAP | THERM_MONITOR_ENABLE | SOFT_COLD_CC_FV_COMPENSATION | SOFT_HOT_FV_COMPENSATION,
/*8*/	INOK_OPERATION | USB_2 | VFLT_MINUS_240mV | HARD_TEMP_CHARGE_SUSPEND | PC_TO_FC_THRESHOLD_ENABLE | INOK_ACTIVE_LOW,
/*9*/	RID_DISABLE_OTG_I2C_CONTROL | OTG_PIN_ACTIVE_HIGH | LOW_BAT_VOLTAGE_3_4_6_V,
/*a*/	CCC_700mA | DTRT_130C | OTG_CURRENT_LIMIT_500mA | OTG_BAT_UVLO_THRES_2_7_V,
/*b*/	0xa6,
/*c*/	AICL_COMPLETE_TRIGGER_IRQ,
/*d*/	INOK_TRIGGER_IRQ | LOW_BATTERY_TRIGGER_IRQ,
};
#endif

static struct summit_smb347_platform_data smb347_pdata = {
	.mbid			= 0,
	.pin_en			= 101,
	.pin_en_name		= "CHARGE-EN",
	.pin_susp		= 155,
	.pin_susp_name		= "CHARGE-SUSP",
	.initial_max_aicl	= 1800,
	.initial_pre_max_aicl	= 1800,
	.initial_charge_current	= 2000,
	.irq_trigger_falling	= true,
	.irq_set_awake		= true,
	.charger_setting	= chargerdetect_setting,
	.charger_setting_skip_flag = true,
	.led_callback		= battery_led_callback,
};

static struct i2c_board_info __initdata sdp4430_i2c_boardinfo_dvt[] = {
#ifdef CONFIG_BATTERY_BQ27541_Q
	{ I2C_BOARD_INFO("bq27541", 0x55), .platform_data = &bq27541_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_boardinfo[] = {
#ifdef CONFIG_BATTERY_BQ27541_Q
	{ I2C_BOARD_INFO("bq27541", 0x55), .platform_data = &bq27541_pdata, },
#endif
#ifdef CONFIG_SUMMIT_SMB347_Q
	{ I2C_BOARD_INFO("summit_smb347", 0x6), .irq = OMAP_GPIO_IRQ(OMAP4_CHARGER_IRQ), .platform_data = &smb347_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_2_boardinfo[] = {
#ifdef CONFIG_TOUCHSCREEN_ILITEK
	{ I2C_BOARD_INFO("ilitek_i2c", 0x41), .irq = OMAP_GPIO_IRQ(OMAP4_TOUCH_IRQ_1), },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_3_boardinfo[] = {
#ifdef CONFIG_SND_SOC_TLV320AIC31XX
	{ I2C_BOARD_INFO("tlv320aic31xx", 0x18), .platform_data = &aic31xx_codec_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo[] = {
#ifdef CONFIG_INPUT_STK_ALS22x7
	{ I2C_BOARD_INFO("stk_als22x7", 0x10), },
#endif
#ifdef CONFIG_SENSORS_LM75
	{ I2C_BOARD_INFO("tmp105", 0x49), .platform_data = &lm75_pdata, },
#endif
#ifdef CONFIG_INPUT_BMA250
	{ I2C_BOARD_INFO("bma250", 0x18), .platform_data = &bma250_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo_c1c[] = {
#ifdef CONFIG_INPUT_STK_ALS22x7
	{ I2C_BOARD_INFO("stk_als22x7", 0x10), },
#endif
#ifdef CONFIG_SENSORS_LM75
	{ I2C_BOARD_INFO("lm75", 0x48), },
#endif
#ifdef CONFIG_INPUT_BMA250
	{ I2C_BOARD_INFO("bma250", 0x18), .platform_data = &bma250_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo_dvt[] = {
#ifdef CONFIG_SUMMIT_SMB347_Q
	{ I2C_BOARD_INFO("summit_smb347", 0x6), .irq = OMAP_GPIO_IRQ(OMAP4_CHARGER_IRQ), .platform_data = &smb347_pdata, },
#endif    
#ifdef CONFIG_INPUT_STK_ALS22x7
	{ I2C_BOARD_INFO("stk_als22x7", 0x10), },
#endif
#ifdef CONFIG_SENSORS_LM75
	{ I2C_BOARD_INFO("tmp105", 0x49), .platform_data = &lm75_pdata, },
#endif
#ifdef CONFIG_INPUT_BMA250
	{ I2C_BOARD_INFO("bma250", 0x18), .platform_data = &bma250_pdata, },
#endif
};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo_pvt[] = {
#ifdef CONFIG_SUMMIT_SMB347_Q
	{ I2C_BOARD_INFO("summit_smb347", 0x6), .irq = OMAP_GPIO_IRQ(OMAP4_CHARGER_IRQ), .platform_data = &smb347_pdata, },
#endif    
#ifdef CONFIG_INPUT_STK_ALS22x7
	{ I2C_BOARD_INFO("stk_als22x7", 0x10), },
#endif
#ifdef CONFIG_INPUT_BMA250
	{ I2C_BOARD_INFO("bma250", 0x18), .platform_data = &bma250_pdata, },
#endif
#ifdef CONFIG_SENSORS_LM75
	{ I2C_BOARD_INFO("tmp105", 0x48), .platform_data = &lm75_pdata, },
#endif
};

static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_2_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_3_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_4_bus_pdata;

static void __init omap_i2c_hwspinlock_init(int bus_id, int spinlock_id,
				struct omap_i2c_bus_board_data *pdata)
{
	/* spinlock_id should be -1 for a generic lock request */
	if (spinlock_id < 0)
		pdata->handle = hwspin_lock_request(USE_MUTEX_LOCK);
	else
		pdata->handle = hwspin_lock_request_specific(spinlock_id, USE_MUTEX_LOCK);

	if (pdata->handle != NULL) {
		pdata->hwspin_lock_timeout = hwspin_lock_timeout;
		pdata->hwspin_unlock = hwspin_unlock;
	} else {
		pr_err("I2C hwspinlock request failed for bus %d\n", bus_id);
	}
}

static struct regulator_init_data sdp4430_vdac = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.always_on		= true,
	},
};

static struct regulator_consumer_supply emmc_supply[] = {
	REGULATOR_SUPPLY("vmmc", "omap_hsmmc.1"),
};

static struct regulator_init_data sdp4430_vaux1 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.always_on		= true,
	},
	.num_consumer_supplies		= 1,
	.consumer_supplies		= emmc_supply,
};

static struct regulator_consumer_supply gsensor_supply[] = {
	{ .supply = "g-sensor-pwr", },
};

static struct regulator_init_data sdp4430_vaux2 = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.always_on		= true,
	},
    .num_consumer_supplies = 1,
    .consumer_supplies = gsensor_supply,
};

static struct regulator_consumer_supply vaux3_supply[] = {
	{ .supply = "vaux3", },
};

static struct regulator_init_data sdp4430_vaux3 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
	},
	.num_consumer_supplies		= 1,
	.consumer_supplies		= vaux3_supply,
};

static struct regulator_consumer_supply vmmc_supply[] = {
	{ .supply = "vmmc", .dev_name = "omap_hsmmc.0", },
};

/* VMMC1 for MMC1 card */
static struct regulator_init_data sdp4430_vmmc = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 3000000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
	},
	.num_consumer_supplies		= 1,
	.consumer_supplies		= vmmc_supply,
};

static struct regulator_init_data sdp4430_vpp = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 2500000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.initial_state          = PM_SUSPEND_MEM,
	},
};

static struct regulator_consumer_supply audio_supply[] = {
	{ .supply = "audio-pwr", },
};

static struct regulator_init_data sdp4430_vusim = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = audio_supply,
};

static struct regulator_init_data sdp4430_vana = {
	.constraints = {
		.min_uV			= 2100000,
		.max_uV			= 2100000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.always_on		= true,
	},
};

static struct regulator_consumer_supply sdp4430_vcxio_supply[] = {
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dss"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi.0"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi.1"),
};

static struct regulator_init_data sdp4430_vcxio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem		= { .enabled = false, .disabled = true, },
		.always_on		= true,
	},
	.num_consumer_supplies	= ARRAY_SIZE(sdp4430_vcxio_supply),
	.consumer_supplies	= sdp4430_vcxio_supply,
};

static struct regulator_init_data sdp4430_clk32kg = {
	.constraints = {
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
		.always_on		= true,
	},
};

static struct regulator_init_data sdp4430_v2v1 = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
		.always_on		= true,
		.state_mem = {
			.disabled	= true,
		},
		.initial_state		= PM_SUSPEND_MEM,
	},
};

static struct twl4030_platform_data sdp4430_twldata = {
	/* TWL6030 regulators at OMAP443X/4460 based SOMs */
	.vdac		= &sdp4430_vdac,
	.vaux1		= &sdp4430_vaux1,
	.vaux2		= &sdp4430_vaux2,
	.vaux3		= &sdp4430_vaux3,
	.vmmc		= &sdp4430_vmmc,
	.vpp		= &sdp4430_vpp,
	.vusim		= &sdp4430_vusim,
	.vana		= &sdp4430_vana,
	.vcxio		= &sdp4430_vcxio,

	/* TWL6030/6032 common resources */
	.clk32kg	= &sdp4430_clk32kg,

	/* SMPS */
	.v2v1		= &sdp4430_v2v1,
};

static int __init omap4_i2c_init(void)
{
	int err;

	smb347_pdata.mbid = quanta_get_mbid();

	omap_i2c_hwspinlock_init(1, 0, &sdp4430_i2c_bus_pdata);
	omap_i2c_hwspinlock_init(2, 1, &sdp4430_i2c_2_bus_pdata);
	omap_i2c_hwspinlock_init(3, 2, &sdp4430_i2c_3_bus_pdata);
	omap_i2c_hwspinlock_init(4, 3, &sdp4430_i2c_4_bus_pdata);

	omap_register_i2c_bus_board_data(1, &sdp4430_i2c_bus_pdata);
	omap_register_i2c_bus_board_data(2, &sdp4430_i2c_2_bus_pdata);
	omap_register_i2c_bus_board_data(3, &sdp4430_i2c_3_bus_pdata);
	omap_register_i2c_bus_board_data(4, &sdp4430_i2c_4_bus_pdata);

	omap4_pmic_get_config(&sdp4430_twldata, TWL_COMMON_PDATA_USB | TWL_COMMON_PDATA_MADC,
		TWL_COMMON_REGULATOR_VUSB
		);
#if 0
		TWL_COMMON_REGULATOR_VDAC |
		TWL_COMMON_REGULATOR_VAUX2 |
		TWL_COMMON_REGULATOR_VAUX3 |
		TWL_COMMON_REGULATOR_VMMC |
		TWL_COMMON_REGULATOR_VPP |
		TWL_COMMON_REGULATOR_VANA |
		TWL_COMMON_REGULATOR_VCXIO |
		TWL_COMMON_REGULATOR_VUSB |
		TWL_COMMON_REGULATOR_CLK32KG |
		TWL_COMMON_REGULATOR_V1V8 |
		TWL_COMMON_REGULATOR_V2V1);
#endif
	omap_pmic_init(1, 400, "twl6030", OMAP44XX_IRQ_SYS_1N, &sdp4430_twldata);

	/*
	 * Phoenix Audio IC needs I2C1 to
	 * start with 400 KHz or less
	 */
        if (quanta_mbid < 0x04) {
		err = i2c_register_board_info(1, sdp4430_i2c_boardinfo, ARRAY_SIZE(sdp4430_i2c_boardinfo));
        }
	// DVT
        else {
		err = i2c_register_board_info(1, sdp4430_i2c_boardinfo_dvt, ARRAY_SIZE(sdp4430_i2c_boardinfo_dvt));
        }
	if (err)
	  return err;

	omap_register_i2c_bus(2, 400, sdp4430_i2c_2_boardinfo, ARRAY_SIZE(sdp4430_i2c_2_boardinfo));
	omap_register_i2c_bus(3, 400, sdp4430_i2c_3_boardinfo, ARRAY_SIZE(sdp4430_i2c_3_boardinfo));
	if (quanta_mbid<0x02) {
		omap_register_i2c_bus(4, 400, sdp4430_i2c_4_boardinfo_c1c, ARRAY_SIZE(sdp4430_i2c_4_boardinfo_c1c));
	}
	else if (quanta_mbid<0x04) {
		omap_register_i2c_bus(4, 400, sdp4430_i2c_4_boardinfo, ARRAY_SIZE(sdp4430_i2c_4_boardinfo));
	}
	// DVT
	else if (quanta_mbid<0x06) {
		pr_info("quanta_mbid < 0x06\n");
		omap_register_i2c_bus(4, 400, sdp4430_i2c_4_boardinfo_dvt, ARRAY_SIZE(sdp4430_i2c_4_boardinfo_dvt));
	}
	// PVT
	else {
		pr_info("quanta_mbid >= 0x06\n");
		omap_register_i2c_bus(4, 400, sdp4430_i2c_4_boardinfo_pvt, ARRAY_SIZE(sdp4430_i2c_4_boardinfo_pvt));
	}

	regulator_has_full_constraints();

	return 0;
}


static void __init omap4_display_init(void)
{
	void __iomem *phymux_base = NULL;
	u32 val;

	phymux_base = ioremap(0x4A100000, 0x1000);

	/* GPIOs 101, 102 */
	val = __raw_readl(phymux_base + 0x90);
	val = (val & 0xFFF8FFF8) | 0x00030003;
	__raw_writel(val, phymux_base + 0x90);

	/* GPIOs 103, 104 */
	val = __raw_readl(phymux_base + 0x94);
	val = (val & 0xFFF8FFF8) | 0x00030003;
	__raw_writel(val, phymux_base + 0x94);

	iounmap(phymux_base);
}

static bool enable_suspend_off = true;
module_param(enable_suspend_off, bool, S_IRUGO);

/******** END I2C BOARD INIT ********/

#if defined(CONFIG_TOUCHSCREEN_ILITEK)
static void __init omap_ilitek_init(void)
{
	//printk("~~~~~~~~%s\n", __func__);
	omap_mux_init_signal("dpm_emu7.gpio_18", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_NONE);
        
        if (gpio_request(OMAP4_TOUCH_RESET_GPIO , "ilitek_reset_gpio_18") <0 ){
		pr_err("Touch IRQ reset request failed\n");
                return;
	}
        gpio_direction_output(OMAP4_TOUCH_RESET_GPIO, 0);
        gpio_set_value(OMAP4_TOUCH_RESET_GPIO, 1);

	if (gpio_request(OMAP4_TOUCH_IRQ_1, "Touch IRQ") < 0) {
		pr_err("Touch IRQ GPIO request failed\n");
		return;
	}
	gpio_direction_input(OMAP4_TOUCH_IRQ_1);

}
#endif //CONFIG_TOUCHSCREEN_ILITEK

/* BEGIN SERIAL */
static struct omap_device_pad tablet_uart3_pads[] __initdata = {
	{
		.name	= "uart3_cts_rctx.uart3_cts_rctx",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE7,
	},
	{
		.name	= "uart3_rts_sd.uart3_rts_sd",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE7,
	},
	{
		.name	= "uart3_tx_irtx.uart3_tx_irtx",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart3_rx_irrx.uart3_rx_irrx",
		.flags	= OMAP_DEVICE_PAD_REMUX | OMAP_DEVICE_PAD_WAKEUP,
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
		.idle	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
	},
};

static struct omap_uart_port_info tablet_uart_info __initdata = {
	.dma_enabled = 0,
	.autosuspend_timeout = -1,
};

void __init board_serial_init(void)
{
//	omap_serial_init_port(&uart3_board_data, &tablet_uart_info);
}
/* END SERIAL */

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
#if 0
    OMAP4_MUX(ABE_DMIC_CLK1,OMAP_MUX_MODE3 | OMAP_OFF_EN),//+5V ADO_SPEAK_ENABLE
    OMAP4_MUX(GPMC_AD0, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT0
    OMAP4_MUX(GPMC_AD1, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT1
    OMAP4_MUX(GPMC_AD2, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT2
    OMAP4_MUX(GPMC_AD3, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT3
    OMAP4_MUX(GPMC_AD4, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT4
    OMAP4_MUX(GPMC_AD5, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT5
    OMAP4_MUX(GPMC_AD6, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT6
    OMAP4_MUX(GPMC_AD7, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // SDMMC2_DAT7
    OMAP4_MUX(GPMC_AD9, OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PULL_ENA | OMAP_PULL_UP),
    OMAP4_MUX(GPMC_AD10, OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PULL_ENA | OMAP_PULL_UP),
    OMAP4_MUX(GPMC_AD11, OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PULL_ENA),
    OMAP4_MUX(GPMC_AD12, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 36
    OMAP4_MUX(GPMC_AD13,OMAP_MUX_MODE3 | OMAP_OFF_EN),//OMAP_RGB_SHTDN
    OMAP4_MUX(GPMC_AD14, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 38
    OMAP4_MUX(GPMC_A18, OMAP_MUX_MODE3), // gpio 42
    OMAP4_MUX(GPMC_A19, OMAP_MUX_MODE3 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 43
    OMAP4_MUX(GPMC_A20, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 44
    OMAP4_MUX(GPMC_A21, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 45
    OMAP4_MUX(GPMC_A23,OMAP_MUX_MODE3 | OMAP_OFF_EN),//OMAP_3V_ENABLE ,LCDVCC
    OMAP4_MUX(GPMC_A24, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 48
    OMAP4_MUX(GPMC_A25, OMAP_MUX_MODE3 | OMAP_PIN_INPUT), // gpio 49
    OMAP4_MUX(GPMC_CLK, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 55
    OMAP4_MUX(GPMC_NADV_ALE, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 56
    OMAP4_MUX(GPMC_NWE, OMAP_MUX_MODE1 | OMAP_PIN_INPUT | OMAP_OFF_EN | OMAP_OFFOUT_EN), // sdmmc2_cmd
    OMAP4_MUX(GPMC_NBE0_CLE, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 59
    OMAP4_MUX(GPMC_WAIT0, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 61
    OMAP4_MUX(GPMC_WAIT1, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 62
    OMAP4_MUX(C2C_DATA12, OMAP_MUX_MODE3 | OMAP_OFF_EN ),//Charger en pin
    OMAP4_MUX(C2C_DATA13, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 102
    OMAP4_MUX(UART4_RX, OMAP_MUX_MODE3 | OMAP_OFF_EN | OMAP_OFFOUT_VAL ),//Charger susp pin,
    OMAP4_MUX(USBB1_ULPITLL_DAT6, OMAP_MUX_MODE1 ),//Backlight GPTimer 10,
    OMAP4_MUX(C2C_DATA14, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLDOWN ),//gpio 103: MP/ENG Detect,
    OMAP4_MUX(UNIPRO_TX1, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLDOWN ),//gpio 174: mbid1,
    OMAP4_MUX(UNIPRO_TY1, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLDOWN ),//gpio 173: mbid0,
    OMAP4_MUX(UNIPRO_RX1, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLDOWN ),//gpio 177: mbid3,
    OMAP4_MUX(UNIPRO_RY1, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLDOWN ),//gpio 178: mbid2,
    OMAP4_MUX(UNIPRO_RX0, OMAP_MUX_MODE3 | OMAP_INPUT_EN ),//gpio 175: LCDid1,
    OMAP4_MUX(UNIPRO_RY0, OMAP_MUX_MODE3 | OMAP_INPUT_EN ),//gpio 176: LCDid0,
    OMAP4_MUX(GPMC_NCS0, OMAP_MUX_MODE3 | OMAP_INPUT_EN ),//gpio 50: TPid0,
    OMAP4_MUX(GPMC_NCS1, OMAP_MUX_MODE3 | OMAP_INPUT_EN ),//gpio 51: TPid1,
    OMAP4_MUX(UNIPRO_TX0, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLUP ),//gpio 171: Wifi IP select,
    OMAP4_MUX(UNIPRO_TY0, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLUP ),//gpio 172: Wifi IP select,
    /* OMAP4_MUX(C2C_DATA11, OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_WAKEUP_EN |OMAP_PIN_OFF_INPUT_PULLUP),//Power button, same as GPMC_WAIT2*/
    /* OMAP4_MUX(C2C_DATA15, OMAP_MUX_MODE7 | OMAP_PULL_ENA),//Power button, same as GPMC_NCS7 */
    OMAP4_MUX(HDMI_HPD, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // HDMI_HDP
    OMAP4_MUX(HDMI_CEC, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // HDMI_CEC
    OMAP4_MUX(HDMI_DDC_SCL, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // HDMI_DDC_SCL
    OMAP4_MUX(HDMI_DDC_SDA, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // HDMI_DDC_SDA
    OMAP4_MUX(CSI21_DX0, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DX0
    OMAP4_MUX(CSI21_DY0, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DY0
    OMAP4_MUX(CSI21_DX1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DX1
    OMAP4_MUX(CSI21_DY1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DY1
    OMAP4_MUX(CSI21_DX2, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DX2
    OMAP4_MUX(CSI21_DY2, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI21_DY2
    OMAP4_MUX(CSI22_DX0, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI22_DX0
    OMAP4_MUX(CSI22_DY0, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI22_DY0
    OMAP4_MUX(CSI22_DX1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI22_DX1
    OMAP4_MUX(CSI22_DY1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CSI22_DY1
    OMAP4_MUX(CAM_SHUTTER, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CAM_SHUTTER
    OMAP4_MUX(CAM_STROBE, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // CAM_STROBE
    OMAP4_MUX(CAM_GLOBALRESET, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 83
    OMAP4_MUX(USBB1_ULPITLL_CLK, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 84
    OMAP4_MUX(USBB1_ULPITLL_STP, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 85
    OMAP4_MUX(USBB1_ULPITLL_DIR, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 86
    OMAP4_MUX(USBB1_ULPITLL_NXT, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 87
    OMAP4_MUX(USBB1_ULPITLL_DAT0, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 88
    OMAP4_MUX(USBB1_ULPITLL_DAT1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 89
    OMAP4_MUX(USBB1_ULPITLL_DAT2, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 90
    OMAP4_MUX(USBB1_ULPITLL_DAT3, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 91
    OMAP4_MUX(USBB1_ULPITLL_DAT4, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 92
    OMAP4_MUX(USBB1_ULPITLL_DAT5, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 93
    OMAP4_MUX(USBB1_ULPITLL_DAT7, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // USBB1_ULPITLL_DAT7
    OMAP4_MUX(USBB1_HSIC_DATA, OMAP_MUX_MODE7), // USBB1_HSIC_DATA
    OMAP4_MUX(USBB1_HSIC_STROBE, OMAP_MUX_MODE7), // USBB1_HSIC_STROBE
    OMAP4_MUX(USBC1_ICUSB_DP, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // USBC1_ICUSB_DP
    OMAP4_MUX(USBC1_ICUSB_DM, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // USBC1_ICUSB_DM
    OMAP4_MUX(SDMMC1_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN), // SDMMC1_CLK
    OMAP4_MUX(ABE_MCBSP2_CLKX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP2_CLKX
    OMAP4_MUX(ABE_MCBSP2_DR, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP2_DR
    OMAP4_MUX(ABE_MCBSP2_DX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP2_DX
    OMAP4_MUX(ABE_MCBSP2_FSX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP2_FSX
    OMAP4_MUX(ABE_MCBSP1_CLKX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP1_CLKX
    OMAP4_MUX(ABE_MCBSP1_DR, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP1_DR
    OMAP4_MUX(ABE_MCBSP1_DX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP1_DX
    OMAP4_MUX(ABE_MCBSP1_FSX, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_MCBSP1_FSX
    OMAP4_MUX(ABE_PDM_UL_DATA, OMAP_MUX_MODE1 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_OUTPUT_LOW), // ABE_PDM_UL_DATA
    OMAP4_MUX(ABE_DMIC_DIN1, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 120
    OMAP4_MUX(ABE_DMIC_DIN2, OMAP_MUX_MODE3 | OMAP_PULL_ENA), // gpio 121
    OMAP4_MUX(ABE_DMIC_DIN3, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // ABE_DMIC_DIN3
    OMAP4_MUX(UART2_CTS, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // UART2_CTS
    OMAP4_MUX(UART2_RTS, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // UART2_RTS
    OMAP4_MUX(UART2_RX, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // UART2_RX
    OMAP4_MUX(UART2_TX, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // UART2_TX
    OMAP4_MUX(HDQ_SIO, OMAP_MUX_MODE7), // gpio 127
    OMAP4_MUX(MCSPI1_CS2, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 139
    OMAP4_MUX(MCSPI1_CS3, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 140
    OMAP4_MUX(UART4_TX, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // gpio 156
    OMAP4_MUX(USBB2_ULPITLL_CLK, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // gpio 157
    OMAP4_MUX(USBB2_HSIC_DATA, OMAP_MUX_MODE7), // gpio 169
    OMAP4_MUX(USBB2_HSIC_STROBE, OMAP_MUX_MODE7), // gpio 170
    OMAP4_MUX(UNIPRO_TX0, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN), // gpio 171,for factory rndis
    OMAP4_MUX(UNIPRO_TY0, OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN), // gpio 172,for factory rndis
    OMAP4_MUX(FREF_CLK1_OUT, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // fref_clk1_out
    OMAP4_MUX(DPM_EMU2, OMAP_MUX_MODE7 | OMAP_PULL_ENA), // dpm_emu2
    OMAP4_MUX(I2C1_SCL, OMAP_MUX_MODE0 | OMAP_INPUT_EN), // I2C1_SCL
    OMAP4_MUX(I2C1_SDA, OMAP_MUX_MODE0 | OMAP_INPUT_EN), // I2C1_SDA
    OMAP4_MUX(I2C2_SCL, OMAP_MUX_MODE0 | OMAP_INPUT_EN), // I2C2_SCL
    OMAP4_MUX(I2C2_SDA, OMAP_MUX_MODE0 | OMAP_INPUT_EN), // I2C2_SDA
    OMAP4_MUX(MCSPI1_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN), // MCSPI1_CLK
    OMAP4_MUX(MCSPI1_SOMI, OMAP_MUX_MODE0), // MCSPI1_SOMI
    OMAP4_MUX(MCSPI1_SIMO, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN), // MCSPI1_SIMO
    OMAP4_MUX(MCSPI1_CS0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN), // MCSPI1_CS0
    OMAP4_MUX(MCSPI1_CS1, OMAP_MUX_MODE7 | OMAP_INPUT_EN), // MCSPI1_CS1

    OMAP4_MUX(MCSPI4_SOMI, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP), // MCSPI4_SOMI
    OMAP4_MUX(DPM_EMU0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP), // DPM_EMU0
    OMAP4_MUX(DPM_EMU1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP), // DPM_EMU1
    OMAP4_MUX(DPM_EMU7, OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN), // DISPC2_HSYNC

    OMAP4_MUX(GPMC_NCS7, OMAP_MUX_MODE7 | OMAP_PULL_ENA),				//gpmc_ncs7=GPIO_104-OMAP_TP_RESET
    OMAP4_MUX(GPMC_WAIT2, OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_WAKEUP_EN |OMAP_PIN_OFF_INPUT_PULLUP),		//GPIO_100-ON_BUTTON
#endif
    { .reg_offset = OMAP_MUX_TERMINATOR },
};
static struct omap_board_mux board_mux_wkup[] __initdata = {
#if 0
    OMAP4_MUX(SIM_IO, OMAP_MUX_MODE7 | OMAP_PULL_ENA),
    OMAP4_MUX(SIM_CLK, OMAP_MUX_MODE7 | OMAP_PULL_ENA),
    OMAP4_MUX(SIM_RESET, OMAP_MUX_MODE7 | OMAP_PULL_ENA),
    OMAP4_MUX(SIM_CD, OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP),
    OMAP4_MUX(SIM_PWRCTRL, OMAP_MUX_MODE7 | OMAP_PULL_ENA),
    OMAP4_MUX(FREF_CLK3_REQ, OMAP_MUX_MODE7 | OMAP_PULL_ENA),
    OMAP4_MUX(SR_SCL, OMAP_MUX_MODE0 | OMAP_INPUT_EN),		//No need to use PAD PU
    OMAP4_MUX(SR_SDA, OMAP_MUX_MODE0 | OMAP_INPUT_EN),		//No need to use PAD PU
    OMAP4_MUX(FREF_CLK3_OUT, OMAP_MUX_MODE7 | OMAP_PULL_ENA),		//FREF_CLK3_OUT
    OMAP4_MUX(SYS_PWR_REQ, OMAP_MUX_MODE0 | OMAP_PULL_ENA),		// SYS_PWR_REQ
    OMAP4_MUX(SYS_BOOT7, OMAP_MUX_MODE3 | OMAP_INPUT_EN),		// GPIO_WK10
#endif
    { .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#define board_mux_wkup	NULL
#endif

static void enable_rtc_gpio(void){
	/* To access twl registers we enable gpio6
	 * we need this so the RTC driver can work.
	 */
	gpio_request(TWL6030_RTC_GPIO, "h_SYS_DRM_MSEC");
	gpio_direction_output(TWL6030_RTC_GPIO, 1);

	omap_mux_init_signal("fref_clk0_out.gpio_wk6", \
		OMAP_PIN_OUTPUT | OMAP_PIN_OFF_NONE);
	return;
}

static void __init omap4_kc1_wifi_mux_init(void)
{
	omap_mux_init_gpio(GPIO_WIFI_IRQ, OMAP_PIN_INPUT |
				OMAP_PIN_OFF_WAKEUPENABLE);
	omap_mux_init_gpio(GPIO_WIFI_PMENA, OMAP_PIN_OUTPUT);

	omap_mux_init_signal("sdmmc5_cmd.sdmmc5_cmd",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_clk.sdmmc5_clk",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat0.sdmmc5_dat0",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat1.sdmmc5_dat1",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat2.sdmmc5_dat2",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat3.sdmmc5_dat3",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
}

static void __init omap4_kc1_wifi_init(void)
{
	omap4_kc1_wifi_mux_init();
	if (wl12xx_set_platform_data(&omap4_kc1_wlan_data))
		pr_err("Error setting wl12xx data\n");
	platform_device_register(&omap_vwlan_device);
}

#if defined(CONFIG_USB_EHCI_HCD_OMAP) || defined(CONFIG_USB_OHCI_HCD_OMAP3)
static const struct usbhs_omap_board_data usbhs_bdata __initconst = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_OHCI_PORT_MODE_PHY_6PIN_DATSE0,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
	.phy_reset  = false,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};

static void __init omap4_ehci_ohci_init(void)
{

	omap_mux_init_signal("usbb2_ulpitll_clk.gpio_157", \
		OMAP_PIN_OUTPUT | \
		OMAP_PIN_OFF_NONE);

	/* Power on the ULPI PHY */
	if (gpio_is_valid(OMAP4SDP_MDM_PWR_EN_GPIO)) {
		gpio_request(OMAP4SDP_MDM_PWR_EN_GPIO, "USBB1 PHY VMDM_3V3");
		gpio_direction_output(OMAP4SDP_MDM_PWR_EN_GPIO, 1);
	}

	usbhs_init(&usbhs_bdata);

	return;

}
#else
static void __init omap4_ehci_ohci_init(void){}
#endif

/*
 * LPDDR2 Configeration Data:
 * The memory organisation is as below :
 *	EMIF1 - CS0 -	2 Gb
 *		CS1 -	2 Gb
 *	EMIF2 - CS0 -	2 Gb
 *		CS1 -	2 Gb
 *	--------------------
 *	TOTAL -		8 Gb
 *
 * Same devices installed on EMIF1 and EMIF2
 */

#if defined(CONFIG_TI_EMIF) || defined(CONFIG_TI_EMIF_MODULE)
struct ddr_device_info lpddr2_elpida_2G_S4_x1_info = {
.type	= DDR_TYPE_LPDDR2_S4,
.density	= DDR_DENSITY_2Gb,
.io_width	= DDR_IO_WIDTH_32,
.cs1_used	= false,
.cal_resistors_per_cs = false,
.manufacturer	= "Elpida"
};
#endif

static void __init omap_kc1_init(void)
{
	int package = OMAP_PACKAGE_CBS;

	quanta_boardids();
	if (omap_rev() == OMAP4430_REV_ES1_0)
		package = OMAP_PACKAGE_CBL;

#if defined(CONFIG_TI_EMIF) || defined(CONFIG_TI_EMIF_MODULE)
	omap_emif_set_device_details(1, &lpddr2_elpida_2G_S4_x1_info,
			lpddr2_elpida_2G_S4_timings,
			ARRAY_SIZE(lpddr2_elpida_2G_S4_timings),
			&lpddr2_elpida_S4_min_tck, NULL);
	omap_emif_set_device_details(2, &lpddr2_elpida_2G_S4_x1_info,
			lpddr2_elpida_2G_S4_timings,
			ARRAY_SIZE(lpddr2_elpida_2G_S4_timings),
			&lpddr2_elpida_S4_min_tck, NULL);
#endif

	omap4_mux_init(board_mux, board_mux_wkup, package);
	omap_create_board_props();

	omap4_i2c_init();
	enable_rtc_gpio();

	omap_sdrc_init(NULL, NULL);
	omap4_twl6030_hsmmc_init(mmc);
	omap4_ehci_ohci_init();
	usb_musb_init(&musb_board_data);

	omap_init_dmm_tiler();
	omap4_display_init();

#ifdef CONFIG_ION_OMAP
	omap4_register_ion();
#endif
	omap_die_governor_register_pdata(&omap_gov_pdata);
	platform_add_devices(sdp4430_devices, ARRAY_SIZE(sdp4430_devices));

	gpio_request(0, "sysok");

	gpio_request(42, "OMAP_GPIO_ADC");
	gpio_direction_output(42, 0);

	/*For smb347*/
	//Enable charger interrupt wakeup function.
	omap_mux_init_signal("fref_clk4_req.gpio_wk7", \
		OMAP_PIN_INPUT_PULLUP|OMAP_PIN_OFF_WAKEUPENABLE|OMAP_PIN_OFF_INPUT_PULLUP);

	if(quanta_mbid>=4) {
		//EN pin
		omap_mux_init_signal("c2c_data12.gpio_101", OMAP_PIN_OUTPUT |OMAP_PIN_OFF_OUTPUT_LOW);
		//SUSP
		omap_mux_init_signal("uart4_rx.gpio_155", OMAP_PIN_OUTPUT |OMAP_PIN_OFF_OUTPUT_HIGH);
		gpio_request(101, "CHARGE-en");
		gpio_direction_output(101, 0);
		gpio_request(155, "CHARGE-SUSP");
		gpio_direction_output(155, 1);
	}
	omap4_kc1_wifi_init();

	omap4_kc1_display_init();
#if defined(CONFIG_TOUCHSCREEN_ILITEK)
	omap_ilitek_init();
#endif //CONFIG_TOUCHSCREEN_ILITEK

	gpio_request(119, "ADO_SPK_ENABLE");
	gpio_direction_output(119, 1);
	gpio_set_value(119, 1);
	gpio_request(120, "SKIPB_GPIO");
	gpio_direction_output(120, 1);
	gpio_set_value(120, 1);

	// Qunata_diagnostic 20110506 set GPIO 171 172 to be input
	omap_writew(omap_readw(0x4a10017C) | 0x011B, 0x4a10017C); // KPD_COL3
	omap_writew(omap_readw(0x4a10017C) & ~0x04, 0x4a10017C);
	 
	omap_writew(omap_readw(0x4a10017C) | 0x011B, 0x4a10017E); // KPD_COL4
	omap_writew(omap_readw(0x4a10017C) & ~0x04, 0x4a10017E);
	////////////////////////////////////////////

	omap_enable_smartreflex_on_init();
	if (enable_suspend_off)
		omap_pm_enable_off_mode();
}

static void __init omap_4430sdp_reserve(void)
{
	omap_init_ram_size();

	omap_ram_console_init(OMAP_RAM_CONSOLE_START_DEFAULT, OMAP_RAM_CONSOLE_SIZE_DEFAULT);
	omap_rproc_reserve_cma(RPROC_CMA_OMAP4);

#ifdef CONFIG_ION_OMAP
	omap4_kc1_android_display_setup(get_omap_ion_platform_data());
	omap4_ion_init();
#else
	omap4_kc1_android_display_setup(NULL);
#endif

	/* do the static reservations first */
	omap_secure_set_secure_workspace_addr(omap_smc_addr(), omap_smc_size());

	omap_reserve();
}

MACHINE_START(OMAP_4430SDP, "OMAP4430")
	.atag_offset	= 0x100,
	.reserve	= omap_4430sdp_reserve,
	.map_io		= omap4_map_io,
	.init_early	= omap4430_init_early,
	.init_irq	= gic_init_irq,
	.handle_irq	= gic_handle_irq,
	.init_machine	= omap_kc1_init,
	.timer		= &omap4_timer,
	.restart	= omap_prcm_restart,
MACHINE_END
