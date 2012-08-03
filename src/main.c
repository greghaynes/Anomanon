#include <pcap.h>

#include <unistd.h>
#include <stdio.h>

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
	if_handle = pcap_open_live(interface, BUFSIZ, 1, 1000, errbuf);
	if(!if_handle) {
		fprintf(stderr, "Couldn't open device %s: %s\n", interface, errbuf);
		return 1;
	}

	return 0;
}
