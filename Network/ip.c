#include <network/ethernet.h>
#include <network/ip.h>
#include <string.h>

uint16_t ip_checksum(IpHeader *header, uint16_t length) {
	uint8_t* data = (uint8_t*) header;

	uint32_t acc = 0xffff;

	for (uint16_t i = 0; i + 1 < length; i += 2) {
		uint16_t word;
		memcpy(&word, data + i, 2);
		acc += word;
		if (acc > 0xffff) {
			acc -= 0xffff;
		}
	}

	if (length & 1) {
		uint16_t word = 0;
		memcpy(&word, data + length - 1, 1);
		acc += word;
		if (acc > 0xffff) {
			acc -= 0xffff;
		}
	}
	return ~acc;
}

uint8_t ip_ip_compare(ip_addr ip1, ip_addr ip2) {
	for (uint8_t i = 0; i < 4; i++) {
		if (*ip1++ != *ip2++)
			return 0;
	}
	return 1;
}

void ip_init_header(IpHeader *header, ip_addr source_ip, ip_addr dest_ip,
		uint8_t protocol) {
	memset(header, 0, sizeof(IpHeader));
	header->ver_length = (4 << 4) | 5;
	header->flags_offset = ethernet_change_endianess((2 << 13));
	header->time_to_live = 64;
	header->protocol = protocol;
	memcpy(header->source_ip, source_ip, 4);
	memcpy(header->dest_ip, dest_ip, 4);

	//header->checksum = ip_checksum(header, sizeof(IpHeader));
}

void ip_set_data_length(IpHeader *header, uint16_t length) {
	header->total_length = ethernet_change_endianess(length);
	header->checksum = ip_checksum(header, 20);
}
void ip_create_udp_header(IpHeader *header, ip_addr source_ip, ip_addr dest_ip) {
	ip_init_header(header, source_ip, dest_ip, IP_PROTOCOL_UDP);
}
