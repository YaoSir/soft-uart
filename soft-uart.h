#ifndef _SOFT_UART_H
#define _SOFT_UART_H

#include <stdio.h>
#include <stdbool.h>
#include "gd32f3x0.h"

//Soft uart struct
typedef struct  {
	unsigned char shiftBuff;   										            //Transmit register
	unsigned char bitCount;  											        //Trasnmit bits
} _rsTx;

typedef struct {
	unsigned char shiftBuff;     									            //Recieve register
	unsigned char bitCount;  											        //Recieve bits
	unsigned char finishFlag;											        //One byte recieve finished flag
} _rsRx;

typedef struct
{ 
    _rsTx rsTx;
    _rsRx rsRx;
    bool rsTxBusy; 													            //Transmit busy flag
	bool rsRxBusy; 													            //Recieve busy flag
    uint32_t timer_periph;										                //Soft uart timer(shared by TX and RX)
    uint32_t tx_port;
	uint32_t tx_pin;
	uint32_t rx_port;
	uint32_t rx_pin;
    exti_line_enum rx_linex;									                //Exit line for RX
}soft_uart_struct;

//Transmit a byte using soft uart
void soft_uart_transmit(soft_uart_struct *uart, uint8_t ch);

//Recieve a byte using soft uart. Return the byte recieved
uint8_t soft_uart_recieve(soft_uart_struct* uart);

#endif
