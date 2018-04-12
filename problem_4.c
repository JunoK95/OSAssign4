#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>

/*
 * atomic_cmpxchg
 *
 * equivalent to atomic execution of this code:
 *
 * if (*ptr == old) {
 *   *ptr = new;
 *   return old;
 * } else {
 *   return *ptr;
 * }
 *
 */
 signed int threadCount;
 int boolswitch = 1;
 volatile int Number[99];
 volatile int entering[99];

 volatile int csCount[99];
 volatile int in_cs = 0;

 struct spin_lock_t {
   volatile int lockvar;
 };

struct spin_lock_t thread_lock;

static inline int atomic_cmpxchg (volatile int *ptr, int old, int new)
{
  int ret;
  asm volatile ("lock cmpxchgl %2,%1"
    : "=a" (ret), "+m" (*ptr)
    : "r" (new), "0" (old)
    : "memory");
  return ret;
}

void spin_lock (struct spin_lock_t *s)
{
    while(atomic_cmpxchg(&s->lockvar,0,1));
}

void spin_unlock (struct spin_lock_t *s)
{
    atomic_cmpxchg(&s->lockvar,1,0);
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
     csCount[thread]++;
     __sync_synchronize();
 }

 void *thread_body(void *arg) {
   while(boolswitch){
     long thread = (long)arg;
     spin_lock(&thread_lock);
     critical_section(thread);
     spin_unlock(&thread_lock);
   }
   return 0;
 }

int main(int argc, char **argv) {
  if (argc != 3){
    printf("%s\n", "Enter Number of Thread and Number of Seconds Only");
    exit(-1);
  }

  threadCount = atoi(argv[1]);
  int secondLimit = atoi(argv[2]);

  thread_lock.lockvar = 0;

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
