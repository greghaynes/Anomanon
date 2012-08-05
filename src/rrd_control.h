#ifndef RRD_CONTROL_H
#define RRD_CONTROL_H

#include <pthread.h>

#include <sys/time.h>

struct rrd_control_t {
	unsigned int record_interval;
	char *db_path;
	pthread_t thread_id;
	time_t next_update;
};

struct rrd_control_t *rrd_control_init(const char *db_path,
                                       unsigned int record_interval,
                                       unsigned int predict_size,
                                       float alpha, float beta,
                                       unsigned int season_records);

int rrd_control_start(struct rrd_control_t*);

void rrd_control_deinit(struct rrd_control_t *ctl);

#endif

