#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>

signed int threadCount;
volatile int tickets[99];
volatile int choosing[99];

volatile int csCount[99];
volatile int resource;
volatile int in_cs = 0;

void lock(int thread) {
    choosing[thread] = 1;
    __sync_synchronize();
    int max_ticket = 0;
    int i = 0;
    for (i = 0; i < threadCount; ++i) {
        int ticket = tickets[i];
        max_ticket = ticket > max_ticket ? ticket : max_ticket;
    }
    tickets[thread] = max_ticket + 1;
    __sync_synchronize();
    choosing[thread] = 0;
    __sync_synchronize();
    int other = 0;
    for (other = 0; other < threadCount; ++other) {
        while (choosing[other]) { }
        __sync_synchronize();
        while (tickets[other] != 0 &&
               (tickets[other] < tickets[thread] ||
                (tickets[other] == tickets[thread] && other < thread))) { }
    }
}

void unlock(int thread) {
    __sync_synchronize();
    tickets[thread] = 0;
}

void critical_section(int thread) {
    if (resource != 0) {
        printf("Resource was acquired by %d, but is still in-use by %d!\n",
               thread, resource);
    }
    resource = thread;
    //Checks to see only one is in criticalsection
    assert(in_cs==0);
    in_cs++;
    assert(in_cs==1);
    in_cs++;
    assert(in_cs==2);
    in_cs++;
    assert(in_cs==3);
    in_cs=0;
    printf("%d using resource...\n", thread);
    csCount[thread]++;
    printf("in cs %d times\n", csCount[thread]);
    __sync_synchronize();
    sleep(2);
    resource = 0;
}

void *thread_body(void *arg) {
    long thread = (long)arg;
    lock(thread);
    critical_section(thread);
    unlock(thread);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3){
      printf("%s\n", "Enter Number of Thread and Number of Seconds Only");
      exit(-1);
    }

    threadCount = atoi(argv[1]);
    long long int secondLimit = atoll(argv[2]);
    printf("%lli\n", secondLimit );

    memset((void*)tickets, 0, sizeof(tickets));
    memset((void*)choosing, 0, sizeof(choosing));
    memset((void*)csCount, 0, sizeof(choosing));
    resource = 0;

    pthread_t threads[threadCount];
    int i = 0;

    for (i = 0; i < threadCount; ++i) {
        pthread_create(&threads[i], NULL, &thread_body, (void*)((long)i));
    }

    for (i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
