#include <stdio.h>
#include <stdbool.h>
#include "soft-uart.h"
#include "gd32f3x0.h"

//Local fuctions declaration
void transmit_process(soft_uart_struct* uart);
void recieve_process(soft_uart_struct* uart);


//Read GPIO level 
uint8_t soft_uart_rx_r(uint32_t port, uint32_t pin)								//Read RX gpio level
{    
	if((uint32_t)RESET != (GPIO_ISTAT(port)&(pin)))
		return SET;
	else 
		return RESET;
}
//Write GPIO level
void soft_uart_tx_w(uint32_t port, uint32_t pin, uint8_t x)						//Write TX gpio level
{ 
	gpio_bit_write(port, pin, (bit_status)(x)); 
}

//Transmit a byte using soft uart
void soft_uart_transmit(soft_uart_struct *uart, uint8_t ch)
{
	if(uart->rsTxBusy || uart->rsRxBusy) 										//Make sure now is not in transmit or recieve
		return;
	uart->rsTx.shiftBuff = ch;
	uart->rsTx.bitCount = 0;
	uart->rsTxBusy = true;
	timer_enable(uart->timer_periph); 											//Enable timer
	while(uart->rsTxBusy);
}

//Recieve a byte using soft uart. Return the byte recieved
uint8_t soft_uart_recieve(soft_uart_struct* uart)
{
	if(uart->rsTxBusy || uart->rsRxBusy)
		return 0;
	uart->rsRx.shiftBuff = 0;
	uart->rsRxBusy = true;
	exti_interrupt_enable(uart->rx_linex);
	while(uart->rsRxBusy);
	return uart->rsRx.shiftBuff;
}

//Implement the transmit process
void transmit_process(soft_uart_struct* uart)
{
	if(0 == uart->rsTx.bitCount)
	{
		soft_uart_tx_w(uart->tx_port,uart->tx_pin,0); 		                    //Set start bit
	}
	else if(9 == uart->rsTx.bitCount)
	{
		soft_uart_tx_w(uart->tx_port,uart->tx_pin,1); 		                    //Set stop bit
	}
	else if(10 == uart->rsTx.bitCount)
	{
		uart->rsTxBusy=false;
		timer_disable(uart->timer_periph);										//DISABLE timer
	}
	else
	{
		if (uart->rsTx.shiftBuff & 0x01)
		{
			soft_uart_tx_w(uart->tx_port,uart->tx_pin,1);
		}
		else
		{
			soft_uart_tx_w(uart->tx_port,uart->tx_pin,0);
		}
		uart->rsTx.shiftBuff >>= 1;
	}
	uart->rsTx.bitCount++;
}
//Implement the recieve process
void recieve_process(soft_uart_struct* uart)
{
	uart->rsRx.bitCount++;
	if(9 == uart->rsRx.bitCount)
	{
		uart->rsRxBusy = false;
		timer_disable(uart->timer_periph);										//STOP BIT, Close timer
		return ;
	}
	if(SET == soft_uart_rx_r(uart->rx_port,uart->rx_pin))
	{
		uart->rsRx.shiftBuff |= (0x01U << (uart->rsRx.bitCount-1));
	}
}
