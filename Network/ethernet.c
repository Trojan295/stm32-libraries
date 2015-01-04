#include <network/ethernet.h>
#include <string.h>

uint16_t ethernet_change_endianess(uint16_t data) {
	return (data << 8) | (data >> 8);
}

uint8_t ethernet_mac_compare(mac_addr mac1, mac_addr mac2) {
	for (uint8_t i = 0; i < 6; i++) {
		if (*mac1++ != *mac2++)
			return 0;
	}
	return 1;
}

void ethernet_create_header(EthernetHeader *header, mac_addr source_mac,
		mac_addr destination_mac, uint16_t protocol) {
	memset(header, 0, sizeof(EthernetHeader));
	memcpy(header->dest_mac, destination_mac, 6);
	memcpy(header->source_mac, source_mac, 6);
	header->protocol = ethernet_change_endianess(protocol);
}
