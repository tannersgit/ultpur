/*
 * UltraPure.c
 *
 *  Created on: Nov 5, 2018
 *      Author: Tanner
 */

#include "stm32f0xx.h"
#include "UltraPure.h"

//global status structures
volatile ultra_pure_status_t status;


static void gpio_config_input( GPIO_TypeDef* port, uint8_t pin )
{
	port->MODER		&=	~(3 << (pin * 2));		//clear pin mode selection, default to input mode
	port->PUPDR		&=	~(3 << (pin * 2));		//clear pull up resistor selection
	port->PUPDR		|=	2 << (pin * 2);			//enable pull down resistors
}

static void gpio_config_output( GPIO_TypeDef* port, uint8_t pin )
{
	port->MODER		&=	~(3 << (pin * 2));		//clear pin mode selection
	port->MODER		|=	1 << (pin * 2);			//set pin mode to output
	port->OTYPER	&=	~(1 << pin);			//set output type to push-pull
	port->OSPEEDR	|=	~(3 << (pin * 2));		//set output speed to low
	port->PUPDR		&=	~(3 << (pin * 2));		//disable pull resistors
	port->BSRR		=	1 << (pin + 16);		//output low
}

static void gpio_output_high( GPIO_TypeDef* port, uint8_t pin )
{
	port->BSRR	=	1 << pin;					//output high
}

static void gpio_output_low( GPIO_TypeDef* port, uint8_t pin )
{
	port->BSRR	=	1 << (pin + 16);			//output low
}

static int gpio_output( GPIO_TypeDef* port, uint8_t pin )
{
	return ( port->ODR & (1 << pin) ? 1 : 0 );	//return output state
}

static int gpio_input( GPIO_TypeDef* port, uint8_t pin )
{
	return ( port->IDR & (1 << pin) ? 1 : 0 );	//return input state
}



void sensor_monitor( sensor_id_t sensor_id )
{
	static uint32_t success_counter_1 = 0;
	static uint32_t success_counter_2 = 0;
	uint32_t*		success_counter;
	int com_output, nc_input, no_input;
	GPIO_TypeDef* port;
	uint8_t pin;
	volatile sensor_status_t* sensor_status;

	//load appropriate sensor
	switch( sensor_id ){
	case SENSOR_1:
		com_output 		= gpio_output( SENSOR_1_COM_PORT, SENSOR_1_COM_PIN );
		nc_input		= gpio_input( SENSOR_1_NC_PORT, SENSOR_1_NC_PIN );
		no_input		= gpio_input( SENSOR_1_NO_PORT, SENSOR_1_NO_PIN );
		success_counter	= &success_counter_1;
		port			= SENSOR_1_COM_PORT;
		pin				= SENSOR_1_COM_PIN;
		sensor_status	= &(status.sensor_1);
		break;
	case SENSOR_2:
		com_output 		= gpio_output( SENSOR_2_COM_PORT, SENSOR_2_COM_PIN );
		nc_input		= gpio_input( SENSOR_2_NC_PORT, SENSOR_2_NC_PIN );
		no_input		= gpio_input( SENSOR_2_NO_PORT, SENSOR_2_NO_PIN );
		success_counter	= &success_counter_2;
		port			= SENSOR_2_COM_PORT;
		pin				= SENSOR_2_COM_PIN;
		sensor_status	= &(status.sensor_2);
		break;
	default:
		return;			//invalid sensor ID, return without any action.
	}

	//if sensor is state is normal, the common output should be equivalent to the normally closed input
	//if the sensor is in an alarm state, the common output should be equivalent to the normally open input
	if( com_output == nc_input )
	{
		(*success_counter)++;
		if( *success_counter > 25 )
		{
			*sensor_status = NORMAL;
		}
	}
	else if( com_output == no_input )
	{
		*success_counter = 0;
		*sensor_status = ALARM;
	}
	else
	{
		*success_counter = 0;
	}

	//update pin output
	if( com_output == 0 )
		gpio_output_high( port, pin );
	else
		gpio_output_low( port, pin );
}


void pump_monitor( void )
{
	static uint32_t active_delay;
	int pump_output = gpio_output( PUMP_P_PORT, PUMP_P_PIN );
	int pump_input = gpio_input( PUMP_M_PORT, PUMP_M_PIN );

	if( pump_output == pump_input )
	{
		active_delay++;
		if( active_delay > 2 * MEASUREMENT_FREQUENCY * PUMP_DELAY )
		{
			status.pump = ACTIVE;
		}
	}
	else
	{
		active_delay = 0;
		status.pump = INACTIVE;
	}

	if( pump_output )
		gpio_output_low( PUMP_P_PORT, PUMP_P_PIN );
	else
		gpio_output_high( PUMP_P_PORT, PUMP_P_PIN );
}

void led_color( led_color_t color )
{
	if( color == RED )
	{
		gpio_output_low( LED_OUTPUT_AQUA_PORT, LED_OUTPUT_AQUA_PIN );
		gpio_output_high( LED_OUTPUT_RED_PORT, LED_OUTPUT_RED_PIN );
	}
	else if( color == AQUA )
	{
		gpio_output_low( LED_OUTPUT_RED_PORT, LED_OUTPUT_RED_PIN );
		gpio_output_high( LED_OUTPUT_AQUA_PORT, LED_OUTPUT_AQUA_PIN );
	}
}

void led_control( void )
{
	if( status.pump == ACTIVE )
	{
		if( status.sensor_1 == NORMAL && status.sensor_2 == NORMAL )
		{
			led_color( AQUA );
		}
		else
		{
			led_color( RED );
		}
	}
}



void ultra_pure_init( void )
{
	FLASH->ACR |= FLASH_ACR_LATENCY;						//Set the flash latency to one wait state, required to allow sufficient flash access time when system clock is greater than 24MHz

	RCC->CFGR &= ~RCC_CFGR_PLLSRC;						//Configure PLL input as the high speed internal clock (HSI 8MHz) divided by two.
	RCC->CFGR |= RCC_CFGR_PLLMUL12;						//Configure PLL output to multiply the input by 12 (48MHz).
	RCC->CR 	|= RCC_CR_PLLON;							//Enable PLL.
	while( !(RCC->CR & RCC_CR_PLLRDY) );					//Wait until the PLL has stabilized, indicated by the PLL Ready flag.
	RCC->CFGR &= ~RCC_CFGR_SW;							//Clear System Clock Selection Bits
	RCC->CFGR |= RCC_CFGR_SW_1;							//Select PLL output as System Clock
	while( !(RCC->CFGR & RCC_CFGR_SWS_1) );				//Wait until PLL is established as the system clock
	SysTick_Config( 48E6 / (2 * MEASUREMENT_FREQUENCY) );	//Enable SysTick interrupts 2000 times per second.

	RCC->AHBENR	|= (RCC_AHBENR_GPIOAEN);					//Enable clock to used pin ports

	gpio_config_output( PUMP_P_PORT, PUMP_P_PIN );
	gpio_config_input( PUMP_M_PORT, PUMP_M_PIN );

	gpio_config_output( SENSOR_1_COM_PORT, SENSOR_1_COM_PIN );
	gpio_config_input( SENSOR_1_NO_PORT, SENSOR_1_NO_PIN );
	gpio_config_input( SENSOR_1_NC_PORT, SENSOR_1_NC_PIN );

	gpio_config_output( SENSOR_2_COM_PORT, SENSOR_2_COM_PIN );
	gpio_config_input( SENSOR_2_NO_PORT, SENSOR_2_NO_PIN );
	gpio_config_input( SENSOR_2_NC_PORT, SENSOR_2_NC_PIN );

	gpio_config_output( LED_OUTPUT_RED_PORT, LED_OUTPUT_RED_PIN );
	gpio_config_output( LED_OUTPUT_AQUA_PORT, LED_OUTPUT_AQUA_PIN );

	status.pump = ACTIVE;
	status.sensor_1 = NORMAL;
	status.sensor_2 = NORMAL;
}


