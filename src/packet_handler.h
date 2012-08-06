#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <pcap.h>

void packet_handler_got_packet(const struct pcap_pkthdr *header,
                            const u_char *packet);

unsigned int packet_handler_get_cnt(void);
unsigned int packet_handler_get_size(void);
void packet_handler_reset(void);

#endif

