#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdint.h>
#include <network/arp.h>
#include <network/enc28j60.h>
#include <network/ethernet.h>
#include <network/ip.h>
#include <network/udp.h>

void network_init(mac_addr addr);
void network_send_arp_answer(mac_addr dest_mac, ip_addr dest_ip);
uint8_t network_send_data_packet(ip_addr dest_ip, uint16_t port, uint8_t *msg);

#endif /* NETWORK_H_ */
