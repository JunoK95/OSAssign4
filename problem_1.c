#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>

signed int threadCount;
int boolswitch = 1;
volatile int Number[99];
volatile int entering[99];

volatile int csCount[99];
volatile int in_cs = 0;

void lock(int thread) {
    entering[thread] = 1;
    __sync_synchronize();
    int max_ticket = 0;
    int i = 0;
    for (i = 0; i < threadCount; ++i) {
        int ticket = Number[i];
        max_ticket = ticket > max_ticket ? ticket : max_ticket;
    }
    Number[thread] = max_ticket + 1;
    __sync_synchronize();
    entering[thread] = 0;
    __sync_synchronize();
    int j = 0;
    for (j = 0; j < threadCount; ++j) {
        while (entering[j]) { }
        __sync_synchronize();
        while (Number[j] != 0 &&
               (Number[j] < Number[thread] ||
                (Number[j] == Number[thread] && j < thread))) { }
    }
}

void unlock(int thread) {
    __sync_synchronize();
    Number[thread] = 0;
}

void critical_section(int thread) {
    //Checks to see only one is in criticalsection
    assert(in_cs==0);
    in_cs++;
    assert(in_cs==1);
    in_cs++;
    assert(in_cs==2);
    in_cs++;
    assert(in_cs==3);
    in_cs=0;
    // printf("%d using resource...\n", thread);
    csCount[thread]++;
    __sync_synchronize();
}

void *thread_body(void *arg) {
  // int msec = 0;
  // int trigger = secs * 1000;
  // clock_t before = clock();
  while(boolswitch){
    long thread = (long)arg;
    lock(thread);
    critical_section(thread);
    unlock(thread);
    // clock_t difference = clock() - before;
    // msec = difference * 1000 / CLOCKS_PER_SEC;
  } //while (msec < trigger);
  return 0;
}

int main(int argc, char **argv) {
    if (argc != 3){
      printf("%s\n", "Enter Number of Thread and Number of Seconds Only");
      exit(-1);
    }
    threadCount = atoi(argv[1]);
    int secondLimit = atoi(argv[2]);

    pthread_t threads[threadCount];
    int i = 0;

    for (i = 0; i < threadCount; ++i) {
        pthread_create(&threads[i], NULL, &thread_body, (void*)((long)i));
    }

    sleep(secondLimit);
    boolswitch = 0;

    for (i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
        printf("%d in cs %d times\n", i,csCount[i]);
    }

    return 0;
}
