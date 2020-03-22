#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define sleep_time 1

static int             x, y;
static pthread_mutex_t mut      = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond_inc = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  cond_dec = PTHREAD_COND_INITIALIZER;

void *thread_inc(void *) {
    while (true) {
        pthread_mutex_lock(&mut);
        printf("inc get lock\n");
        while (x > 5) {
			printf("inc going to wait\n");
            pthread_cond_wait(&cond_inc, &mut);
			printf("inc wait done\n");
        }
        printf("++x=%d\n", ++x);
        if (x > 0) pthread_cond_signal(&cond_dec);
        pthread_mutex_unlock(&mut);
        printf("inc left\n");
        sleep(sleep_time);
    }
    return nullptr;
}

void *thread_dec(void *) {
    while (true) {
        pthread_mutex_lock(&mut);
        printf("dec get lock\n");
        while (x <= 0) {
			printf("dec going to wait\n");
            pthread_cond_wait(&cond_dec, &mut);
			printf("dec wait done\n");
        }
        printf("--x=%d\n", --x);
        if (x <= 5) pthread_cond_broadcast(&cond_inc);
        pthread_mutex_unlock(&mut);
        printf("dec left\n");
        sleep(sleep_time);
    }
    return nullptr;
}

int main() {
    int       i       = 0;
    int       inc_num = 1;
    int       dec_num = 2;
    pthread_t tid[100];
    x = y = 0;

    //pthread_mutex_init(&mut, nullptr);
    //pthread_cond_init(&cond_dec, nullptr);
    //pthread_cond_init(&cond_inc, nullptr);

    for (i = 0; i < inc_num; ++i) {
        pthread_create(&tid[i], nullptr, thread_inc, nullptr);
    }
    for (i = inc_num; i < inc_num + dec_num; ++i) {
        pthread_create(&tid[i], nullptr, thread_dec, nullptr);
    }

    for (i = 0; i < inc_num + dec_num; ++i) {
        pthread_join(tid[i], nullptr);
    }

    //pthread_mutex_destroy(&mut);
    //pthread_cond_destroy(&cond_dec);
    //pthread_cond_destroy(&cond_inc);

    return 0;
}
