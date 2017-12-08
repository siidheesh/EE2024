/*
 * uart.h
 *
 *  Created on: Nov 12, 2017
 *      Author: Siidheesh
 */

#ifndef UART_H_
#define UART_H_

#include "stdint.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "core_cm3.h"

#define XBEE_UART LPC_UART3
#define XBEE_BAUD 115200

#define SERIAL_BUFFER_SIZE 1024
#define SERIAL_BUF_SIZEOP (SERIAL_BUFFER_SIZE - 1)//1023

#define rx_available ((SERIAL_BUFFER_SIZE+rx.head-rx.tail) & SERIAL_BUF_SIZEOP)
#define tx_available ((SERIAL_BUFFER_SIZE+tx.head-tx.tail) & SERIAL_BUF_SIZEOP)
#define rx_empty (pcrx.head == pcrx.tail)
#define tx_empty (camrx.head == camrx.tail)

#define BUF_EMPTY 0x100



typedef struct {
    char buffer[SERIAL_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} serial_ring_buffer_t;


extern uint8_t cmdbuf;

extern volatile serial_ring_buffer_t rx;
extern void uart_rx_callback(void);
extern uint16_t rx_read(uint8_t *buf);
extern void init_UART(void);

extern void UART3_IRQHandler(void);

#endif /* UART_H_ */
