#include "packet_cntr.h"

static unsigned int _packet_count;

void packet_cntr_got_packet(const struct pcap_pkthdr *header,
                            const u_char *packet) {
	_packet_count++;
}

unsigned int packet_cntr_get_cnt(void) {
	return _packet_count;
}

void packet_cntr_reset(void) {
	_packet_count = 0;
}

