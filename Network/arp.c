#include <network/arp.h>
#include <string.h>

ArpEntry arp_table[ARP_TABLE_SIZE];
uint8_t ptr_entry;

uint8_t arp_ip_compare(uint8_t *ip1, uint8_t *ip2) {
	for (uint8_t i = 0; i < 4; i++) {
		if (*ip1++ != *ip2++)
			return 0;
	}
	return 1;
}

void arp_create_frame(ArpFrame *frame, uint16_t oper, mac_addr SHA, ip_addr SPA,
		mac_addr THA, ip_addr TPA) {
	memset(frame, 0, sizeof(ArpFrame));
	frame->HTYPE = ethernet_change_endianess(0x0001);
	frame->PTYPE = ethernet_change_endianess(0x0800);
	frame->HLEN = 6;
	frame->PLEN = 4;
	frame->OPER = ethernet_change_endianess(oper);

	memset(frame->SHA, 0xff, 20);

	if (SHA != 0)
		memcpy(frame->SHA, SHA, 6);
	if (SPA != 0)
		memcpy(frame->SPA, SPA, 4);
	if (THA != 0)
		memcpy(frame->THA, THA, 6);
	if (TPA != 0)
		memcpy(frame->TPA, TPA, 4);
}

void arp_create_ask(ArpFrame *frame, mac_addr SHA, ip_addr SPA, ip_addr TPA) {
	arp_create_frame(frame, ARP_OPER_ASK, SHA, SPA, 0, TPA);
}

void arp_create_answer(ArpFrame *frame, mac_addr SHA, ip_addr SPA, mac_addr THA,
		ip_addr TPA) {
	arp_create_frame(frame, ARP_OPER_ANSWER, SHA, SPA, THA, TPA);
}

ArpEntry* arp_table_get_mac(ip_addr ip) {
	for (uint8_t i = 0; i < ARP_TABLE_SIZE; i++) {
		if (ip_ip_compare(arp_table[i].ip, ip)) {
			return arp_table + i;
		}
	}
	return 0;
}

uint8_t arp_table_set_entry(mac_addr mac, ip_addr ip) {
	memcpy(arp_table[ptr_entry].mac, mac, 6);
	memcpy(arp_table[ptr_entry++].ip, ip, 4);
	return 1;
}
