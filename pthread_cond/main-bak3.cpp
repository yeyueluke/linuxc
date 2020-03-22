#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

static int             mycount = 0;
static pthread_mutex_t mut     = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond    = PTHREAD_COND_INITIALIZER;
//该函数增加mycount数值
void *creator(void *) {
    while (true) {
        pthread_mutex_lock(&mut);
        cout << "creator add lock" << endl;
        mycount++;
        cout << "in creator mycount is : " << mycount << endl;
        //条件满足时发送信号
        if (mycount > 0) { pthread_cond_signal(&cond); }
        cout << "creator release lock" << endl;
        pthread_mutex_unlock(&mut);
		sleep(3);
    }

    return nullptr;
}

//该函数减少mycount数值
void *consumer(void *) {
    while (true) {
        pthread_mutex_lock(&mut);
        cout << "consumer add lock" << endl;
        //当条件不满足时等待
        while (mycount <= 0) //防止虚假唤醒
        {
            cout << "begin wait" << endl;
            pthread_cond_wait(&cond, &mut);
            cout << "end wait" << endl;
        }
        mycount--;
        cout << "in consumer mycount is " << mycount << endl;
        pthread_mutex_unlock(&mut);
        cout << "consumer release lock" << endl;
    }
    return nullptr;
}

int main() {
    //两个线程，一个生产者线程一个消费者线程
    pthread_t createthread, consumethread1, consumethread2;

    pthread_create(&consumethread1, nullptr, consumer, nullptr);
    pthread_create(&consumethread2, nullptr, consumer, nullptr);
    sleep(2);
    pthread_create(&createthread, nullptr, creator, nullptr);
    //主进程等待两个线程结束
    pthread_join(createthread, nullptr);
    pthread_join(consumethread1, nullptr);
    pthread_join(consumethread2, nullptr);
    return 0;
}
