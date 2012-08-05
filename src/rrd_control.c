#include "rrd_control.h"
#include "packet_cntr.h"

#include <rrd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int rrd_control_do_update(struct rrd_control_t *ctl) {
	struct timeval cur_time;
	
	// Perform nanosleep
	struct timeval next_update;
	struct timeval sleep_len;
	struct timespec nano_sleep_len;
	next_update.tv_sec = ctl->next_update;
	next_update.tv_usec = 0;
	if(gettimeofday(&cur_time, 0) == -1) {
		perror("Getting time of day");
		return 1;
	}

	timersub(&next_update, &cur_time, &sleep_len);
	nano_sleep_len.tv_sec = sleep_len.tv_sec;
	nano_sleep_len.tv_nsec = sleep_len.tv_usec * 1000;
	nanosleep(&nano_sleep_len, 0);
	
	// Get current time
	if(gettimeofday(&cur_time, 0) == -1) {
		perror("Getting time of day");
		return 1;
	}

	// Set next update time
	ctl->next_update = cur_time.tv_sec + ctl->record_interval;

	// Do rrd_update
	char rrd_update_str[50];
	char *rrd_update_argv[] = {
		"update",
		0,
		rrd_update_str,
		0 };
	rrd_update_argv[1] = ctl->db_path;
	printf("Updating with %u packets\n", packet_cntr_get_cnt());
	sprintf(rrd_update_str, "%u:%u", cur_time.tv_sec,
	        packet_cntr_get_cnt());
	packet_cntr_reset();
	if(rrd_update(3, rrd_update_argv) == -1) {
		fprintf(stderr, "Couldn't update rrd database: %s\n:", rrd_get_error());
		return 1;
	}

	return 0;
}

void *rrd_control_main(void *arg) {
	while(!rrd_control_do_update(arg));
	printf("Done\n");
}

int rrd_control_start(struct rrd_control_t *ctl) {
	pthread_attr_t attr;
	int s = pthread_attr_init(&attr);
	if(s != 0) {
		errno = s;
		perror("Initializing rrd_control thread attrs:");
		return -1;	
	}

	// Set initial next_update
	struct timeval cur_time;
	if(gettimeofday(&cur_time, 0)) {
		perror("Getting time of day");
		return;
	}
	ctl->next_update = cur_time.tv_sec + 1;

	s = pthread_create(&ctl->thread_id, &attr, rrd_control_main, ctl);
	if(s != 0) {
		errno = s;
		perror("Initializing rrd_control thread attrs:");
		return -1;	
	}
	return 0;
}

struct rrd_control_t *rrd_control_init(const char *db_path,
                                       unsigned int record_interval) {
	char record_interval_str[25];
	sprintf(record_interval_str, "%u", record_interval);

	// Create rrd database
	const char *rrd_create_argv[] = {
		"rrdcreate",
		db_path,
		"--step", record_interval_str,
		"--start", "n", // Start now
		"DS:packets:COUNTER:1:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:HWPREDICT:100:0.5:0.5:5",
		0};
	if(rrd_create(9, (char**)rrd_create_argv)) {
		fprintf(stderr, "Couldn't create rrd database: %s\n:", rrd_get_error());
		return 0;
	}

	struct rrd_control_t *t = malloc(sizeof(struct rrd_control_t));
	t->record_interval = record_interval;
	// Copy db_path
	t->db_path = malloc(strlen(db_path) + 1);
	strcpy(t->db_path, db_path);

	return t;
}

void rrd_control_deinit(struct rrd_control_t *ctl) {
	free(ctl->db_path);
	free(ctl);
}

#ifdef RRD_CONTROL_TEST
int main() {
	struct rrd_control_t *t = rrd_control_init("/tmp/rrd_ctl_test.db", 1000);
	if(t) {
		rrd_control_start(t);
		rrd_control_deinit(t);
	}
	return 0;
}
#endif
