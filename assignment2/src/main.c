/*****************************************************************************
 *   EE2024 Assignment 2: CATS by Siidheesh & Peter
 *
 *   Copyright(C) 2017, Siidheesh & Peter
 *   All rights reserved.
 *
 ******************************************************************************/

#include "main.h"

uint8_t displayStatus = DISPLAY_MODE;
uint8_t displayUpdate = DISPLAY_MODE;
uint8_t strbuf[100] = {0};
char segchar = '0';
uint32_t txCount = 0;
uint16_t txEvent = 0;
uint8_t rxEvent = 0;
uint8_t txSent = 0;
volatile uint8_t collOccured = 0;
volatile uint8_t battOccured = 0;
volatile _Bool ledState = 0;
uint32_t ledBrightness = 0;
int32_t statSeconds = 0;

uint32_t lightr = 0;
int32_t gr = 0;
uint32_t tempr = 0;

volatile tick_t msTicks = 0; // counter for 1ms SysTicks
volatile tick_t secTicks = 0;
volatile tick_t toggleTicks = 0;

volatile tick_t tempTicks = 0;
volatile int32_t tempState = 0;

int8_t x,y,z;
int8_t xoff,yoff,zoff;

_Bool sw4State, newSW4State;
uint8_t joyState, newJoyState;
uint32_t joyTicks = 0;
_Bool joyRepeat = 0;

char framebuf[100] = {0};
uint32_t framept = 0;

void EINT0_IRQHandler(void);
void EINT3_IRQHandler(void);
void SysTick_Handler(void);
void RIT_IRQHandler(void);

static void init_ssp(void) {
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
	SSP_Cmd(LPC_SSP1, ENABLE);

}

static void init_i2c(void) {
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);
	I2C_Init(LPC_I2C2, 100000);
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_GPIO(void) {
	// init SW4 as GPIO input
	PINSEL_CFG_Type PinCfg;
	PinCfg.OpenDrain = 0;
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 31;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(1, 1<<31, 0);
	// init SW3 as EINT0 int
	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, 1<<10, 0);
	// init P2.5 (light sensor) as GPIO int
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 5;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, 1<<5, 0);
	// init P0.2 (temp sensor) as GPIO int
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(0, 1<<2, 0);
	// Enable GPIO Interrupt P0.2
	LPC_GPIOINT->IO0IntClr = 1 << 2;
	LPC_GPIOINT->IO0IntEnF |= 1 << 2;
	// Enable EINT3 interrupt
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);
}

void init_PWM(void) {
	// init OLED PWM (green led P2.1) and RGB Red led (P2.0)
	PINSEL_CFG_Type PinCfg;
	PinCfg.OpenDrain = 0;
	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 1;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PWM_TIMERCFG_Type PWMCfgDat;
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PWM_ConfigStructInit(PWM_MODE_TIMER, &PWMCfgDat);
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &PWMCfgDat);
	PWM_MatchUpdate(LPC_PWM1, 0, 1024, PWM_MATCH_UPDATE_NOW);
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	PWM_ChannelConfig(LPC_PWM1, 2, PWM_CHANNEL_SINGLE_EDGE);
	PWM_MatchUpdate(LPC_PWM1, 1, 0, PWM_MATCH_UPDATE_NOW);
	PWM_MatchUpdate(LPC_PWM1, 2, 1024, PWM_MATCH_UPDATE_NOW);
	PWMMatchCfgDat.MatchChannel = 1;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	PWMMatchCfgDat.MatchChannel = 2;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	PWM_ChannelCmd(LPC_PWM1, 1, ENABLE);
	PWM_ChannelCmd(LPC_PWM1, 2, ENABLE);
	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);
	PWM_Cmd(LPC_PWM1, ENABLE);
}

void init_DAC(void) {
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	DAC_Init(LPC_DAC);
	DAC_UpdateValue (LPC_DAC, 0);
}

static void init_EXT(void) {
	LPC_SC->EXTINT |= 1;
	LPC_SC->EXTMODE |= 1;
	LPC_SC->EXTPOLAR &= 0;
	NVIC_ClearPendingIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT0_IRQn);
}

static void init_RIT(void) {
	RIT_CMP_VAL ritCfg;
	uint32_t clock_rate = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_RIT);
	ritCfg.CMPVAL = ((clock_rate /1000) * 333);
	ritCfg.COUNTVAL = 0;
	ritCfg.MASKVAL = 0;
	RIT_Init(LPC_RIT);
	RIT_Cmd(LPC_RIT, DISABLE);
	RIT_TimerConfig(LPC_RIT, &ritCfg);
	RIT_TimerClearCmd(LPC_RIT, ENABLE);
	RIT_IRQHandler();
	NVIC_ClearPendingIRQ(RIT_IRQn);
	NVIC_EnableIRQ(RIT_IRQn);
}

static void init_TIM(void) {
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	// Initialize timer 0, prescale count time of 1us
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

static void light_updateThresh(void) {
	light_setHiThreshold(settings.light);
	light_clearIrqStatus();
	LPC_GPIOINT->IO2IntClr = 1<<5;
}


static void init_light(void) {
	light_enable();
	light_setRange(LIGHT_RANGE_4000);
	light_updateThresh();
	LPC_GPIOINT->IO2IntEnF |= 1<<5;
}

static void init_EEP(void) {
	eeprom_init();
	eeprom_read((uint8_t*)&settings, 0, sizeof(settings));
	if(settings.check != 0x55) {
		settings.acc = 0.2f;
		settings.light = 3000;
		settings.temp = 280;
		settings.direction = 2;
		settings.orientation = 0;
		settings.check = 0x55;
		settings.locked = 0;
		settings.lockCode = 1234;
		eeprom_write((uint8_t*)&settings, 0, sizeof(settings));
	}
}

static void init_NVIC(void) {
	NVIC_SetPriorityGrouping(NVIC_PRIGROUP); // bxxx.yy000
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 1, 0));	// systick
	NVIC_SetPriority(EINT0_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 2, 0));		// sw3
	NVIC_SetPriority(EINT3_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 2, 1));		// gpio
	NVIC_SetPriority(RIT_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 3, 0));		// rit
	NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 3, 1)); 		// adc
	NVIC_SetPriority(UART3_IRQn, NVIC_EncodePriority(NVIC_PRIGROUP, 4, 0));		// uart3 rx
}

static void init(void) {

	SysTick_Config(SystemCoreClock / 1000);
	WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);

	init_i2c();
	init_ssp();
	init_GPIO();
	init_EXT();
	init_NVIC();
	init_EEP();

	led7seg_init();
	pca9532_init();
	joystick_init();
	acc_init();
	oled_init();
	init_light();

	init_PWM();
	init_ADC();
	init_DAC();
	init_UART();
	init_RIT();
	init_TIM();

	oled_clearScreen(OLED_COLOR_BLACK);
	oled_setOrientation(settings.orientation);

	WDT_Feed();

	if(LPC_SC->RSID & (1<<3)) {
		LPC_SC->RSID |= 1<<3;
		UART_SendString(LPC_UART3, (uint8_t*)"System reset due to brown-out: Check the battery!\r\n");
	} else if(LPC_SC->RSID & (1<<2)) {
		LPC_SC->RSID |= 1<<2;
		UART_SendString(LPC_UART3, (uint8_t*)"System reset due to a watchdog timeout: Bring it in for servicing!\r\n");
	}
	LPC_SC->RSID &= ~0b111111;

	acc_read(&xoff, &yoff, &zoff);
	xoff = 0-xoff;
	yoff = 0-yoff;
	zoff = 64-zoff;

	sw4State = (GPIO_ReadValue(1)>>31)&1;
	joyState = joystick_read();

	sysMode = settings.locked ? SYS_TRANSIT_TO_LOCKED : SYS_TRANSIT_TO_STATIONARY;
	timerFlags.bits.modeToggle = 1;

	WDT_Start(1000000); //https://community.nxp.com/thread/389112
}

int main(void) {
	init();
	for(;;) {
		newSW4State = (GPIO_ReadValue(1)>>31)&1;
		if(sw4State && !newSW4State) {
			statSeconds = 0;
			settings.orientation = !settings.orientation;
			switch(sysMode) {
				case SYS_GUI:
					displayUpdate = DISPLAY_GUI;
					break;
				case SYS_FORWARD:
					led7seg_setChar(segchar, 0, !settings.orientation);
				default:
					displayUpdate = DISPLAY_ALL;
					break;
			}
		}
		sw4State = newSW4State;

		newJoyState = joystick_read();
		if(newJoyState != joyState && sysMode != SYS_LOCKED) {
			if(sysMode != SYS_GUI) {
				if(newJoyState & JOYSTICK_CENTER) {
					oldSysMode = sysMode;
					sysMode = SYS_TRANSIT_TO_GUI;
					timerFlags.bits.modeToggle = 1;
				}
			} else if(newJoyState & JOYSTICK_CENTER) {
				switch(displayGui.page) {
					case GUI_HOME:
						switch(displayGui.index.bits.home) {
							default:
							case 0:
								displayGui.page = GUI_THRESH;
								break;
							case 1:
								displayGui.page = GUI_MODE;
								break;
							case 2:
								displayGui.page = GUI_DIR;
								break;
						}
						break;
					case GUI_THRESH:
						switch(displayGui.index.bits.thresh) {
							default:
							case 0:
								displayGui.page = GUI_THRESH_OBS;
								break;
							case 1:
								displayGui.page = GUI_THRESH_ACC;
								break;
							case 2:
								displayGui.page = GUI_THRESH_BATT;
								break;
						}
						break;
					case GUI_MODE:
						switch(displayGui.index.bits.mode) {
							default:
							case 0:
								oldSysMode = SYS_TRANSIT_TO_STATIONARY;
								break;
							case 1:
								oldSysMode = SYS_TRANSIT_TO_FORWARD;
								break;
							case 2:
								oldSysMode = SYS_TRANSIT_TO_REVERSE;
								break;
						}
						break;
					case GUI_DIR:
						if(displayGui.index.bits.dir<=2) {
							if(settings.direction != displayGui.index.bits.dir) collOccured = 0;
							settings.direction = displayGui.index.bits.dir;
							eeprom_write((uint8_t*)&settings, 0, sizeof(settings));
						}
						break;
					case GUI_THRESH_BATT:
					case GUI_THRESH_ACC:
					case GUI_THRESH_OBS:
						eeprom_write((uint8_t*)&settings, 0, sizeof(settings));
						displayGui.page = GUI_THRESH;
						txEvent |= TX_THRESH_UPDATE;
						light_updateThresh();
						break;
					default: break;
				}
				displayUpdate |= DISPLAY_GUI;
			} else if((!settings.orientation && (newJoyState & JOYSTICK_UP)) || (settings.orientation && (newJoyState & JOYSTICK_DOWN))) {
				switch(displayGui.page) {
					case GUI_HOME:
						if(displayGui.index.bits.home) {
							--displayGui.index.bits.home;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH:
						if(displayGui.index.bits.thresh) {
							--displayGui.index.bits.thresh;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_MODE:
						if(displayGui.index.bits.mode) {
							--displayGui.index.bits.mode;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_DIR:
						if(displayGui.index.bits.dir) {
							--displayGui.index.bits.dir;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					default: break;
				}
			} else if((!settings.orientation && (newJoyState & JOYSTICK_DOWN)) || (settings.orientation && (newJoyState & JOYSTICK_UP))) {
				switch(displayGui.page) {
					case GUI_HOME:
						if(displayGui.index.bits.home < 2) {
							++displayGui.index.bits.home;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH:
						if(displayGui.index.bits.thresh < 2) {
							++displayGui.index.bits.thresh;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_MODE:
						if(displayGui.index.bits.mode < 2) {
							++displayGui.index.bits.mode;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_DIR:
						if(displayGui.index.bits.dir < 2) {
							++displayGui.index.bits.dir;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					default: break;
				}
			} else if((!settings.orientation && (newJoyState & JOYSTICK_LEFT)) || (settings.orientation && (newJoyState & JOYSTICK_RIGHT))) {
				switch(displayGui.page) {
					case GUI_HOME:
						sysMode = SYS_TRANSIT_OUT_OF_GUI;
						timerFlags.bits.modeToggle = 1;
						break;
					default:
					case GUI_THRESH:
					case GUI_MODE:
					case GUI_DIR:
						displayGui.page = GUI_HOME;
						displayUpdate |= DISPLAY_GUI;
						break;
					case GUI_THRESH_BATT:
						if(settings.temp) {
							settings.temp--;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH_ACC:
						if(settings.acc >= 0.1f) {
							settings.acc -= 0.1f;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH_OBS:
						if(settings.light >= 10) {
							settings.light -= 10;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
				}
			} else if((!settings.orientation && (newJoyState & JOYSTICK_RIGHT)) || (settings.orientation && (newJoyState & JOYSTICK_LEFT))) {
				switch(displayGui.page) {
					case GUI_THRESH_BATT:
						if(settings.temp < 500) {
							settings.temp++;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH_ACC:
						if(settings.acc <= 1.9f) {
							settings.acc += 0.1f;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					case GUI_THRESH_OBS:
						if(settings.light <= 4000 - 10) {
							settings.light += 10;
							displayUpdate |= DISPLAY_GUI;
						}
						break;
					default: break;
				}
			}
			joyState = newJoyState;
		} else if((displayGui.page == GUI_THRESH_BATT || displayGui.page == GUI_THRESH_ACC || displayGui.page == GUI_THRESH_OBS) && (joyState & (JOYSTICK_LEFT|JOYSTICK_RIGHT))) {
			//if we're in the settings page and joy appears to be "stuck" left or right
			if(!joyTicks)
				joyTicks = msTicks;
			else if(msTicks - joyTicks >= (joyRepeat?1:800)) {
				joyState &= ~(JOYSTICK_LEFT|JOYSTICK_RIGHT);
				joyRepeat = 1;
				joyTicks = 0;
			}
		} else joyRepeat = 0;

		if (timerFlags.bits.modeToggle) {
			timerFlags.bits.modeToggle = 0;
			txSent = 0;
			displayUpdate = DISPLAY_ALL;
			switch (sysMode) {
				case SYS_TRANSIT_TO_FORWARD:
					sysMode = SYS_FORWARD;
					segchar = 'F';
					displayStatus = DISPLAY_MODE | DISPLAY_SENS;
					txEvent = TX_MODE;
					pca9532_setLeds(0, 0xffff);
					break;
				case SYS_TRANSIT_TO_REVERSE:
					sysMode = SYS_REVERSE;
					displayStatus = DISPLAY_MODE | DISPLAY_SENS;
					txEvent = TX_MODE;
					break;
				case SYS_TRANSIT_TO_STATIONARY:
					sysMode = SYS_STATIONARY;
					displayStatus = DISPLAY_MODE;
					collOccured = 0;
					battOccured = 0;
					RIT_Cmd(LPC_RIT, DISABLE);
					statSeconds = 0;
					pca9532_setLeds(0, 0xffff);
					led7seg_setChar('~', 0, 0);
					break;
				case SYS_TRANSIT_TO_GUI:
					sysMode = SYS_GUI;
					displayGui.page = GUI_HOME;
					displayGui.index.val = 0;
					displayStatus = DISPLAY_GUI;
					displayUpdate = DISPLAY_GUI;
					collOccured = 0;
					battOccured = 0;
					RIT_Cmd(LPC_RIT, DISABLE);
					pca9532_setLeds(0, 0xffff);
					led7seg_setChar('~', 0, 0);
					break;
				case SYS_TRANSIT_OUT_OF_GUI:
					displayStatus &= ~DISPLAY_GUI;
					switch(oldSysMode) {
						case SYS_STATIONARY:
						case SYS_GUI:
							sysMode = SYS_TRANSIT_TO_STATIONARY;
							break;
						case SYS_FORWARD:
							sysMode = SYS_TRANSIT_TO_FORWARD;
							break;
						case SYS_REVERSE:
							sysMode = SYS_TRANSIT_TO_REVERSE;
							break;
						default:
							sysMode = oldSysMode;
							break;
					}
					timerFlags.bits.modeToggle = 1;
					break;
				case SYS_TRANSIT_TO_LOCKED:
					sysMode = SYS_LOCKED;
					pca9532_setLeds(0xffff, 0xffff);
					led7seg_setChar('~', 0, 0);
					RIT_Cmd(LPC_RIT, ENABLE);
					displayStatus = DISPLAY_MODE;
					break;
				default: break;
			}
			secTicks = msTicks - 1000;
			RIT_IRQHandler(); //update leds
		}

		int32_t secDelta = msTicks - secTicks;
		if (secDelta > 999) {
			secTicks = msTicks;
			switch (sysMode) {
				case SYS_FORWARD:
					switch (++segchar) {
						case 'G':
							segchar = '0';
							break;
						case ':':
							segchar = 'A';
						default:
							break;
					}
					led7seg_setChar(segchar, 0, !settings.orientation);
					if(segchar == 'A' || segchar == '5') {
						displayUpdate |= DISPLAY_SENS;
					} else if(segchar == 'F') {
						displayUpdate |= DISPLAY_SENS;
						txEvent |= TX_SENS;
						if(collOccured) txEvent |= TX_COLL;
						if(battOccured) txEvent |= TX_BATT;
					}

					acc_read(&x,&y,&z);
					x += xoff;
					y += yoff;
					float xg = (float)x;
					float yg = (float)y;
					xg /= 64.0f;
					yg /= 64.0f;
					if(xg < 0) xg = -xg;
					if(yg < 0) yg = -yg;
					float g = 0;
					switch(settings.direction) {
						case 0:
							g = xg;
							break;
						case 1:
							g = yg;
							break;
						default:
						case 2:
							xg *= xg;
							yg *= yg;
							xg += yg;
							g = sqrtf(xg);
							break;
					}
					gr = (int32_t)(g+0.5f);
					if(g > settings.acc) {
						if(!(displayStatus & DISPLAY_COLL))
							displayUpdate |= DISPLAY_COLL;
						displayStatus |= DISPLAY_COLL;
						if(collOccured != 1) {
							collOccured = 1;
							txEvent |= TX_COLL;
							RIT_Cmd(LPC_RIT, ENABLE);
							RIT_IRQHandler();
						}
					} else if(displayStatus & DISPLAY_COLL) {
						displayUpdate |= DISPLAY_COLL;
						displayStatus &= ~DISPLAY_COLL;
					} else displayUpdate &= ~DISPLAY_COLL;

					if(tempr > settings.temp) {
						if(!(displayStatus & DISPLAY_BATT))
							displayUpdate |= DISPLAY_BATT;
						displayStatus |= DISPLAY_BATT;
						if(battOccured != 1) {
							battOccured = 1;
							txEvent |= TX_BATT;
							RIT_Cmd(LPC_RIT, ENABLE);
							RIT_IRQHandler();
						}
					} else if(displayStatus & DISPLAY_BATT) {
						displayUpdate |= DISPLAY_BATT;
						displayStatus &= ~DISPLAY_BATT;
					} else displayUpdate &= ~DISPLAY_BATT;
					break;
				case SYS_REVERSE:
					if(!(displayStatus & DISPLAY_SENS))
						displayStatus |= DISPLAY_SENS;
					displayUpdate |= DISPLAY_SENS;
					lightr = light_read();
					pca9532_setLeds(0xff >> ((lightr>>4)&0xff), 0xffff);
					if(lightr > settings.light) {
						txEvent |= TX_OBST;
						if(!(displayStatus & DISPLAY_OBST)) {
							displayUpdate |= DISPLAY_OBST;
							displayStatus |= DISPLAY_OBST;
						}
					} else {
						txSent &= ~TX_OBST;
						if(displayStatus & DISPLAY_OBST) {
							displayUpdate |= DISPLAY_OBST;
							displayStatus &= ~DISPLAY_OBST;
						}
					}
					break;
				case SYS_STATIONARY:
					if(statSeconds >= 0 && ++statSeconds > 3) {
						statSeconds = -1;
						oled_putString(2, 30, "SLEEP mode", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						CLKPWR_Sleep();
					}
					break;
				default:
					break;
			}
		}

		if(adcFlag) {
			adcFlag = 0;
			uint32_t val = (uint32_t)adc_val>>2;
			ledBrightness = val;
			ADC_StartCmd(LPC_ADC,ADC_START_NOW);
			NVIC_EnableIRQ(ADC_IRQn);
		}

		if(tempState == -1) {
			tempTicks -= 2731;
			tempr = (float)tempTicks;
			tempState = 0;
		}

		if(displayUpdate) {
			if(sysMode == SYS_GUI) {
				if(displayUpdate & DISPLAY_GUI) {
					oled_clearScreen(OLED_COLOR_BLACK);
					oled_setOrientation(settings.orientation);
				}
			} else if(displayUpdate & DISPLAY_CLEAR) {
				oled_clearScreen(OLED_COLOR_BLACK);
				oled_setOrientation(settings.orientation);
				displayUpdate &= ~DISPLAY_CLEAR;
			}
			else if(displayUpdate & DISPLAY_MODE)
				oled_fillRect(2, 10, 95, 18, OLED_COLOR_BLACK);
			else if(displayUpdate & DISPLAY_SENS)
				oled_fillRect(2, 30, 95, 38, OLED_COLOR_BLACK);
			else if(displayUpdate & DISPLAY_COLL)
				oled_fillRect(2, 40, 95, 48, OLED_COLOR_BLACK);
			else if((sysMode == SYS_FORWARD && displayUpdate & DISPLAY_BATT) || (sysMode == SYS_REVERSE && displayUpdate & DISPLAY_OBST))
				oled_fillRect(2, 50, 95, 58, OLED_COLOR_BLACK);
			uint8_t delta = displayUpdate & displayStatus;
			switch(sysMode) {
				case SYS_STATIONARY:
					if(delta & DISPLAY_MODE) {
						oled_putString(2, 10, (uint8_t*)"Stationary", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_MODE;
					}
					displayUpdate &= ~(DISPLAY_SENS|DISPLAY_COLL|DISPLAY_BATT|DISPLAY_OBST);
					break;
				case SYS_FORWARD:
					if(delta & DISPLAY_MODE) {
						oled_putString(2, 10, (uint8_t*)"Forward", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_MODE;
					} else if(delta & DISPLAY_SENS) {
						snprintf(strbuf, 100, (uint8_t*)"T:%02d.%01dC A:%01d.%02dg", tempr/10, tempr%10, gr/100, gr%100);
						oled_putString(2, 30, strbuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_SENS;
					} else if(delta & DISPLAY_COLL) {
						oled_putString(2, 40, (uint8_t*)"Airbag released", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_COLL;
					} else if(delta & DISPLAY_BATT) {
						oled_putString(2, 50, (uint8_t*)"Temp. too high", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_BATT;
					}
					displayUpdate &= ~(DISPLAY_OBST);
					break;
				case SYS_REVERSE:
					if(delta & DISPLAY_MODE) {
						oled_putString(2, 10, (uint8_t*)"Reverse", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_MODE;
					} else if(delta & DISPLAY_SENS) {
						snprintf(strbuf, 100, (uint8_t*)"Light: %d lux", lightr);
						oled_putString(2, 30, strbuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_SENS;
					} else if(delta & DISPLAY_OBST) {
						oled_putString(2, 50, (uint8_t*)"Obstacle near", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_OBST;
					}
					displayUpdate &= ~(DISPLAY_COLL|DISPLAY_BATT);
					break;
				case SYS_GUI:
					if(delta & DISPLAY_GUI) {
						switch(displayGui.page) {
							case GUI_HOME:
								oled_rect(16, 8, 79, 18, OLED_COLOR_WHITE);
								oled_putString(20, 10, (uint8_t*)"THRESH", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 28, 79, 38, OLED_COLOR_WHITE);
								oled_putString(20, 30, (uint8_t*)"MODE", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 48, 79, 58, OLED_COLOR_WHITE);
								oled_putString(20, 50, (uint8_t*)"DIR", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								switch(displayGui.index.bits.home) {
									default:
										displayGui.index.bits.home = 0;
									case 0:
										oled_fillRect(69, 8, 79, 18, OLED_COLOR_WHITE);
										break;
									case 1:
										oled_fillRect(69, 28, 79, 38, OLED_COLOR_WHITE);
										break;
									case 2:
										oled_fillRect(69, 48, 79, 58, OLED_COLOR_WHITE);
										break;
								}
								break;
							case GUI_THRESH:
								oled_rect(16, 8, 79, 18, OLED_COLOR_WHITE);
								oled_putString(20, 10, (uint8_t*)"OBSTACLE", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 28, 79, 38, OLED_COLOR_WHITE);
								oled_putString(20, 30, (uint8_t*)"ACCL", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 48, 79, 58, OLED_COLOR_WHITE);
								oled_putString(20, 50, (uint8_t*)"TEMP", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								switch(displayGui.index.bits.thresh) {
									default:
										displayGui.index.bits.thresh = 0;
									case 0:
										oled_fillRect(69, 8, 79, 18, OLED_COLOR_WHITE);
										break;
									case 1:
										oled_fillRect(69, 28, 79, 38, OLED_COLOR_WHITE);
										break;
									case 2:
										oled_fillRect(69, 48, 79, 58, OLED_COLOR_WHITE);
										break;
								}
								break;
							case GUI_MODE:
								oled_rect(16, 8, 79, 18, OLED_COLOR_WHITE);
								oled_putString(20, 10, (uint8_t*)"STAT", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 28, 79, 38, OLED_COLOR_WHITE);
								oled_putString(20, 30, (uint8_t*)"FORW", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 48, 79, 58, OLED_COLOR_WHITE);
								oled_putString(20, 50, (uint8_t*)"REVE", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								switch(oldSysMode) {
									case SYS_STATIONARY:
									case SYS_TRANSIT_TO_STATIONARY:
										oled_line(60,13,65,13,OLED_COLOR_WHITE);
										break;
									case SYS_FORWARD:
									case SYS_TRANSIT_TO_FORWARD:
										oled_line(60,33,65,33,OLED_COLOR_WHITE);
										break;
									case SYS_REVERSE:
									case SYS_TRANSIT_TO_REVERSE:
										oled_line(60,53,65,53,OLED_COLOR_WHITE);
										break;
									default: break;
								}
								switch(displayGui.index.bits.mode) {
									default:
										displayGui.index.bits.mode = 0;
									case 0:
										oled_fillRect(69, 8, 79, 18, OLED_COLOR_WHITE);
										break;
									case 1:
										oled_fillRect(69, 28, 79, 38, OLED_COLOR_WHITE);
										break;
									case 2:
										oled_fillRect(69, 48, 79, 58, OLED_COLOR_WHITE);
										break;
									}
								break;
							case GUI_DIR:
								oled_rect(16, 8, 79, 18, OLED_COLOR_WHITE);
								oled_putString(20, 10, (uint8_t*)"X", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 28, 79, 38, OLED_COLOR_WHITE);
								oled_putString(20, 30, (uint8_t*)"Y", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								oled_rect(16, 48, 79, 58, OLED_COLOR_WHITE);
								oled_putString(20, 50, (uint8_t*)"X + Y", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								switch(settings.direction) {
									case 0:
										oled_line(60,13,65,13,OLED_COLOR_WHITE);
										break;
									case 1:
										oled_line(60,33,65,33,OLED_COLOR_WHITE);
										break;
									default:
										settings.direction = 2;
									case 2:
										oled_line(60,53,65,53,OLED_COLOR_WHITE);
										break;
								}
								switch(displayGui.index.bits.dir) {
									case 0:
										oled_fillRect(69, 8, 79, 18, OLED_COLOR_WHITE);
										break;
									case 1:
										oled_fillRect(69, 28, 79, 38, OLED_COLOR_WHITE);
										break;
									default:
										displayGui.index.bits.dir = 2;
									case 2:
										oled_fillRect(69, 48, 79, 58, OLED_COLOR_WHITE);
										break;
								}
								break;
							case GUI_THRESH_BATT: {
								float v = (float)settings.temp;
								v = ((v/500.0f)*90.0f)+5.5f;
								uint32_t x = (uint32_t)v;
								snprintf(strbuf, 100, (uint8_t*)"Temp: %02u.%01uC", settings.temp/10, settings.temp%10);
								oled_rect(5, 30, 95, 40, OLED_COLOR_WHITE);
								oled_fillRect(5, 30, x, 40, OLED_COLOR_WHITE);
								oled_putString(5, 50, (uint8_t*)strbuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								break;
							}
							case GUI_THRESH_ACC: {
								float v = (float)settings.acc;
								v = ((v/2.0f)*90.0f)+5.5f;
								uint32_t x = (uint32_t)v;
								snprintf(strbuf, 100, (uint8_t*)"Acc: %0.3fg", settings.acc);
								oled_rect(5, 30, 95, 40, OLED_COLOR_WHITE);
								oled_fillRect(5, 30, x, 40, OLED_COLOR_WHITE);
								oled_putString(5, 50, (uint8_t*)strbuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								break;
							}
							case GUI_THRESH_OBS: {
								float v = (float)settings.light;
								v = ((v/4000.0f)*90.0f)+5.5f;
								uint32_t x = (uint32_t)v;
								snprintf(strbuf, 100, (uint8_t*)"Light: %u lx", settings.light);
								oled_rect(5, 30, 95, 40, OLED_COLOR_WHITE);
								oled_fillRect(5, 30, x, 40, OLED_COLOR_WHITE);
								oled_putString(5, 50, (uint8_t*)strbuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
								break;
							}
							default: break;
						}
						displayUpdate &= ~DISPLAY_GUI;
					}
				case SYS_LOCKED:
					if(delta & DISPLAY_MODE) {
						oled_putString(2, 10, (uint8_t*)"LOCKED!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
						displayUpdate &= ~DISPLAY_MODE;
					}
				default: break;
			}
		}

		if(txEvent) {
			switch(sysMode) {
				case SYS_FORWARD:
					if(txEvent & TX_MODE) {
						UART_SendString(XBEE_UART, (uint8_t*)"Entering Forward mode.\r\n");
						txEvent &= ~TX_MODE;
					} else if(txEvent & TX_BATT) {
						UART_SendString(XBEE_UART, (uint8_t*)"Temperature too high.\r\n");
						txEvent &= ~TX_BATT;
					} else if(txEvent & TX_COLL) {
						UART_SendString(XBEE_UART, (uint8_t*)"Collision has been detected.\r\n");
						txEvent &= ~TX_COLL;
					} else if(txEvent & TX_SENS) {
						snprintf(strbuf, 100, (uint8_t*)"%03d_Temp_%02u.%02u_ACC_%02u.%02u\r\n", txCount, tempr/10, tempr%10, gr/100, gr%100);
						UART_SendString(XBEE_UART, strbuf);
						txCount++;
						txEvent &= ~TX_SENS;
					}
					break;
				case SYS_REVERSE:
					if(txEvent & TX_MODE) {
						UART_SendString(XBEE_UART, (uint8_t*)"Entering Reverse mode.\r\n");
						txEvent &= ~TX_MODE;
					} else if(txEvent & TX_OBST && !(txSent & TX_OBST)) {
						UART_SendString(XBEE_UART, (uint8_t*)"Obstacle too near.\r\n");
						txEvent &= ~TX_OBST;
						txSent |= TX_OBST;
					}
					break;
				case SYS_GUI:
					if(txEvent & TX_GUI) {
						UART_SendString(XBEE_UART, (uint8_t*)"Entering GUI mode.\r\n");
						txEvent &= ~TX_GUI;
					}
					break;
				default: break;
			}
			if(txEvent & TX_THRESH_UPDATE) {
				UART_SendString(XBEE_UART, (uint8_t*)"Threshold settings updated.\r\n");
				txEvent &= ~TX_THRESH_UPDATE;
			} else if(txEvent & TX_UNKNOWN_CMD) {
				UART_SendString(XBEE_UART, (uint8_t*)"Invalid command!\r\n");
				txEvent &= ~TX_UNKNOWN_CMD;
			}
		}

		if(rx_available && rx_read(framebuf+framept) == 0) {
			if(framebuf[framept] == '\r') { //cr, not all terms send crlf, so treat cr as end of frame
				int i, j;
				_Bool unknownCmd = 0;
				uint32_t spaces[100] = {0};
				for(i = 0, j = 0; i <= framept; i++) { //iterate to find spaces as strtok implementation is flawed
					if(framebuf[i] == ' ') spaces[j++] = i;
				}
				switch(j) {
					default:
					case 0:
						if(strncmp(framebuf, "reset\r", 6) == 0) {
							NVIC_SystemReset();
						} else unknownCmd = 1;
						break;
					case 1:
						if(strncmp(framebuf, "mode ", 5) == 0) {
							if(strncmp(framebuf+spaces[0]+1, "stationary\r", 11) == 0) {
								sysMode = SYS_TRANSIT_TO_STATIONARY;
								timerFlags.bits.modeToggle = 1;
							} else if(strncmp(framebuf+spaces[0]+1, "forward\r", 8) == 0) {
								sysMode = SYS_TRANSIT_TO_FORWARD;
								timerFlags.bits.modeToggle = 1;
							} else if(strncmp(framebuf+spaces[0]+1, "reverse\r", 8) == 0) {
								sysMode = SYS_TRANSIT_TO_REVERSE;
								timerFlags.bits.modeToggle = 1;
							} else if(strncmp(framebuf+spaces[0]+1, "gui\r", 4) == 0) {
								sysMode = SYS_TRANSIT_TO_GUI;
								timerFlags.bits.modeToggle = 1;
							} else unknownCmd = 1;
						} else if(!settings.locked && strncmp(framebuf, "lock ", 5) == 0) {
							uint32_t p;
							if(sscanf(framebuf+spaces[0]+1,"%4u",&p) && p <= 9999) {
								settings.lockCode = p;
								settings.locked = 1;
								sysMode = SYS_TRANSIT_TO_LOCKED;
								timerFlags.bits.modeToggle = 1;
								eeprom_write((uint8_t*)&settings, 0, sizeof(settings));
							}  else unknownCmd = 1;
						} else if(settings.locked && strncmp(framebuf, "unlock ", 7) == 0) {
							uint32_t p;
							if(sscanf(framebuf+spaces[0]+1,"%4u",&p) && p <= 9999 && settings.lockCode == p) {
								settings.locked = 0;
								sysMode = SYS_TRANSIT_TO_STATIONARY;
								timerFlags.bits.modeToggle = 1;
								eeprom_write((uint8_t*)&settings, 0, sizeof(settings));
							}  else unknownCmd = 1;
						}  else unknownCmd = 1;
						break;
					case 2:
						if(strncmp(framebuf, "thresh ", 7) == 0) {
							if(strncmp(framebuf+spaces[0]+1, "acc ", 4) == 0) {
								float f;
								if(sscanf(framebuf+spaces[1]+1,"%fg",&f) && (f < 2.0f && f >= 0)) {
									settings.acc = f;
									txEvent |= TX_THRESH_UPDATE;
								}  else unknownCmd = 1;
							} else if(strncmp(framebuf+spaces[0]+1, "light ", 6) == 0) {
								uint32_t t;
								if(sscanf(framebuf+spaces[1]+1,"%u",&t) && t <= 4000) {
									settings.light = t;
									light_updateThresh();
									txEvent |= TX_THRESH_UPDATE;
								}  else unknownCmd = 1;
							} else if(strncmp(framebuf+spaces[0]+1, "temp ", 5) == 0) {
								uint32_t t;
								if(sscanf(framebuf+spaces[1]+1,"%u",&t) && t <= 500) {
									settings.temp = t;
									txEvent |= TX_THRESH_UPDATE;
								}  else unknownCmd = 1;
							}  else unknownCmd = 1;
						}  else unknownCmd = 1;
						break;
				}
				if(unknownCmd) txEvent |= TX_UNKNOWN_CMD;
				framept = 0;
			} else if(framebuf[framept] == '\b' || framebuf[framept] == 0x7f) { //backspace or del
				if(framept > 0) framept--;
			} else if(framebuf[framept] == '\n' || ++framept == 100) //newline, not all terms send crlf, so reset framept on lf | otherwise if framept out of bounds, reset it
				framept = 0;
		}

		WDT_Feed();

	}
}

void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1);
}

void EINT0_IRQHandler(void) {
	switch (sysMode) {
		case SYS_STATIONARY:
			toggleTicks = 1000;
			sysMode = SYS_TRANSIT_TO_FORWARD;
			break;
		case SYS_TRANSIT_TO_FORWARD:
			if (toggleTicks > 0) sysMode = SYS_TRANSIT_TO_REVERSE;
			break;
		case SYS_FORWARD:
		case SYS_REVERSE:
			sysMode = SYS_TRANSIT_TO_STATIONARY;
			timerFlags.bits.modeToggle = 1;
			break;
		case SYS_GUI:
			sysMode = SYS_TRANSIT_OUT_OF_GUI;
			timerFlags.bits.modeToggle = 1;
		default: break;
	}
	LPC_SC->EXTINT |= 1;
}

void EINT3_IRQHandler(void) {
	if(((LPC_GPIOINT->IO0IntStatF>>2) & 1)) {
		if(tempState > -1) {
			if(tempState == 0) {
				tempTicks = LPC_TIM0->TC;
				tempState++;
			} else if(tempState++ == 256) {
				int32_t tempDelta = LPC_TIM0->TC - tempTicks;
				tempTicks = (uint32_t) tempDelta;
				tempTicks >>= 8;
				tempState = -1;
			}
		}
		LPC_GPIOINT->IO0IntClr |= 1<<2;
	}
	if ((LPC_GPIOINT->IO2IntStatF>>5)& 0x1) {
		if(sysMode == SYS_REVERSE) {
			txEvent |= TX_OBST;
			if(!(displayStatus & DISPLAY_OBST)) {
				displayUpdate |= DISPLAY_OBST;
				displayStatus |= DISPLAY_OBST;
			}
		}
		light_clearIrqStatus(); // Clear IRQ otherwise the interrupt will never be issued again.
		LPC_GPIOINT->IO2IntClr = (1<<5); // Clear the interrupt register
	}
}

void SysTick_Handler(void) {
	++msTicks;
	if (toggleTicks > 0 && --toggleTicks == 0) timerFlags.bits.modeToggle = 1;
}

void RIT_IRQHandler(void) {
	RIT_GetIntStatus(LPC_RIT);
	if(collOccured == 1 || sysMode == SYS_LOCKED) {
		LPC_DAC->DACR = (LPC_DAC->DACR & DAC_BIAS_EN) | DAC_VALUE(ledState?(sysMode == SYS_LOCKED?1023:ledBrightness):0);
	} else if(!collOccured) {
		LPC_DAC->DACR = (LPC_DAC->DACR & DAC_BIAS_EN) | DAC_VALUE(0);
		collOccured = 2;
	}
	if(battOccured ==  1 || sysMode == SYS_LOCKED) {
		LPC_PWM1->MR1 = (sysMode == SYS_LOCKED?!ledState:ledState)?(sysMode == SYS_LOCKED?1023:ledBrightness):0;
		LPC_PWM1->MR2 = (sysMode == SYS_LOCKED)?(ledState?1023:255):255;
		LPC_PWM1->LER |= PWM_LER_EN_MATCHn_LATCH(1) | PWM_LER_EN_MATCHn_LATCH(2);
	} else if(!battOccured) {
		LPC_PWM1->MR1 = 0;
		LPC_PWM1->MR2 = 1023;
		LPC_PWM1->LER |= PWM_LER_EN_MATCHn_LATCH(1) | PWM_LER_EN_MATCHn_LATCH(2);
		LPC_PWM1->TCR |= PWM_TCR_COUNTER_RESET;
		LPC_PWM1->TCR &= (~PWM_TCR_COUNTER_RESET) & PWM_TCR_BITMASK;
		battOccured = 2;
	}
	ledState = !ledState;
}
