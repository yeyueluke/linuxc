#ifndef POOL_H_
#define POOL_H_
#include <stdlib.h> 
#include <pthread.h>
#define step_num 3
typedef struct {
    void *(*callback)(void *);
    void *arg;
} event;

typedef struct {
	bool shutdown;
    pthread_mutex_t pool_mut;
    pthread_mutex_t active_thd_mut;

    pthread_cond_t empty_cond;
    pthread_cond_t full_cond;

    int max_events_num;
    int cur_events_num;

    int max_thd_num;
    int cur_thd_num;
    int active_thd_num;
	int min_thd_num;

	event* evts;
	int evts_front;
	int evts_back;
	int evts_size;

	
	pthread_t* tids;
	pthread_t managertid;

	int wait_die_num;

} threadpool;

#endif
