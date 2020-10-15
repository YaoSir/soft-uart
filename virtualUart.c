#include "soft_uart.h"

#define SYS_FREQ SystemCoreClock											        //MCU system clock frequency

//Virtual uart configuration
#define VIRTUAL_USART_TX_RCU 						RCU_GPIOB 						//Soft uart TX RCU
#define VIRTUAL_USART_RX_RCU 						RCU_GPIOB 						//Soft uart RX RCU

#define VIRTUAL_USART_TX_PORT 					    GPIOB     						//Soft uart TX port B
#define VIRTUAL_USART_RX_PORT 					    GPIOB      						//Soft uart RX port B

#define VIRTUAL_USART_TX_PIN 						GPIO_PIN_3 						//Soft uart TX pin 4
#define VIRTUAL_USART_RX_PIN 						GPIO_PIN_4 						//Soft uart RX pin 3

#define VIRTUAL_USART_RX_EXTI_PORT			        EXTI_SOURCE_GPIOB			    //RX port interrupt config
#define VIRTUAL_USART_RX_EXTI_SOURCE		        EXTI_SOURCE_PIN4			    //RX pin interrupt config
#define VIRTUAL_USART_RX_EXTI_LINE 			        EXTI_4						    //Virtual uart data comming interrupt line

#define VIRTUAL_USART_TIMER						    TIMER2                          //Timer virtual uart used
#define VIRTUAL_TIMER_IRQ							TIMER2_IRQn                     //Timer IRQ
#define VIRTUAL_USART_BAUDRATE						9600							//Baudrate for virtual uart(8N1 default)

//Struct of virtual uart 
soft_uart_struct virtual_uart;


//GPS driver interface
void virtual_uart_init(void)
{
    timer_parameter_struct timer_initpara;
  
	nvic_irq_enable(TIMER2_IRQn, 3,3);												//Enable virtual_uart timer interrupt
    nvic_irq_enable(EXTI4_15_IRQn, 2U,2U);											//Enable virtual_uart software rx exit line interrupt
	
    //Init virtual uart configuration
	virtual_uart.rsTxBusy = false;
	virtual_uart.rsRxBusy = false;
	virtual_uart.rx_linex = VIRTUAL_USART_RX_EXTI_LINE;
	virtual_uart.timer_periph = TIMER2;
    virtual_uart.tx_port = VIRTUAL_USART_TX_PORT;
	virtual_uart.tx_pin = VIRTUAL_USART_TX_PIN;
    virtual_uart.rx_port = VIRTUAL_USART_RX_PORT;
	virtual_uart.rx_pin = VIRTUAL_USART_RX_PIN;
	
    //Config clock
    rcu_periph_clock_enable(VIRTUAL_USART_TX_RCU);
    rcu_periph_clock_enable(VIRTUAL_USART_RX_RCU);
    rcu_periph_clock_enable(RCU_CFGCMP);
  
	//Config tx gpio
    gpio_bit_set(VIRTUAL_USART_TX_PORT,VIRTUAL_USART_TX_PIN);
    gpio_mode_set(VIRTUAL_USART_TX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,VIRTUAL_USART_TX_PIN);
    gpio_output_options_set(VIRTUAL_USART_TX_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_10MHZ,VIRTUAL_USART_TX_PIN);
	
	//Config rx gpio and interrupt
    gpio_mode_set(VIRTUAL_USART_RX_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,VIRTUAL_USART_RX_PIN); 
    syscfg_exti_line_config(VIRTUAL_USART_RX_EXTI_PORT, VIRTUAL_USART_RX_EXTI_SOURCE);
    exti_init(virtual_uart.rx_linex, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(virtual_uart.rx_linex);
	exti_interrupt_disable(virtual_uart.rx_linex);
 
    //Config virtual_uart timer
    rcu_periph_clock_enable(RCU_TIMER2);
    timer_deinit(virtual_uart.timer_periph);
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = SystemCoreClock/(VIRTUAL_USART_BAUDRATE); 	//a symbol interval(a bit last time)
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(virtual_uart.timer_periph,&timer_initpara);
  
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(virtual_uart.timer_periph);
    timer_interrupt_enable(virtual_uart.timer_periph,TIMER_INT_UP);
}
void virtual_uart_transmit(uint8_t ch)
{
	soft_uart_transmit(&virtual_uart,ch);
}
uint8_t virtual_uart_recieve()
{
	return soft_uart_recieve(&virtual_uart);
}


//Timer interrupt handler
void TIMER2_IRQHandler(void)
{
	soft_uart_struct *soft_uart;
	if(SET == timer_interrupt_flag_get(VIRTUAL_USART_TIMER,TIMER_INT_UP)) 			//Timer interrupt has been enabled
	{
		soft_uart = &virtual_uart;
        //Every timer interrpt means a bit period finished
		if(soft_uart->rsRxBusy)
		{
			recieve_process(soft_uart);
		}else if(soft_uart->rsTxBusy){
			transmit_process(soft_uart);
		}else{
			timer_disable(soft_uart->timer_periph);
		}
		timer_interrupt_flag_clear(VIRTUAL_USART_TIMER,TIMER_INT_UP);
	}
}
