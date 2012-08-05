#include "rrd_control.h"

#include <rrd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

void *rrd_control_main(void *arg) {
	return 0;
}

int rrd_control_start(struct rrd_control_t *ctl) {
	pthread_attr_t attr;
	int s = pthread_attr_init(&attr);
	if(s != 0) {
		errno = s;
		perror("Initializing rrd_control thread attrs:");
		return -1;	
	}

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
