#include "packet_handler.h"

static unsigned int _packet_count;
static unsigned int _packet_size;

void packet_handler_got_packet(const struct pcap_pkthdr *header,
                            const u_char *packet) {
	_packet_count++;
	_packet_size += header->len;
}

unsigned int packet_handler_get_cnt(void) {
	return _packet_count;
}

unsigned int packet_handler_get_size(void) {
	return _packet_size;
}

void packet_handler_reset(void) {
	_packet_count = 0;
	_packet_size = 0;
}

