#include <network/ethernet.h>
#include <network/udp.h>
#include <string.h>

void udp_create_header(UdpHeader *header, uint16_t source_port,
		uint16_t dest_port, uint16_t length) {
	memset(header, 0, sizeof(UdpHeader));
	header->source_port = ethernet_change_endianess(source_port);
	header->dest_port = ethernet_change_endianess(dest_port);
	header->length = ethernet_change_endianess(length+sizeof(UdpHeader));
}
