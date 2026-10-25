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
	gpio_init();
	LCD_init(true, false, true, true, true, false);
}

// configure microcontroller clock output on PA8
void set_PA8_clock_MSI()
{
	// unselect all clocks
	RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_MCOSEL)));

	// select 0010 = MSI system clock
	RCC->CFGR |= RCC_CFGR_MCOSEL_1;

	// configure selected clock(s)

	// enable GPIOA
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// set mode
	GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
	GPIOA->MODER |= (ALTERNATIVE << GPIO_MODER_MODE8_Pos);

	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8); // Push-pull output
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD8); // no resistor
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED8); // high speed

	// Set Pin 8's Alternate Function to be MCO
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL8); // select MCO function
}

void TIM2_enable()
{
	// APB1 = advanced peripheral bus
	// enable the CPU to TIM2 connection for configuring.
	// also connects a peripheral clock to TIM2
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN_Msk);

	// disable timer while configuring
	TIM2->CR1 &= ~(TIM_CR1_CEN_Msk);

	TIM2->PSC = 0;
	TIM2->ARR = 799;
	
	// start at a known val
	TIM2->CNT = 0;

	// enable update interrupt	
	// goes off whenever CNT == ARR
	TIM2->DIER |= TIM_DIER_UIE_Msk;
	
	// initialize the update interrupt to zero
	TIM2->SR &= ~(TIM_SR_UIF_Msk);

	// enable TIM2 in the NVIC
	// TIM2 is ISER[0] within NVIC. 
	NVIC->ISER[0] = 1 << TIM2_IRQn;

	// can also do this
	// NVIC_EnableIRQ(TIM2_IRQn);

	// re-enable timer
	TIM2->CR1 |= TIM_CR1_CEN_Msk;

	// DIER enable timer interrupt requests?
	// EGR generate timer events by software?
	// CR counter enable
	// UIE - update interrupt enable
	// CEN - counter enable
}

int main()
{
	init();
	welcome_message();
	LED_turn_on();

	HAL_Delay(3000);

	// enough space for longest string and a null terminator
	char password[LINE_WIDTH + 1] = { 0 };
	char input[LINE_WIDTH + 1] = { 0 };

	strcpy(password, "1234");

	int keyInput = 0;

	enum State state = LOCKED;

	while (1) {
		status_message(state, input);

		keyInput = get_pressed_key();
		while (keyInput == INVALID_VALUE) {
			keyInput = get_pressed_key();
			HAL_Delay(25);
			continue;
		}

		switch (state) {
		case LOCKED:
			if (keyInput == INPUT_STAR) {
				buffer_reset(input);
				continue;
			}

			if (keyInput == INPUT_POUND) {
				buffer_backspace(input);
				continue;
			}

			buffer_append(input, '0' + keyInput);

			if (strcmp(input, password) == 0) {
				state = UNLOCKED;
			}

			break;

		case UNLOCKED:
			// buffer is empty while unlocked
			memset(input, 0, sizeof(input));

			if (keyInput == INPUT_STAR) {
				state = LOCKED;
				continue;
			}

			if (keyInput == INPUT_POUND) {
				state = CHANGING;
				continue;
			}

			break;

		case CHANGING:
			// change password
			int len = strlen(input);
			if (keyInput == INPUT_POUND) {
				if (len == 0) {
					state = LOCKED;
				}

				buffer_backspace(input);
				continue;
			}

			if (keyInput == INPUT_STAR) {
				if (len < 4)
					continue;

				strcpy(password, input);
				buffer_reset(input);
				state = LOCKED;
				continue;
			}

			buffer_append(input, '0' + keyInput);
		}
	}
}

void TIM2_IRQHandler() {
	if (TIM2->SR & TIM_SR_UIF) {
		// ....

		
		TIM2->SR &= ~(TIM_SR_UIF);
	}
}
