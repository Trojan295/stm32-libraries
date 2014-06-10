#include "stm32f0xx.h"
#include "hd44780.h"
#include <stdarg.h>

static char x;
static char y;
static volatile int LCD_Counter;

void LCD_WriteText(char* text);
void LCD_WriteCommand(unsigned char command);
void LCD_WriteChar(unsigned char command);
void LCD_WriteDPort(unsigned char command);
unsigned char LCD_ReadDPort();
unsigned char LCD_ReadBusyFlag();
void LCD_SetCursor(unsigned char x, unsigned char y);
int doubleToStr(int x, char *ptr);
int intToStr(int x, char *ptr);
int addString(char *text, char *ptr);
void reset_position();
void delay(int ms);

void LCD_Init(void) {

	RCC->AHBENR |= PORT_CLK_ENABLE;
	PORT->MODER |= PIN_D4_MODER | PIN_D5_MODER | PIN_D6_MODER | PIN_D7_MODER
			| PIN_E_MODER | PIN_RS_MODER | PIN_RW_MODER;

delay(20000000);
	PORT->ODR &= ~PIN_E & ~PIN_RS & ~PIN_RW;

	delay(20000000);
	
	LCD_WriteDPort(0x02); // 4 bit
	delay(20000000);
	LCD_WriteCommand(0x28); // ustawienie interfejsu 4bit, 2line, 5x8matrix
	LCD_WriteCommand(0x0C); // display on, cursor off, blink off
	LCD_WriteCommand(0x01);	// clear
	reset_position();
}

void LCD_printf(const char *format, ...) {
	char text[50];
	char *ptr = text;
	va_list arg;
	va_start(arg, format);
	for (; *format; format++) {
		if (*format == '%') {
			format++;
			switch (*format) {
			case 's':
				ptr += addString(va_arg(arg, char*),ptr);
				break;
			case 'd':
				ptr += intToStr(va_arg(arg, int),ptr);
				break;
			case 'f':
				ptr += doubleToStr(va_arg(arg, int),ptr);
				break;
			default:
				*ptr = *format;
				ptr++;
				break;
			}
		} else {
			*ptr = *format;
			ptr++;
		}
	}
	*ptr = 0;
	LCD_WriteText(text);

	va_end(arg);
}

void LCD_DecCounter() {
	if (LCD_Counter > 0)
		LCD_Counter--;
}

void LCD_WriteText(char* text) {
	LCD_WriteCommand(0x01);
	reset_position();
	LCD_SetCursor(0, 0);
	while (*text) {
		if (*text == '\n') {
			LCD_SetCursor(0, 1);
			text++;
		} else {
			LCD_WriteChar(*text++);
		}
	}
}

void LCD_WriteCommand(unsigned char command) {
	PORT->BRR = PIN_RS;
	PORT->BRR = PIN_RW;

	LCD_WriteDPort(command >> 4);
	LCD_WriteDPort(command);

	while (LCD_ReadBusyFlag() & 0x80) {
	}
}

void LCD_WriteChar(unsigned char c) {
	if (y >= HEIGHT)
		return;

	PORT->BSRR = PIN_RS;
	PORT->BRR = PIN_RW;

	LCD_WriteDPort(c >> 4);
	LCD_WriteDPort(c);

	//Busy flag
	while (LCD_ReadBusyFlag() & 0x80) {
	}

	x++;

	if (x == WIDTH) {
		x = 0;
		y++;
		LCD_SetCursor(0, y);
	}
}

unsigned char LCD_ReadBusyFlag() {
	unsigned char status = 0;

	PORT->MODER &=
			~PIN_D7_MODER & ~PIN_D6_MODER & ~PIN_D5_MODER & ~PIN_D4_MODER;

	PORT->BRR = PIN_RS;
	PORT->BSRR = PIN_RW;

	status |= (LCD_ReadDPort() << 4);
	status |= LCD_ReadDPort();

	PORT->MODER |= PIN_D4_MODER | PIN_D5_MODER | PIN_D6_MODER | PIN_D7_MODER;

	return (status);
}

void LCD_WriteDPort(unsigned char command) {
	PORT->BSRR = PIN_E;
	if (command & 0x01)
		PORT->BSRR = PIN_D4;
	else
		PORT->BRR = PIN_D4;
	if (command & 0x02)
		PORT->BSRR = PIN_D5;
	else
		PORT->BRR = PIN_D5;
	if (command & 0x04)
		PORT->BSRR = PIN_D6;
	else
		PORT->BRR = PIN_D6;
	if (command & 0x08)
		PORT->BSRR = PIN_D7;
	else
		PORT->BRR = PIN_D7;
	PORT->BRR = PIN_E;
}

unsigned char LCD_ReadDPort() {
	unsigned char status = 0;

	PORT->BSRR = PIN_E;
	if (PORT->IDR & PIN_D4)
		status |= (1 << 0);
	if (PORT->IDR & PIN_D5)
		status |= (1 << 1);
	if (PORT->IDR & PIN_D6)
		status |= (1 << 2);
	if (PORT->IDR & PIN_D7)
		status |= (1 << 3);
	PORT->BRR = PIN_E;

	return status;
}

void LCD_SetCursor(unsigned char x, unsigned char y) {
	int i;
	LCD_WriteCommand(0x02);
	for (i = 0; i < 40 * y; i++) {
		LCD_WriteCommand(0x14);
	}
	for (i = 0; i < x; i++) {
		LCD_WriteCommand(0x14);
	}
}

int doubleToStr(int x, char *ptr) {
	char tempBuf[20] = "";
	int i = 0;

	do {
		tempBuf[i++] = '0' + x % 10;
		x = x / 10;
	} while (x);
	for (x = 4; x > i;)
		tempBuf[i++] = '0';

	//i - wielkosc tablicy, tablica jest odwrotnie
	for (x = i; x > 2; x--) {
		tempBuf[x + 1] = tempBuf[x];
	}
	tempBuf[3] = '.';

	for (x = 0; x <= i; x++) {
		ptr[x] = tempBuf[i - x];
	}
	return i + 1;
}

int intToStr(int x, char *ptr) {
	char tempBuf[20] = "";
	int i = 0;

	do {
		tempBuf[i++] = '0' + x % 10;
		x = x / 10;
	} while (x);

	for (x = 0; x < i; x++) {
		ptr[x] = tempBuf[i - x - 1];
	}
	return i;
}

int addString(char *text, char *ptr) {
	int i = 0;

	while (*text) {
		*ptr = *text;
		text++;
		ptr++;
		i++;
	}

	return i;
}

void reset_position() {
	x = 0;
	y = 0;
}

void delay(int ms) {
	int i,j,k,l;
	for (i=0;i<200000000;i++) {
		for (k=0;k<200000000;k++) {
			for (l=0;l<200000000;l++) {
				for (j=0;j<ms;j++) {
				}
			}
		}
	}
}
