#include <rfm73.h>
#include <stdint.h>
#include <stm32f0xx.h>

// RFM73 SPI read and write commands
#define RFM73_CMD_READ_REG            0x00
#define RFM73_CMD_WRITE_REG           0x20

//interrupt status
#define STATUS_RX_DR    0x40
#define STATUS_TX_DS    0x20
#define STATUS_MAX_RT   0x10

#define STATUS_TX_FULL  0x01

//FIFO_STATUS
#define FIFO_STATUS_TX_REUSE  0x40
#define FIFO_STATUS_TX_FULL   0x20
#define FIFO_STATUS_TX_EMPTY  0x10

#define FIFO_STATUS_RX_FULL   0x02
#define FIFO_STATUS_RX_EMPTY  0x01

void RFM73_CSN(char i) {
	if (i > 0)
		GPIOB->BSRR = (1 << 6);
	else
		GPIOB->BRR = (1 << 6);

}

void RFM73_CE(char i) {
	if (i > 0)
		GPIOB->BSRR = (1 << 7);
	else
		GPIOB->BRR = (1 << 7);
}

void RFM73_WAIT_US(char a) {
	for (int ii = 0; ii < a; ii++)
		for (int i = 0; i < 1000; i++)
			;
}

// Bank0 register initialization values
#define BANK0_ENTRIES 10
const unsigned char Bank0_Reg[BANK0_ENTRIES][2] = { { 0, 0x2F }, // receive, enabled, CRC 2, enable interupts
		{ 1, 0x3F }, // auto-ack on all pipes enabled
		{ 2, 0x03 }, // Enable pipes 0 and 1
		{ 3, 0x03 }, // 5 bytes addresses
		{ 4, 0xff }, // auto retransmission delay 4000 ms, 15 times
		{ 5, 0x0A }, // channel 10
		{ 6, 0x07 }, // data rate 1Mbit, power 5dbm, LNA gain high
		{ 7, 0x07 }, // why write this at all?? but seems required to work...
		{ 8, 0x00 }, // clear Tx packet counters
		{ 23, 0x00 }, // fifo status
		};

// default receive address data pipe 0:
// just a bunch of bytes, nothing magical
const unsigned char RX0_Address[] = { 0x34, 0x43, 0x10, 0x10, 0x01 };

void SPI_SendData8(SPI_TypeDef* SPIx, uint8_t Data) {
	uint32_t spixbase = 0x00;
	spixbase = (uint32_t) SPIx;
	spixbase += 0x0C;
	*(__IO uint8_t *) spixbase = Data;
}

uint8_t SPI_ReceiveData8(SPI_TypeDef* SPIx) {
	uint32_t spixbase = 0x00;
	spixbase = (uint32_t) SPIx;
	spixbase += 0x0C;
	return *(__IO uint8_t *) spixbase;
}

unsigned char rfm73_SPI_RW(unsigned char value) {

	uint8_t temp = 0;
	while (!(SPI1->SR & SPI_SR_TXE))
		;
	SPI_SendData8(SPI1, value);
	while (!(SPI1->SR & SPI_SR_RXNE))
		;
	temp = SPI_ReceiveData8(SPI1);
	for (int i = 0; i < 100; i++)
		;
	return temp;
}

void rfm73_register_write(unsigned char reg, unsigned char value) {
	if (reg < RFM73_CMD_WRITE_REG) {
		reg |= RFM73_CMD_WRITE_REG;
	}

	RFM73_CSN(0);                // CSN low, init SPI transaction
	(void) rfm73_SPI_RW(reg);   // select register
	(void) rfm73_SPI_RW(value);
	RFM73_CSN(1);                // CSN high again
}

unsigned char rfm73_register_read(unsigned char reg) {
	unsigned char value;
	RFM73_CSN(0);               // CSN low, initialize SPI communication...
	(void) rfm73_SPI_RW(reg);   // Select register to read from..
	value = rfm73_SPI_RW(0);   // ..then read register value
	RFM73_CSN(1);               // CSN high, terminate SPI communication
	return value;                 // return register value
}

void rfm73_buffer_read(unsigned char reg, unsigned char pBuf[],
		unsigned char length) {
	unsigned char i;
	if (reg < RFM73_CMD_WRITE_REG) {
		reg |= RFM73_CMD_READ_REG;
	}
	RFM73_CSN(0);                     // Set CSN 0
	(void) rfm73_SPI_RW(reg);         // Select register to write
	for (i = 0; i < length; i++) {      // read all bytes
		pBuf[i] = rfm73_SPI_RW(0);  // read one byte from RFM73
	}
	RFM73_CSN(1);                     // Set CSN high again
}

void rfm73_buffer_write(char reg, const unsigned char pBuf[],
		unsigned char length) {
	unsigned char i;
	if (reg < RFM73_CMD_WRITE_REG) {
		reg |= RFM73_CMD_WRITE_REG;
	}
	RFM73_CSN(0);                      // Set CSN low, init SPI tranaction
	(void) rfm73_SPI_RW(reg);          // Select register to write tio write
	for (i = 0; i < length; i++) {       // write all bytes in buffer(*pBuf)
		(void) rfm73_SPI_RW(pBuf[i]);  // write one byte
	}
	RFM73_CSN(1);                      // Set CSN high again
}

void rfm73_bank(unsigned char b) {
	unsigned char st = 0x80 & rfm73_register_read(RFM73_REG_STATUS);
	if ((st && (b == 0)) || ((st == 0) && b)) {
		rfm73_register_write(RFM73_CMD_ACTIVATE, 0x53);
	}
}

// magic bank1 register initialization values
const unsigned long Bank1_Reg0_13[] = { 0xE2014B40, 0x00004BC0, 0x028CFCD0,

0x41390099, 0x1B8296D9, 0xA67F0624, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00127300, 0x36B48000 };

// more magic bank1 register initialization values
const unsigned char Bank1_Reg14[] = { 0x41, 0x20, 0x08, 0x04, 0x81, 0x20, 0xCF,
		0xF7, 0xFE, 0xFF, 0xFF };

// initialize bank1 like in the example code.
// don't ask why, just do it
void rfm73_init_bank1(void) {
	unsigned char i, j;
	unsigned char WriteArr[12];

	rfm73_bank(1);

	for (int a = 0; a < 20000; a++)
		;

	for (i = 0; i <= 8; i++) { //reverse!
		for (j = 0; j < 4; j++) {
			WriteArr[j] = (Bank1_Reg0_13[i] >> (8 * (j))) & 0xff;
		}
		rfm73_buffer_write(i, WriteArr, 4);
	}

	for (i = 9; i <= 13; i++) {
		for (j = 0; j < 4; j++) {
			WriteArr[j] = (Bank1_Reg0_13[i] >> (8 * (3 - j))) & 0xff;
		}
		rfm73_buffer_write(i, WriteArr, 4);
	}

	rfm73_buffer_write(14, Bank1_Reg14, 11);

	//toggle REG4<25,26>
	for (j = 0; j < 4; j++) {
		WriteArr[j] = (Bank1_Reg0_13[4] >> (8 * (j))) & 0xff;
	}

	WriteArr[0] = WriteArr[0] | 0x06;
	rfm73_buffer_write(4, WriteArr, 4);

	WriteArr[0] = WriteArr[0] & 0xf9;
	rfm73_buffer_write(4, WriteArr, 4);

	rfm73_bank(0);
}

unsigned char rfm73_is_present(void) {
	unsigned char st1, st2;
	st1 = rfm73_register_read(RFM73_REG_STATUS);
	rfm73_register_write(RFM73_CMD_ACTIVATE, 0x53);
	st2 = rfm73_register_read(RFM73_REG_STATUS);
	rfm73_register_write(RFM73_CMD_ACTIVATE, 0x53);
	return (st1 ^ st2) == 0x80;
}

void rfm73_mode_receive(void) {
	unsigned char value;

	// flush receive queue
	rfm73_register_write(RFM73_CMD_FLUSH_RX, 0);

	// clear interrupt status
	value = rfm73_register_read(RFM73_REG_STATUS);
	rfm73_register_write(RFM73_REG_STATUS, value);

	// switch to receive mode
	RFM73_CE(0);
	value = rfm73_register_read(RFM73_REG_CONFIG);
	value |= 0x01; // set RX bit
	value |= 0x02; // set PWR_UP bit
	rfm73_register_write(RFM73_REG_CONFIG, value);
	RFM73_CE(1);
}

void rfm73_mode_transmit(void) {
	unsigned char value;

	// flush transmit queue
	rfm73_register_write(RFM73_CMD_FLUSH_TX, 0);

	// clear interrupt status
	value = rfm73_register_read(RFM73_REG_STATUS);
	rfm73_register_write(RFM73_REG_STATUS, value);

	// switch to transmit mode
	RFM73_CE(0);
	value = rfm73_register_read(RFM73_REG_CONFIG);
	value &= 0xFE; // clear RX bit
	value |= 0x02; // set PWR_UP bit
	rfm73_register_write(RFM73_REG_CONFIG, value);
	RFM73_CE(1);
}

void rfm73_mode_standby(void) {
	unsigned char value;
	RFM73_CE(0);
	value = rfm73_register_read(RFM73_REG_CONFIG);
	value |= 0x02; // set PWR_UP bit
	rfm73_register_write(RFM73_REG_CONFIG, value);
}

void rfm73_mode_powerdown(void) {
	unsigned char value;
	RFM73_CE(0);
	value = rfm73_register_read(RFM73_REG_CONFIG);
	value &= 0xFD; // clear PWR_UP bit
	rfm73_register_write(RFM73_REG_CONFIG, value);
}

void rfm73_channel(unsigned char ch) {
	// MSB must be 0
	rfm73_register_write(RFM73_REG_RF_CH, ch & 0x7E);
}

void rfm73_air_data_rate(unsigned char rate) {
	unsigned char value;
	RFM73_CE(0);
	value = rfm73_register_read(RFM73_REG_RF_SETUP);
	value &= 0x07;
	if (rate == 0) {
		value |= 0x20;
	}
	if (rate > 1) {
		value |= 0x08;
	}
	rfm73_register_write(RFM73_REG_RF_SETUP, value);
	RFM73_CE(1);
}

void rfm73_crc_length(unsigned char len) {
	unsigned char val;
	if (len > 2) {
		len = 2;
	}
	val = rfm73_register_read(RFM73_REG_CONFIG);
	if (len == 0) {
		val &= 0xF3;
		rfm73_register_write(RFM73_REG_EN_AA, 0);
		rfm73_register_write(RFM73_REG_CONFIG, val);
	} else {
		rfm73_register_write(RFM73_REG_EN_AA, 0x3F);
		val &= 0xFB;
		if (val == 2) {
			val |= 0x04;
		}
		rfm73_register_write(RFM73_REG_CONFIG, val);
	}
}

void rfm73_address_length(unsigned char len) {
	if (len > 5) {
		len = 5;
	}
	if (len < 3) {
		len = 3;
	}
	rfm73_register_write(RFM73_REG_SETUP_AW, len - 2);
}

unsigned char rfm73_transmit_fifo_full(void) {
	unsigned char s;
	s = rfm73_register_read(RFM73_REG_FIFO_STATUS);
	return (s & FIFO_STATUS_TX_FULL) != 0;
}

unsigned char rfm73_receive_fifo_empty(void) {
	unsigned char s;
	s = rfm73_register_read(RFM73_REG_FIFO_STATUS);
	return (s & FIFO_STATUS_RX_EMPTY) != 0;
}

void rfm73_receive_address_p0(const unsigned char address[5]) {
	rfm73_buffer_write(RFM73_REG_RX_ADDR_P0, address, 5);
}

void rfm73_receive_address_p1(const unsigned char address[5]) {
	rfm73_buffer_write(RFM73_REG_RX_ADDR_P1, address, 5);
}

void rfm73_receive_address_pn(unsigned char channel, unsigned char address) {
	rfm73_register_write(RFM73_REG_RX_ADDR_P0 + channel, address);
}

void rfm73_transmit_address(const unsigned char address[]) {
	rfm73_buffer_write(RFM73_REG_TX_ADDR, address, 5);
}

unsigned char rfm73_retransmit_count(void) {
	return rfm73_register_read(RFM73_REG_OBSERVE_TX) & 0x0F;
}

unsigned char rfm73_lost_packets_count(void) {
	return (rfm73_register_read(RFM73_REG_OBSERVE_TX) >> 4) & 0x0F;
}

void rfm73_pipe_autoack(unsigned char pipe, unsigned char enabled) {
	unsigned char val = rfm73_register_read(RFM73_REG_EN_AA);
	if (pipe > 5) {
		pipe = 5;
	}
	if (enabled) {
		val |= 1 << pipe;
	} else {
		val &= ~(1 << pipe);
	}
	rfm73_register_write(RFM73_REG_EN_AA, val);
}

void rfm73_pipe_enable(unsigned char pipe, unsigned char enabled) {
	unsigned char val = rfm73_register_read(RFM73_REG_EN_RXADDR);
	if (pipe > 5) {
		pipe = 5;
	}
	if (enabled) {
		val |= 1 << pipe;
	} else {
		val &= ~(1 << pipe);
	}
	rfm73_register_write(RFM73_REG_EN_RXADDR, val);
}

void rfm73_lost_packets_reset(void) {
	unsigned char val = rfm73_register_read(RFM73_REG_RF_CH);
	rfm73_register_write(RFM73_REG_RF_CH, val);
}

void rfm73_retransmit_delay_attempts(unsigned char d, unsigned char n) {
	rfm73_register_write(RFM73_REG_SETUP_RETR, (n & 0x0F) | ((d & 0x0F) << 4));
}

void rfm73_lna_high(void) {
	unsigned char val = rfm73_register_read(RFM73_REG_RF_SETUP);
	val |= 0x01;
	rfm73_register_write(RFM73_REG_RF_SETUP, val);
}

void rfm73_lna_low(void) {
	unsigned char val = rfm73_register_read(RFM73_REG_RF_SETUP);
	val &= 0xFE;
	rfm73_register_write(RFM73_REG_RF_SETUP, val);
}

void rfm73_power(unsigned char level) {
	if (level > 3) {
		level = 3;
	}
	RFM73_CE(0);
	unsigned char val = rfm73_register_read(RFM73_REG_RF_SETUP);
	val &= 0x09;
	val |= 0x30;
	val |= (level << 1);
	rfm73_register_write(RFM73_REG_RF_SETUP, val);
	RFM73_CE(1);
}

void rfm73_channel_payload_size(unsigned char channel, unsigned char size) {
	unsigned char val;
	if (size > 32) {
		size = 32;
	}
	val = rfm73_register_read(RFM73_REG_DYNPD);
	if (size == 0) {
		val |= 1 << channel;
	} else {
		val &= ~(1 << channel);
	}
	rfm73_register_write(RFM73_REG_DYNPD, val);
	rfm73_register_write(RFM73_REG_RX_PW_P0 + channel, size);
}

void rfm73_transmit_message(const unsigned char buf[], unsigned char length) {
	if (length > 32) {
		length = 32;
	}
	rfm73_buffer_write(RFM73_CMD_W_TX_PAYLOAD, buf, length);
}

void rfm73_rssi_level(const unsigned char lvl) {
	rfm73_bank(1);

	unsigned char buff[] = { 0x24, 0x02, 0x7F, 0xA6 };
	buff[0] = (lvl << 2);
	rfm73_buffer_write(5, buff, 4);

	rfm73_bank(0);

}

void rfm73_transmit_message_once(const unsigned char buf[],
		unsigned char length) {
	if (length > 32) {
		length = 32;
	}
	rfm73_buffer_write(RFM73_CMD_W_TX_PAYLOAD_NOACK, buf, length);
}

unsigned char rfm73_receive_next_pipe(void) {
	unsigned char status = rfm73_register_read(RFM73_REG_STATUS);
	return (status >> 1) & 0x07;
}

unsigned char rfm73_receive_next_length(void) {
	return rfm73_register_read(RFM73_CMD_R_RX_PL_WID);
}

unsigned char rfm73_receive(unsigned char buf[], unsigned char *length) {
	if (rfm73_receive_fifo_empty())
		return 0;

	*length = rfm73_receive_next_length();
	rfm73_buffer_read(RFM73_CMD_R_RX_PAYLOAD, buf, *length);
	rfm73_register_write(RFM73_CMD_FLUSH_RX, 0);
	return 1;
}

void rfm73_init(void) {
	unsigned char i;

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;     //enalble rcc spi2
	GPIOB->MODER |=
	GPIO_MODER_MODER7_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER5_1
			| GPIO_MODER_MODER4_1 | GPIO_MODER_MODER3_1;

	SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR;
	SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH;
	SPI1->CR1 |= SPI_CR1_SPE;

	RFM73_CE(0);
	RFM73_CSN(1);

	for (int a = 0; a < 200000; a++)
		;

	// write array of default init settings
	rfm73_bank(0);

	for (i = 0; i < BANK0_ENTRIES; i++) {
		rfm73_register_write(Bank0_Reg[i][0], Bank0_Reg[i][1]);
	}

	rfm73_receive_address_p0(RX0_Address);
	rfm73_receive_address_p1(RX0_Address);
	rfm73_transmit_address(RX0_Address);

	// enable the extra features
	i = rfm73_register_read(29);
	if (i == 0) { // only when the extra features are not yet activated!
		rfm73_register_write(RFM73_CMD_ACTIVATE, 0x73);
	}

	// select dynamic payload length data pipe5\4\3\2\1\0
	rfm73_register_write(28, 0x3F);

	// select Dynamic Payload Length, Payload with ACK, W_TX_PAYLOAD_NOACK
	rfm73_register_write(29, 0x07);

	// dynamic payload sizing on channels 0 and 1
	rfm73_channel_payload_size(0, 0);
	rfm73_channel_payload_size(1, 0);

	rfm73_init_bank1();
	for (int a = 0; a < 200000; a++)
		;
}
