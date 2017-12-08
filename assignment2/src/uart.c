/*
 * uart.c
 *
 *  Created on: Nov 12, 2017
 *      Author: Siidheesh
 */

#include "uart.h"

uint8_t cmdbuf;
volatile serial_ring_buffer_t rx = {{0}, 0, 0};

void uart_rx_callback(void) {
	int i = (unsigned int) (rx.head + 1) & SERIAL_BUF_SIZEOP;
	if (i != rx.tail) {
		rx.buffer[rx.head] = UART_ReceiveData(LPC_UART3);
		rx.head = i;
	}
}

uint16_t rx_read(uint8_t *buf) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (rx.head == rx.tail) return BUF_EMPTY;
	*buf = rx.buffer[rx.tail];
	rx.tail = (unsigned int) (rx.tail + 1) & SERIAL_BUF_SIZEOP;
	return 0;
}

void init_UART(void) {
	// init 0.0 TXD3
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	// init 0.1 RXD3
	PinCfg.Funcnum = 2;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
	UART_CFG_Type UartCfg;
	UART_ConfigStructInit(&UartCfg);
	UartCfg.Baud_rate = XBEE_BAUD;
	UART_Init(XBEE_UART, &UartCfg);
	UART_SetupCbs(XBEE_UART, 0, uart_rx_callback);
	UART_IntConfig(XBEE_UART, UART_INTCFG_RBR, ENABLE);
	UART_TxCmd(XBEE_UART, ENABLE);
	NVIC_ClearPendingIRQ(UART3_IRQn);
	NVIC_EnableIRQ(UART3_IRQn);
}

void UART3_IRQHandler(void) {
	UART3_StdIntHandler();
}
