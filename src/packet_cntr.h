#ifndef PACKET_CNTR_H
#define PACKET_CNTR_H

#include <pcap.h>

void packet_cntr_got_packet(const struct pcap_pkthdr *header,
                            const u_char *packet);

unsigned int packet_cntr_get_cnt(void);
void packet_cntr_reset(void);

#endif

