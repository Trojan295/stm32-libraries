#include <stm32f0xx.h>
#include "usart.h"

void usart_init(uint32_t baudrate) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	GPIOA->MODER |= (1 << 5) | (1 << 7);
	GPIOA->AFR[0] |= (1 << 8) | (1 << 12);

	USART2->BRR = SystemCoreClock / baudrate;
	USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 4);
}

void usart_send_byte(uint8_t byte) {
	while ((USART2->ISR & USART_ISR_TC) != USART_ISR_TC);
	USART2->TDR = byte;
}

void usart_send_string(uint8_t* str) {
	while (*str) {
		usart_send_byte(*str++);
	}
}
