/*
 * adc.h
 *
 *  Created on: Nov 12, 2017
 *      Author: Siidheesh
 */

#ifndef ADC_H_
#define ADC_H_

#include "stdint.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "core_cm3.h"

extern volatile uint32_t adc_val;
extern volatile uint32_t adc_off;
extern volatile _Bool adcFlag;

extern void init_ADC(void);

extern void ADC_IRQHandler(void);

#endif /* ADC_H_ */
