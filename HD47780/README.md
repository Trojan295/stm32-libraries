Simple HD44780 LCD driver for STM32F0xx uC.
With a few modification it can be used on other uC as well.

It contains only 2 functions:

LCD_Init();
LCD_printf(const char *format, ...);

To use this library as first you need to call LCD_Init(). It initialize
the LCD display (4bit, 2 lines, 5x8 matrix, can be changed in the
hd44780.c file).

LCD_printf() works like the standard printf, but it parses only int,
char and float/double. (%d,%s,%f). Double/float numbers are shown with 
3 digits precision. What's important, every call clears the display
and sets the cursor position to the beginning, so now, there's no option
to append some text.
