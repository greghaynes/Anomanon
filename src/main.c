#include <pcap.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#define RECORD_INTERVAL 2

static int _timeout_secs;

void alarm_handler(int sig) {
	_timeout_secs ++;
	alarm(1);
}

void usage(int argc, char **argv) {
	printf("Usage: %s -i <interface>\n", argv[0]);
}

int main(int argc, char **argv) {
	int interface_set = 0;
	char *interface;

	int opterr = 0;
	char c;
	while((c = getopt (argc, argv, "i:")) != -1) {
		switch(c) {
			case 'i':
				interface_set = optarg ? 1 : 0;
				interface = optarg;
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
	char filter_exp[] = "port 22";
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

	signal(SIGALRM, alarm_handler);
	alarm(1);

	const u_char *packet;
	struct pcap_pkthdr header;
	int keep_running = 1;
	int packet_cnt = 0;
	while(keep_running) {
		packet = pcap_next(if_handle, &header);

		if(header.len > 0)
			packet_cnt++;

		if(_timeout_secs >= RECORD_INTERVAL) {
			printf("Got %d packets\n", packet_cnt);
			_timeout_secs = 0;
			packet_cnt = 0;
		}
	}

	pcap_close(if_handle);

	return 0;
}
