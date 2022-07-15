#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

long long int total_sum = 0;

sem_t espera;

void * partial_sum(void * arg) {
  int j = 0;
  int ni=((int*)arg)[0];
  int nf=((int*)arg)[1];

  for (j = ni; j <= nf; j++) {
    sem_wait(&espera); // acceso a la variable total_sum con exclusión mutua
    total_sum = total_sum + j;
    sem_post(&espera);
  }

  pthread_exit(0);
}

int main(int argc, char* argv[]) {

  if (argc != 3){
    fprintf(stderr, "Usage: ./partial_sum numHilos limiteSuma\n");
    exit(-1);
  }

  int numHilos = atoi(argv[1]);
  long long int limiteSuma = atoi(argv[2]);

  sem_init(&espera, 0, 1);

  pthread_t ths [numHilos];
  int sumaCada = limiteSuma/ numHilos;
  int num[numHilos][2];

  /* numHilos threads  are created */
  for(int i = 0; i< numHilos -1; ++i){
      num[i][0] = i*sumaCada+1;
      num[i][1] = (i+1)*sumaCada;
      pthread_create(&ths[i], NULL, partial_sum, (void*)num[i]);
  }

  num[numHilos-1][0] = (numHilos-1)*sumaCada+1;
  num[numHilos-1][1] = limiteSuma;
  pthread_create(&ths[numHilos-1], NULL, partial_sum, (void*)num[numHilos-1]);

  /* the main thread waits until both threads complete */
  for(int i = 0; i < numHilos; ++i){
    pthread_join(ths[i], NULL);
  }
 
  printf("total_sum=%lld and it should be %lld\n", total_sum, limiteSuma*(limiteSuma+1)/2);

  sem_destroy(&espera);

  return 0;
}
