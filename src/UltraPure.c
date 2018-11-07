/*
 * UltraPure.c
 *
 *  Created on: Nov 5, 2018
 *      Author: Tanner
 */

#include "stm32f0xx.h"
#include "UltraPure.h"


volatile ultra_pure_status_t status;


static void gpio_config_input( GPIO_TypeDef * port, uint8_t pin )
{
	port->MODER		&=	~(3 << (pin * 2));		//clear pin mode selection, default to input mode
	port->PUPDR		&=	~(3 << (pin * 2));		//disable pull resistors
}

static void gpio_config_output( GPIO_TypeDef * port, uint8_t pin )
{
	port->MODER	&=	~(3 << (pin * 2));		//clear pin mode selection
	port->MODER	|=	1 << (pin * 2);			//set pin mode to output
	port->OTYPER	&=	~(1 << pin);			//set output type to push-pull
	port->OSPEEDR	|=	~(3 << (pin * 2));		//set output speed to low
	port->PUPDR	&=	~(3 << (pin * 2));		//disable pull resistors
	port->BSRR	=	1 << (pin + 16);		//output low
}

static void gpio_output_high( GPIO_TypeDef * port, uint8_t pin )
{
	port->BSRR	=	1 << pin;				//output high
}

static void gpio_output_low( GPIO_TypeDef * port, uint8_t pin )
{
	port->BSRR	=	1 << (pin + 16);				//output high
}

static int gpio_output( GPIO_TypeDef * port, uint8_t pin )
{
	return ( port->ODR & (1 << pin) );
}

static int gpio_input( GPIO_TypeDef * port, uint8_t pin )
{
	return ( port->IDR & (1 << pin) );
}


void ultra_pure_init( void )
{
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



void sensor_1_monitor( void )
{
	int com_output 	= gpio_output( SENSOR_1_COM_PORT, SENSOR_1_COM_PIN );
	int nc_input	= gpio_input( SENSOR_1_NC_PORT, SENSOR_1_NC_PIN );
	int no_input	= gpio_input( SENSOR_1_NO_PORT, SENSOR_1_NO_PIN );

	if( com_output == nc_input )
	{
		if( com_output != no_input )
		{
			status.sensor_1 = NORMAL;
		}
		else
		{
			//ambiguous reading... not sure how to respond
		}
	}
	else if( com_output == no_input )
	{
		status.sensor_1 = ALARM;
	}
	else
	{
		//ambiguous reading
	}

	if( com_output )
		gpio_output_low( SENSOR_1_COM_PORT, SENSOR_1_COM_PIN );
	else
		gpio_output_high( SENSOR_1_COM_PORT, SENSOR_1_COM_PIN );
}



void sensor_2_monitor( void )
{
	int com_output 	= gpio_output( SENSOR_2_COM_PORT, SENSOR_2_COM_PIN );
	int nc_input	= gpio_input( SENSOR_2_NC_PORT, SENSOR_2_NC_PIN );
	int no_input	= gpio_input( SENSOR_2_NO_PORT, SENSOR_2_NO_PIN );

	if( com_output == nc_input )
	{
		if( com_output != no_input )
		{
			status.sensor_2 = NORMAL;
		}
		else
		{
			//ambiguous reading... not sure how to respond
		}
	}
	else if( com_output == no_input )
	{
		status.sensor_2 = ALARM;
	}
	else
	{
		//ambiguous reading
	}

	if( com_output )
		gpio_output_low( SENSOR_2_COM_PORT, SENSOR_2_COM_PIN );
	else
		gpio_output_high( SENSOR_2_COM_PORT, SENSOR_2_COM_PIN );
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



