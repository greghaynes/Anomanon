#include <pcap.h>
#include <rrd.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#define RECORD_INTERVAL 2
#define RRD_DB "/tmp/anamanon_rrd.db"

static int _timeout_secs;
static int _epoch_now;

void update_epoch_now(void) {
	struct timeval t;
	if(gettimeofday(&t, 0)) {
		perror("Getting time of day");
	} else {
		_epoch_now = t.tv_sec;
	}
}

void alarm_handler(int sig) {
	_timeout_secs++;
	_epoch_now++;
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

	// Setup second counter
	signal(SIGALRM, alarm_handler);
	alarm(1);

	// Update time
	update_epoch_now();
	char epoch_str[25];
	sprintf(epoch_str, "%d", _epoch_now);
	char record_interval_str[5];
	sprintf(record_interval_str, "%d", RECORD_INTERVAL);

	// Create rrd database
	char *rrd_create_argv[] = {
		"rrdcreate",
		RRD_DB,
		"--step", record_interval_str,
		"--start", epoch_str,
		"DS:speed:GAUGE:2:U:U",
		"RRA:LAST:0.5:1:100",
		0};
	if(rrd_create(8, rrd_create_argv)) {
		fprintf(stderr, "Couldn't create rrd database: %s\n:", rrd_get_error());
		return 1;
	}
	printf("Creating rrd database at %s, at %s\n", RRD_DB, epoch_str);

	// Main loop
	const u_char *packet;
	struct pcap_pkthdr header;
	int keep_running = 1;
	int packet_cnt = 0;
	char rrd_update_str[50];
	char *rrd_update_argv[] = {
		"update",
		RRD_DB,
		rrd_update_str,
		0 };
	while(keep_running) {
		// Grab packet
		packet = pcap_next(if_handle, &header);

		// Valid packet
		if(header.len > 0)
			packet_cnt++;

		// Record interval
		if(_timeout_secs >= RECORD_INTERVAL) {
			update_epoch_now();
			printf("Got %d packets\n", packet_cnt);
			sprintf(rrd_update_str, "%d:%d", _epoch_now, packet_cnt);
			printf("Updating with %s\n", rrd_update_str);
			if(rrd_update(3, rrd_update_argv)) {
				fprintf(stderr, "Couldn't update rrd database: %s\n:", rrd_get_error());
				return 1;
			}

			_timeout_secs = 0;
			packet_cnt = 0;
		}
	}

	pcap_close(if_handle);

	return 0;
}
