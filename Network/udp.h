#ifndef NETWORK_UDP_H_
#define NETWORK_UDP_H_

#include <stdint.h>

typedef struct {
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum;
} UdpHeader;

void udp_create_header(UdpHeader *header, uint16_t source_port,
		uint16_t dest_port, uint16_t length);

#endif /* NETWORK_UDP_H_ */
