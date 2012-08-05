#include "rrd_control.h"
#include "packet_cntr.h"

#include <pcap.h>
#include <rrd.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#define RECORD_INTERVAL 1
#define RRD_DB "/tmp/anamanon_rrd.db"

static int _keep_running;

void usage(int argc, char **argv) {
	printf("Usage: %s -i <interface>\n", argv[0]);
}

static char rrd_update_str[50];
static char *rrd_update_argv[] = {
	"update",
	RRD_DB,
	rrd_update_str,
	0 };

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	int keep_running = 1;
	int packet_cnt = 0;

	// Valid packet
	if(packet)
		packet_cntr_got_packet(header, packet);
}

int main(int argc, char **argv) {
	int interface_set = 0;
	char *interface;
	char *filter_exp = "";

	int opterr = 0;
	char c;
	while((c = getopt (argc, argv, "i:q:")) != -1) {
		switch(c) {
			case 'i':
				interface_set = optarg ? 1 : 0;
				interface = optarg;
				break;
			case 'q':
				filter_exp = optarg;
				break;
		}
	}

	if(!interface_set) {
		usage(argc, argv);
		return 1;
	}

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *if_handle;
	struct bpf_program bpf_prog;
	bpf_u_int32 mask;
	bpf_u_int32 net;

	if(pcap_lookupnet(interface, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Can't get netmask for device %s\n", interface);
		net = 0;
		mask = 0;
	}

	if_handle = pcap_open_live(interface, BUFSIZ, 1, 100, errbuf);
	if(!if_handle) {
		fprintf(stderr, "Couldn't open device %s: %s\n", interface, errbuf);
		return 1;
	}

	if(pcap_compile(if_handle, &bpf_prog, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(if_handle));
		return 1;
	}

	if(pcap_setfilter(if_handle, &bpf_prog) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(if_handle));
		return 1;
	}

	struct rrd_control_t *rrd_ctl = rrd_control_init(RRD_DB, 1);
	rrd_control_start(rrd_ctl);

	// Main loop
	_keep_running = 1;
	while(_keep_running) {
		if(pcap_loop(if_handle, 0, got_packet, 0) == -1) {
			fprintf(stderr, "Error calling pcap_loop: %s\n", pcap_geterr(if_handle));
			_keep_running = 0;
		}
	}

	pcap_close(if_handle);

	return 0;
}
