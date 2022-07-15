#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define N 100

int numPares[N];
int numImpares[N];

sem_t permisoPares;
sem_t permisoImpares;

void* thread1(){
    for(int i = 0; i< N; ++i){
        numPares[i]= 2*i+2;
        sem_post(&permisoPares);
    }

    pthread_exit(0);
}

void* thread2(){
    for(int i = 0; i< N; ++i){
        numImpares[i]= 2*i+1;
        sem_post(&permisoImpares);
    }

    pthread_exit(0);
}

void* thread3(){
    for(int i = 0; i< N; ++i){
        sem_wait(&permisoImpares);
        printf("%d\n", numImpares[i]);

        sem_wait(&permisoPares);
        printf("%d\n", numPares[i]);
    }

    pthread_exit(0);
}

int main(){
    sem_init(&permisoPares,0, 0);
    sem_init(&permisoImpares,0,0);

    pthread_t th1,th2,th3;
    pthread_create(&th1,NULL, thread1, NULL);
    pthread_create(&th2,NULL, thread2, NULL);
    pthread_create(&th3,NULL, thread3, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);

    sem_destroy(&permisoPares);
    sem_destroy(&permisoImpares);

    return 0;
}