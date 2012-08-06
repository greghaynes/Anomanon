#include "rrd_control.h"
#include "packet_handler.h"

#include <pcap.h>
#include <rrd.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#define RECORD_INTERVAL 1
#define HWPREDICT_ROWS 600

static int _keep_running;

void usage(int argc, char **argv) {
	printf("Usage: %s -o <db_path> -i <interface>\n -a <alpha> -s <season> [-b <beta>] [-q <filter>]\n", argv[0]);
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	int keep_running = 1;
	int packet_cnt = 0;

	// Valid packet
	if(packet)
		packet_handler_got_packet(header, packet);
}

int main(int argc, char **argv) {
	int interface_set = 0;
	char *interface;
	char *filter_exp = "";
	float alpha = 0, beta = 0;
	unsigned int season = 0;
	char *db_path = 0;

	int opterr = 0;
	char c;
	while((c = getopt (argc, argv, "i:q:a:b:s:o:")) != -1) {
		switch(c) {
			case 'i':
				interface_set = optarg ? 1 : 0;
				interface = optarg;
				break;
			case 'q':
				filter_exp = optarg;
				break;
			case 'a':
				sscanf(optarg, "%f", &alpha);
				break;
			case 'b':
				sscanf(optarg, "%f", &beta);
				break;
			case 's':
				sscanf(optarg, "%d", &season);
				break;
			case 'o':
				db_path = optarg;
		}
	}

	if(!interface_set || !db_path) {
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

	struct rrd_control_t *rrd_ctl = rrd_control_init(db_path, 1, HWPREDICT_ROWS, alpha, beta, season);
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
