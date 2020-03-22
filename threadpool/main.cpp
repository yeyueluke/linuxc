#include "threadpool.h"
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
void *process(void *arg) {
    int *a = (int *)arg;
    printf("tid: %lu is processing event%d\n", pthread_self(), *a);
    sleep(5);
    return nullptr;
}
bool is_tid_alive(pthread_t tid) {
    int ret = pthread_kill(tid, 0);
    if ((ret == ESRCH) || (ret == EINVAL)) { return false; }
    return true;
}
void *thread_worker(void *arg) {
    threadpool *pool = (threadpool *)arg;
    event       evt;
    while (true) {
        printf("worker fight for lock\n");
        pthread_mutex_lock(&pool->pool_mut);
        printf("worker got lock\n");
        while ((pool->cur_events_num <= 0) && (!pool->shutdown)) {
            printf("begin wait\n");
            pthread_cond_wait(&pool->empty_cond, &pool->pool_mut);
            printf("end wait\n");
        }

        if (pool->wait_die_num > 0) {
            pool->wait_die_num--;
            if (pool->cur_thd_num > pool->min_thd_num) {
                pool->cur_thd_num--;
                pthread_mutex_unlock(&pool->pool_mut);
                printf("tid: %lu exiting...\n", pthread_self());
                pthread_exit(nullptr);
            }
        }
        evt.callback = pool->evts[pool->evts_front].callback;
        evt.arg      = pool->evts[pool->evts_front].arg;
        pool->cur_events_num--;
        pool->evts_front = (pool->evts_front + 1) % (pool->evts_size);
        pthread_mutex_unlock(&pool->pool_mut);

        printf("worker fight for active_mut\n");
        pthread_mutex_lock(&pool->active_thd_mut);
        pool->active_thd_num++;
        pthread_mutex_unlock(&pool->active_thd_mut);

        printf("going to process...\n");
        evt.callback(evt.arg);

        pthread_mutex_lock(&pool->active_thd_mut);
        pool->active_thd_num--;
        pthread_mutex_unlock(&pool->active_thd_mut);

        sleep(1);
    }
    return nullptr;
}
void *thread_manager(void *arg) {
    threadpool *pool           = (threadpool *)arg;
    int         cur_evts_num   = 0;
    int         active_thd_num = 0;
    int         cur_thd_num    = 0;
    int         i              = 0;
    int         add            = 0;
    while (true) {
        sleep(10);
        printf("*************manager fight for lock\n");
        pthread_mutex_lock(&pool->pool_mut);
        printf("*************manager got lock\n");
        cur_evts_num = pool->cur_events_num;
        cur_thd_num  = pool->cur_thd_num;
        pthread_mutex_unlock(&pool->pool_mut);

        printf("*************manager fight for lock thd_mut\n");
        pthread_mutex_lock(&pool->active_thd_mut);
        printf("*************manger got lock\n");
        active_thd_num = pool->active_thd_num;
        pthread_mutex_unlock(&pool->active_thd_mut);

        // need dec
        if ((double)active_thd_num <= cur_thd_num * 0.2) {
            printf("going to delete worker\n");
			pool->wait_die_num = (int)(0.2*cur_thd_num);
            continue;
        }

        if ((double)active_thd_num <= cur_thd_num * 0.8) continue;
        // need inc
        printf("going to add worker...\n");
        pthread_mutex_lock(&pool->pool_mut);
        for (i = 0; (i < pool->max_thd_num) && (add < step_num); ++i) {
            if (pool->tids[i] != 0 && is_tid_alive(pool->tids[i])) continue;
            pthread_create(&pool->tids[i], nullptr, thread_worker, pool);
            pool->cur_thd_num++;
            add++;
        }
        printf("add worker done, cur_num: %d\n", pool->cur_thd_num);
        pthread_mutex_unlock(&pool->pool_mut);
    }
    return nullptr;
}
threadpool *threadpool_create(int maxevents, int maxthds, int minthds) {
    int         i    = 0;
    threadpool *pool = nullptr;
mallocpool:
    pool = (threadpool *)malloc(sizeof(threadpool));
    if (pool == nullptr) goto mallocpool;

    pool->pool_mut       = PTHREAD_MUTEX_INITIALIZER;
    pool->active_thd_mut = PTHREAD_MUTEX_INITIALIZER;
    pool->empty_cond     = PTHREAD_COND_INITIALIZER;
    pool->full_cond      = PTHREAD_COND_INITIALIZER;

    pool->max_events_num = maxevents;
    pool->max_thd_num    = maxthds;
    pool->min_thd_num    = minthds;
    pool->evts_size      = maxevents;
    pool->active_thd_num = 0;
    pool->cur_thd_num    = 0;
    pool->cur_events_num = 0;
    pool->evts_back      = 0;
    pool->evts_front     = 0;
    pool->wait_die_num   = 0;
    pool->shutdown       = false;

mallocevts:
    pool->evts = nullptr;
    pool->evts = (event *)malloc(sizeof(event) * (size_t)pool->max_events_num);
    if (pool->evts == nullptr) goto mallocevts;

mallocthd:
    pool->tids = nullptr;
    pool->tids =
        (pthread_t *)malloc(sizeof(pthread_t) * (size_t)pool->max_thd_num);
    if (pool->tids == nullptr) goto mallocthd;

    // create thread
    for (i = 0; i < pool->min_thd_num; ++i) {
        pthread_create(&pool->tids[i], nullptr, thread_worker, pool);
    }
    pthread_create(&pool->managertid, nullptr, thread_manager, pool);
    pthread_mutex_lock(&pool->pool_mut);
    pool->cur_thd_num += pool->min_thd_num;
    pthread_mutex_unlock(&pool->pool_mut);

    printf("pool create done\n");
    return pool;
}
void event_add(threadpool *pool, void *(*callback)(void *), void *arg) {
    pthread_mutex_lock(&pool->pool_mut);
    while (pool->cur_events_num >= pool->evts_size) {
        pthread_cond_wait(&pool->full_cond, &pool->pool_mut);
    }
    pool->evts[pool->evts_back].callback = callback;
    pool->evts[pool->evts_back].arg      = arg;
    pool->evts_back = (pool->evts_back + 1) % pool->evts_size;
    pool->cur_events_num++;
    pthread_cond_signal(&pool->empty_cond);
    printf("event add done\n");
    pthread_mutex_unlock(&pool->pool_mut);
}
int main() {
    int         num[20];
    int         i    = 0;
    threadpool *pool = nullptr;
    pool             = threadpool_create(100, 100, 3);
    sleep(3);
    for (i = 0; i < 20; ++i) {
        num[i] = i;
        event_add(pool, process, &num[i]);
    }
    sleep(20);
    event_add(pool, process, &i);
    sleep(100);
    return 0;
}
