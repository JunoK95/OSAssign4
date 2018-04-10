#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int _totalpointCount = 0;
int _circlepointCount = 0;
pthread_mutex_t lock;

//Random from 0 to 1
double rng(){
  return (double)rand() / (double)RAND_MAX;
}

void *monte_carlo(void* arg){
  pthread_mutex_lock(&lock);
  long long int *secs = (long long*) arg;
  int msec = 0;
  int trigger = *secs;
  clock_t before = clock();

  do {
    _totalpointCount++;
    double x = rng();
    double y = rng();

    // printf("random x %f\n", x - 0.5);
    // printf("random y %f\n", y - 0.5);
    //printf("total point counter %d\n", _totalpointCount);
    double dist = ((x - 0.5) * (x - 0.5)) + ((y - 0.5) * (y - 0.5));
    // printf("distance %f\n", dist );
    if ((dist) < 0.25){
      _circlepointCount++;
    }
    // printf("total points %d\n", _totalpointCount);
    // printf("circle points %d\n", _circlepointCount);
    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
  } while (msec < trigger);

  pthread_mutex_unlock(&lock);
  return NULL;
}

int main(int argc, char **argv){
  int error;

  if (argc != 3){
    printf("%s\n", "Enter Number of Thread and Number of Seconds Only");
    exit(-1);
  }

  if (pthread_mutex_init(&lock, NULL) != 0){
    // printf("%s\n", "mutex init failed");
    return 1;
  }

  unsigned int threadCount = atoi(argv[1]);
  long long secondLimit = atoll(argv[2]);
  //Thread ID:
  pthread_t tid[threadCount];
  unsigned int i;

  for (i = 0; i < threadCount; i++) {
      error = pthread_create(&(tid[i]), NULL, &monte_carlo, &secondLimit);
      if (error != 0){
        // printf("%s\n", "tread can't be created");
      }
  }

  for (i = 0; i < threadCount; i++) {
      pthread_join(tid[i],NULL);
  }
  double division = 4 * ((double)_circlepointCount/(double)_totalpointCount);
  printf("%f\n", division );
  // printf("%s\n", "done");
  pthread_mutex_destroy(&lock);
  return 0;
}
