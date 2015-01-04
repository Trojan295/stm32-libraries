#ifndef USART_H_
#define USART_H_

#include <stdint-gcc.h>

void usart_init(uint32_t baudrate);
void usart_send_byte(uint8_t byte);
void usart_send_string(uint8_t* str);


#endif /* USART_H_ */
