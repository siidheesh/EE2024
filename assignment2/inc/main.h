/*
 * main.h
 *
 *  Created on: Nov 12, 2017
 *      Author: Siidheesh
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stdio.h"
#include "math.h"
#include "string.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_rit.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_wdt.h"

#include "math.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "led7seg.h"
#include "light.h"
#include "eeprom.h"

#include "uart.h"
#include "adc.h"

#define NVIC_PRIGROUP 4

union {
	struct {
		unsigned int modeToggle : 1;
		unsigned int blink : 1;
	} bits;
	uint8_t val;
} timerFlags;

struct {
	union {
		struct {
			unsigned int home : 2;
			unsigned int mode : 2;
			unsigned int dir : 2;
			unsigned int thresh : 2;
		} bits;
		uint8_t val;
	} index;
	enum {
		GUI_HOME,
		GUI_THRESH,
		GUI_MODE,
		GUI_DIR,
		GUI_THRESH_BATT,
		GUI_THRESH_ACC,
		GUI_THRESH_OBS
	} page;
} displayGui;

struct {
	uint32_t light;
	float acc;
	uint32_t temp;
	unsigned int direction : 2;
	unsigned int orientation : 1;
	unsigned int locked : 1;
	unsigned int lockCode : 14;
	uint8_t check;
} settings;

enum {
	SYS_STATIONARY,
	SYS_FORWARD,
	SYS_REVERSE,
	SYS_TRANSIT_TO_FORWARD,
	SYS_TRANSIT_TO_REVERSE,
	SYS_TRANSIT_TO_STATIONARY,
	SYS_TRANSIT_TO_GUI,
	SYS_GUI,
	SYS_TRANSIT_OUT_OF_GUI,
	SYS_TRANSIT_TO_LOCKED,
	SYS_LOCKED
} sysMode, oldSysMode;

typedef uint32_t tick_t;

#define DISPLAY_MODE (1<<0)
#define DISPLAY_COLL (1<<1)
#define DISPLAY_BATT (1<<2)
#define DISPLAY_OBST (1<<3)
#define DISPLAY_SENS (1<<4)
#define DISPLAY_CLEAR (1<<5)
#define DISPLAY_GUI (1<<6)
#define DISPLAY_ALL (DISPLAY_MODE|DISPLAY_COLL|DISPLAY_BATT|DISPLAY_OBST|DISPLAY_SENS|DISPLAY_CLEAR)

#define TX_MODE (1<<0)
#define TX_COLL (1<<1)
#define TX_OBST (1<<2)
#define TX_BATT (1<<3)
#define TX_SENS (1<<4)
#define TX_GUI (1<<5)
#define TX_THRESH_UPDATE (1<<6)
#define TX_UNKNOWN_CMD (1<<7)
#define TX_SYSDUMP (1<<8)

#endif /* MAIN_H_ */
