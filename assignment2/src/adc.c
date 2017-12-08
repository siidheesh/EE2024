/*
 * adc.c
 *
 *  Created on: Nov 12, 2017
 *      Author: Siidheesh
 */

#include "adc.h"

volatile uint32_t adc_val = 0;
volatile uint32_t adc_off = 0;
volatile _Bool adcFlag = 0;
volatile _Bool adcInit = 1;

void init_ADC(void) {
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	ADC_StartCmd(LPC_ADC,ADC_START_NOW);
	NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_IRQHandler(void) {
	adcFlag = 1;
	if(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_0,ADC_DATA_DONE)) {
		adc_val =  ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0);
		if(adcInit) {
			adc_off = adc_val - 10;
			adcInit = 0;
		}
		NVIC_DisableIRQ(ADC_IRQn);
	}
}
