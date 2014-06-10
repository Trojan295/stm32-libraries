#include "stm32f0xx.h"

//Edit the defines

#define PORT GPIOB
#define PORT_CLK_ENABLE RCC_AHBENR_GPIOBEN

#define PIN_RS_MODER GPIO_MODER_MODER3_0
#define PIN_RW_MODER GPIO_MODER_MODER4_0
#define PIN_E_MODER GPIO_MODER_MODER5_0
#define PIN_D4_MODER GPIO_MODER_MODER6_0
#define PIN_D5_MODER GPIO_MODER_MODER7_0
#define PIN_D6_MODER GPIO_MODER_MODER8_0
#define PIN_D7_MODER GPIO_MODER_MODER9_0

#define PIN_RS	GPIO_ODR_3
#define PIN_RW	GPIO_ODR_4
#define PIN_E	GPIO_ODR_5
#define PIN_D4	GPIO_ODR_6
#define PIN_D5	GPIO_ODR_7
#define PIN_D6	GPIO_ODR_8
#define PIN_D7	GPIO_ODR_9

#define WIDTH 16
#define HEIGHT 2

void LCD_Init(void);	//Funkcja inicjująca LCD
void LCD_printf(const char *format, ...); //Funkcja wyswietlajaca tekst
void LCD_DecCounter(); //Obsługa delaya (powinna być wywoływana co 1 sek)
