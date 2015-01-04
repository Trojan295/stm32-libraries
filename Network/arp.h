#ifndef NETWORK_ARP_H_
#define NETWORK_ARP_H_

#include <network/ethernet.h>
#include <network/ip.h>
#include <stdint.h>

#define ARP_TABLE_SIZE	10

typedef struct {
	uint16_t HTYPE;
	uint16_t PTYPE;
	uint8_t HLEN;
	uint8_t PLEN;
	uint16_t OPER;
	mac_addr SHA;
	ip_addr SPA;
	mac_addr THA;
	ip_addr TPA;
} ArpFrame;

typedef struct {
	mac_addr mac;
	ip_addr ip;
} ArpEntry;

#define ARP_OPER_ASK	1
#define ARP_OPER_ANSWER	2

uint8_t arp_ip_compare(ip_addr ip1, ip_addr ip2);

void arp_create_ask(ArpFrame *frame, mac_addr SHA, ip_addr SPA, ip_addr TPA);

void arp_create_answer(ArpFrame *frame, mac_addr SHA, ip_addr SPA, mac_addr THA,
		ip_addr TPA);

ArpEntry* arp_table_get_mac(ip_addr ip);
uint8_t arp_table_set_entry(mac_addr mac, ip_addr ip);

#endif /* NETWORK_ARP_H_ */
