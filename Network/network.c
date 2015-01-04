#include <network/network.h>
#include <stdint.h>
#include <string.h>

extern mac_addr this_mac;
extern ip_addr this_ip;

uint16_t network_strcpy(uint8_t *dest, uint8_t *source) {
	uint16_t length = 0;
	while (*source) {
		*dest++ = *source++;
		length++;
	}
	return length;
}

void network_init(mac_addr addr) {
	enc_init(addr);
}

void network_send_arp_ask(mac_addr source_mac, ip_addr source_ip,
		ip_addr dest_ip) {

	uint8_t frame[1518];
	uint16_t length = 0;
	mac_addr dest_mac = { 255, 255, 255, 255, 255, 255 };

	EthernetHeader *ethernet_header = (EthernetHeader*) frame;
	length += sizeof(EthernetHeader);
	ethernet_create_header(ethernet_header, source_mac, dest_mac,
	ETHERNET_PROTOCOL_ARP);

	ArpFrame *arp_frame = (ArpFrame*) (frame + sizeof(EthernetHeader));
	length += sizeof(ArpFrame);

	arp_create_ask(arp_frame, source_mac, source_ip, dest_ip);

	enc_send(frame, length);
}

void network_send_arp_answer(mac_addr dest_mac, ip_addr dest_ip) {

	uint8_t frame[1518];
	uint16_t length = 0;

	EthernetHeader *ethernet_header = (EthernetHeader*) frame;
	length += sizeof(EthernetHeader);
	ethernet_create_header(ethernet_header, this_mac, dest_mac,
	ETHERNET_PROTOCOL_ARP);

	ArpFrame *arp_frame = (ArpFrame*) (frame + sizeof(EthernetHeader));
	length += sizeof(ArpFrame);

	arp_create_answer(arp_frame, this_mac, this_ip, dest_mac, dest_ip);

	enc_send(frame, length);
}

uint8_t network_send_data_packet(ip_addr dest_ip, uint16_t port, uint8_t *msg) {
	uint8_t frame[1518];
	uint16_t length = 0;

	ArpEntry *entry = arp_table_get_mac(dest_ip);
	if (!entry) {
		network_send_arp_ask(this_mac, this_ip, dest_ip);
		return 0;
	}

	mac_addr dest_mac;
	memcpy(dest_mac, arp_table_get_mac(dest_ip)->mac, 6);

	EthernetHeader *ethernet_header = (EthernetHeader*) frame;
	IpHeader *ip_header = (IpHeader*) (frame + sizeof(EthernetHeader));
	UdpHeader *udp_header = (UdpHeader*) ((uint8_t*) ip_header
			+ sizeof(IpHeader));
	uint8_t *data = (uint8_t*) ((uint8_t*) udp_header + sizeof(UdpHeader));

	length += network_strcpy(data, msg);

	udp_create_header(udp_header, port, 20014, length);
	;

	length += sizeof(UdpHeader);
	length += sizeof(IpHeader);

	ip_create_udp_header(ip_header, this_ip, dest_ip);
	ip_set_data_length(ip_header, length);

	length += sizeof(EthernetHeader);

	ethernet_create_header(ethernet_header, this_mac, dest_mac,
	ETHERNET_PROTOCOL_IP);

	enc_send(frame, length);
	return 1;
}
