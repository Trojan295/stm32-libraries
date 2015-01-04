#ifndef NETWORK_IP_H_
#define NETWORK_IP_H_

#include <stdint.h>

typedef uint8_t ip_addr[4];

typedef struct {
	uint8_t ver_length;
	uint8_t type_of_service;
	uint16_t total_length;
	uint16_t ident;
	uint16_t flags_offset;
	uint8_t time_to_live;
	uint8_t protocol;
	uint16_t checksum;
	ip_addr source_ip;
	ip_addr dest_ip;
} IpHeader;

#define IP_PROTOCOL_UDP	17

uint8_t ip_ip_compare(ip_addr ip1, ip_addr ip2);
void ip_set_data_length(IpHeader *header, uint16_t length);
void ip_create_udp_header(IpHeader *header, ip_addr source_ip, ip_addr dest_ip);

#endif /* NETWORK_IP_H_ */
