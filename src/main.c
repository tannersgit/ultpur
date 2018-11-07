/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.0.0   2018-11-05

The MIT License (MIT)
Copyright (c) 2018 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include "stm32f0xx.h"
#include "UltraPure.h"


/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	FLASH -> ACR |= FLASH_ACR_LATENCY;						//Set the flash latency to one wait state, required to allow sufficient flash access time when system clock is greater than 24MHz

	RCC -> CFGR &= ~RCC_CFGR_PLLSRC;						//Configure PLL input as the high speed internal clock (HSI 8MHz) divided by two.
	RCC -> CFGR |= RCC_CFGR_PLLMUL12;						//Configure PLL output to multiply the input by 12 (48MHz).
	RCC -> CR 	|= RCC_CR_PLLON;							//Enable PLL.
	while( !(RCC -> CR & RCC_CR_PLLRDY) );					//Wait until the PLL has stabilized, indicated by the PLL Ready flag.
	RCC -> CFGR &= ~RCC_CFGR_SW;							//Clear System Clock Selection Bits
	RCC -> CFGR |= RCC_CFGR_SW_1;							//Select PLL output as System Clock
	while( !(RCC -> CFGR & RCC_CFGR_SWS_1) );				//Wait until PLL is established as the system clock
	SysTick_Config( 48E6 / (2 * MEASUREMENT_FREQUENCY) );	//Enable SysTick interrupts 2000 times per second.

	ultra_pure_init();

	/* Infinite loop */
	while (1)
	{

	}
}

/* SYSTICK INTERRUPT ROUTINE */
void SysTick_Handler( void )
{
	static uint32_t counter = 0;

	if( (counter % 200) < 100 )
	{
		sensor_1_monitor();
	}
	else
	{
		sensor_2_monitor();
	}

	pump_monitor();

	led_control();

	counter++;
}
