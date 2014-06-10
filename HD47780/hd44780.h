//Edit the defines below

#include "stm32f0xx.h"

//Port conf
#define PORT GPIOB
#define PORT_CLK_ENABLE RCC_AHBENR_GPIOBEN

//Output conf
#define PIN_RS_MODER GPIO_MODER_MODER3_0
#define PIN_RW_MODER GPIO_MODER_MODER4_0
#define PIN_E_MODER GPIO_MODER_MODER5_0
#define PIN_D4_MODER GPIO_MODER_MODER6_0
#define PIN_D5_MODER GPIO_MODER_MODER7_0
#define PIN_D6_MODER GPIO_MODER_MODER8_0
#define PIN_D7_MODER GPIO_MODER_MODER9_0

//Pin state control
#define PIN_RS	GPIO_ODR_3
#define PIN_RW	GPIO_ODR_4
#define PIN_E	GPIO_ODR_5
#define PIN_D4	GPIO_ODR_6
#define PIN_D5	GPIO_ODR_7
#define PIN_D6	GPIO_ODR_8
#define PIN_D7	GPIO_ODR_9

//LCD size
#define WIDTH 16
#define HEIGHT 2

void LCD_Init(void);	// LCD init

void LCD_printf(const char *format, ...); /*Writes text on the LCD display. Works like printf,
										but only with %d, %f, %s.
										Before sending text data to the LCD it clears the whole
										display and set the cursor position to the beginning*/
										
void LCD_DecCounter(); // Delay handler, should be called every 1 ms
