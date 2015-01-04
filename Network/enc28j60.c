/*
 * enc28j60.c
 *
 *  Created on: 28 gru 2014
 *      Author: damian
 */

#include <network/enc28j60.h>
#include <stm32f4xx.h>

#define spi_read() spi_send_byte(0xff)
#define spi_write(x) spi_send_byte(x)

uint16_t next_packet_ptr;

void spi_chip_select(uint8_t select) {
	if (select)
		GPIOD->BSRRH = (1 << 0);
	else
		GPIOD->BSRRL = (1 << 0);
}

void spi_init() {
	/*
	 * PD0 - CS
	 * PC10 - SCK
	 * PC11 - MISO
	 * PC12 - MOSI
	 * all AF6
	 */

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->BSRRL = (1 << 0);
	GPIOD->MODER |= (1 << 0);
	GPIOD->PUPDR |= (1 << 0);

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	GPIOC->MODER |= (2 << 20) | (2 << 22) | (2 << 24);	//AF
	GPIOC->AFR[1] |= (6 << 8) | (6 << 12) | (6 << 16);	//AF6

	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	//SPI3->CR1 |= (1 << 3); // CLK / 256;
	SPI3->CR1 |= (1 << 0); //CPHA
	SPI3->CR1 |= (1 << 9) | (1 << 8); //set SSM and SSI
	SPI3->CR1 |= (1 << 2) | (1 << 6); //master and enable
}

uint8_t spi_send_byte(uint8_t byte) {
	while ((SPI3->SR & SPI_SR_TXE) != SPI_SR_TXE)
		;

	SPI3->DR = byte;

	while ((SPI3->SR & SPI_SR_RXNE) != SPI_SR_RXNE)
		;

	return SPI3->DR;
}

void enc_bank_select(uint8_t address) {
	uint8_t bank = (address & BANK_MASK) >> 5;

	spi_chip_select(1);
	uint8_t cmd = (5 << 5) | ECON1;
	spi_write(cmd);
	spi_write(0x03);
	spi_chip_select(0);

	spi_chip_select(1);
	cmd = (4 << 5) | ECON1;
	spi_write(cmd);
	spi_write(bank);
	spi_chip_select(0);
}

void enc_reset() {
	spi_chip_select(1);
	spi_write(0xff);
	spi_chip_select(0);
	for (uint16_t i = 0; i < 62000; i++)
		;
}

void enc_bfs(uint8_t address, uint8_t data) {
	enc_bank_select(address);

	spi_chip_select(1);
	uint8_t cmd = (4 << 5) | (ADDR_MASK & address);
	spi_write(cmd);
	spi_write(data);
	spi_chip_select(0);
}

void enc_bfc(uint8_t address, uint8_t data) {
	enc_bank_select(address);

	spi_chip_select(1);
	uint8_t cmd = (5 << 5) | (ADDR_MASK & address);
	spi_write(cmd);
	spi_write(data);
	spi_chip_select(0);
}

uint8_t enc_rcr(uint8_t address) {
	enc_bank_select(address);
	spi_chip_select(1);
	uint8_t cmd = ADDR_MASK & address;
	spi_write(cmd);

	if (address & SPRD_MASK)
		spi_read();

	uint8_t result = spi_read();
	spi_chip_select(0);
	return result;
}

void enc_wcr(uint8_t address, uint8_t data) {
	enc_bank_select(address);
	spi_chip_select(1);
	uint8_t cmd = (1 << 6) | (ADDR_MASK & address);
	spi_write(cmd);
	spi_write(data);
	spi_chip_select(0);
}

uint16_t enc_rcr2(uint8_t address) {
	uint16_t result = 0;
	result = (enc_rcr(address + 1) << 8) | enc_rcr(address);
	return result;
}

void enc_wcr2(uint8_t address, uint16_t data) {
	enc_wcr(address, (uint8_t) data);
	enc_wcr(address + 1, (uint8_t) (data >> 8));
}

void enc_wbm(uint8_t *ptr_data, uint16_t length) {
	spi_chip_select(1);
	spi_write(0x7a);
	while (length--)
		spi_write(*(ptr_data++));
	spi_chip_select(0);
}

void enc_rbm(uint8_t *buffer, uint16_t length) {
	spi_chip_select(1);
	spi_write(0x3a);
	while (length--)
		*(buffer++) = spi_read();
	spi_chip_select(0);
}

uint16_t enc_rphy(uint8_t address) {
	enc_wcr(MIREGADR, address);
	enc_bfs(MICMD, MICMD_MIIRD);
	while (enc_rcr(MISTAT) & MISTAT_BUSY)
		;
	enc_bfc(MICMD, MICMD_MIIRD);
	return enc_rcr2(MIRDL);
}

void enc_wphy(uint8_t address, uint16_t data) {
	enc_wcr(MIREGADR, address);
	enc_wcr2(MIWRL, data);
	while (enc_rcr(MISTAT) & MISTAT_BUSY)
		;
}

void enc_init(uint8_t *mac_addr) {
	spi_init();

	enc_reset();

	enc_wcr2(ERXSTL, ENC28J60_RXSTART);
	enc_rcr2(ERXSTL);
	enc_wcr2(ERXRDPTL, ENC28J60_RXSTART);
	enc_wcr2(ERXNDL, ENC28J60_RXEND);
	next_packet_ptr = ENC28J60_RXSTART;

	enc_wcr(EIE, EIE_PKTIE | EIE_INTIE);

	// Setup MAC
	enc_wcr(MACON1, MACON1_TXPAUS | // Enable flow control
			MACON1_RXPAUS | MACON1_MARXEN); // Enable MAC Rx
	enc_wcr(MACON2, 0); // Clear reset
	enc_wcr(MACON3, MACON3_PADCFG0 | // Enable padding,
			MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX); // Enable crc & frame len chk
	enc_wcr2(MAMXFLL, ENC28J60_MAXFRAME);
	enc_wcr(MABBIPG, 0x15); // Set inter-frame gap
	enc_wcr(MAIPGL, 0x12);
	enc_wcr(MAIPGH, 0x0c);
	enc_wcr(MAADR5, mac_addr[0]); // Set MAC address
	enc_wcr(MAADR4, mac_addr[1]);
	enc_wcr(MAADR3, mac_addr[2]);
	enc_wcr(MAADR2, mac_addr[3]);
	enc_wcr(MAADR1, mac_addr[4]);
	enc_wcr(MAADR0, mac_addr[5]);
	// Setup PHY
	enc_wphy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	enc_wphy(PHCON2, PHCON2_HDLDIS); // Disable loopback
	enc_wphy(PHLCON, PHLCON_LACFG2 | // Configure LED ctrl
			PHLCON_LBCFG2 | PHLCON_LBCFG1 | PHLCON_LBCFG0 |
			PHLCON_LFRQ0 | PHLCON_STRCH);
	// Enable Rx packets
	enc_bfs(ECON1, ECON1_RXEN);
}

void enc_send(uint8_t* packet, uint16_t len) {

	while (enc_rcr(ECON1) & ECON1_TXRTS) {
		if (enc_rcr(EIR) & EIR_TXERIF) {
			enc_bfs(ECON1, ECON1_TXRST);
			enc_bfc(ECON1, ECON1_TXRST);
		}
	}
	enc_wcr2(EWRPTL, ENC28J60_TXSTART);
	enc_wbm((uint8_t*) "\x00", 1);
	enc_wbm(packet, len);
	enc_wcr2(ETXSTL, ENC28J60_TXSTART);
	enc_wcr2(ETXNDL, ENC28J60_TXSTART + len);
	enc_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

uint16_t enc_recv(uint8_t* packet, uint16_t buflen) {
	uint16_t len = 0, rxlen, status, temp;
	if (enc_rcr(EPKTCNT)) {
		enc_wcr2(ERDPTL, next_packet_ptr);
		enc_rbm((void*) &next_packet_ptr, sizeof(next_packet_ptr));
		enc_rbm((void*) &rxlen, sizeof(rxlen));
		enc_rbm((void*) &status, sizeof(status));
		if (status & 0x80) //success
				{
			len = rxlen - 4; //throw out crc
			if (len > buflen)
				len = buflen;
			enc_rbm(packet, len);
		}
		// Set Rx read pointer to next packet
		temp = (next_packet_ptr - 1) & ENC28J60_BUFEND;
		enc_wcr2(ERXRDPTL, temp);
		// Decrement packet counter
		enc_bfs(ECON2, ECON2_PKTDEC);
	}
	return len;
}
