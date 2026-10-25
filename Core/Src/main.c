/* CPE 3160
 * John Oliver
 *
 * P1 - Lock
 * Chris Amey and Alan Kusparmakov
 *
 * Video: 
 * https://drive.google.com/file/d/1hArrJB_lnd-JMD0XwG729pXTaAC3bJnJ/view?usp=sharing
 * 
 * Schematic: 
 * https://drive.google.com/file/d/1SX4DZLqecqLpMrKSfqTl6fLLpEubJk_C/view?usp=sharing
 */

#include "init.h"
#include "keypad.h"
#include "led.h"
#include "lcd.h"
#include "stdbool.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define INPUT_STAR 10
#define INPUT_POUND 15

#define LINE_WIDTH 16

typedef enum { ALTERNATIVE = 2 } GpioMode;

void init()
{
	// gpio_init();
	// LCD_init(true, false, true, true, true, false);
}

// configure microcontroller clock output on PA8
void set_PA8_clock_MSI()
{
	// enable GPIOA
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// unselect all clocks
	// I think "selection" is just for blanket-setting an option to multiple clocks if you need to
	RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_MCOSEL)));

	// select 0010 = MSI system clock
	RCC->CFGR |= RCC_CFGR_MCOSEL_1;

	// now configure selected clock(s)

	// enable GPIOA
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// set mode
	GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
	GPIOA->MODER |= (ALTERNATIVE << GPIO_MODER_MODE8_Pos);

	// allows STM to actively drive either low or high
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8); // Push-pull output


	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD8); // no resistor
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED8); // high speed

	// Set Pin 8's Alternate Function to be MCO
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL8); // select MCO function
}

void TIM2_init()
{
	// APB1 = advanced peripheral bus
	// enable the CPU to TIM2 connection for configuring.
	// also connects a peripheral clock to TIM2
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN_Msk);

	// disable timer while configuring
	TIM2->CR1 &= ~(TIM_CR1_CEN_Msk);

	// CR1.DIR is up count by default
	// this is what we want

	TIM2->PSC = 0;
	TIM2->ARR = 799;
	
	// start at a known val
	TIM2->CNT = 0;

	// enable update interrupt	
	// goes off whenever CNT == ARR
	TIM2->DIER |= TIM_DIER_UIE_Msk;

	// enable CCR1
	// goes off whenever CNT = CCR1
	TIM2->DIER |=TIM_DIER_CC1IE;

	// set the value to 25% of the duty cycle (800 / 4)
	TIM2->CCR1 = 200;
	
	// initialize the update interrupt to zero
	TIM2->SR &= ~(TIM_SR_UIF_Msk);

	// enable TIM2 in the NVIC
	// TIM2 is ISER[0] within NVIC. 
	NVIC->ISER[0] = 1 << TIM2_IRQn;

	// can also do this
	// NVIC_EnableIRQ(TIM2_IRQn);

	// re-enable timer
	TIM2->CR1 |= TIM_CR1_CEN_Msk;

	// terminology:
	// DIER enable timer interrupt requests?
	// EGR generate timer events by software?
	// CR counter enable
	// UIE - update interrupt enable
	// CEN - counter enable
}

int main()
{
	// init();
	// TIM2_init();
	set_PA8_clock_MSI();

	while (true);
}

// void TIM2_IRQHandler() {
// 	if (TIM2->SR & TIM_SR_UIF) {
// 		// ....
// 		// set output to high
//
//
//
// 		// always clear flag
// 		TIM2->SR &= ~(TIM_SR_UIF);
//
// 		return;
// 	}
//
// 	if (TIM2->SR & TIM_SR_CC1IF) {
//
// 		//set output to low
//
//
// 		// clear flag
// 		TIM2->SR &= ~(TIM_SR_CC1IF);
// 		return;
// 	}
// }
