#ifndef NETWORK_ETHERNET_H_
#define NETWORK_ETHERNET_H_

#include <stdint.h>

typedef uint8_t mac_addr[6];

typedef struct {
	mac_addr dest_mac;
	mac_addr source_mac;
	uint16_t protocol;
} EthernetHeader;

#define ETHERNET_PROTOCOL_IP	0x0800
#define ETHERNET_PROTOCOL_ARP	0x0806

uint16_t ethernet_change_endianess(uint16_t data);
uint8_t ethernet_mac_compare(mac_addr mac1, mac_addr mac2);
void ethernet_create_header(EthernetHeader *header, mac_addr source_mac,
		mac_addr destination_mac, uint16_t protocol);

#endif /* NETWORK_ETHERNET_H_ */
