#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define N 100

int numPares[N];
int numImpares[N];
int nPares = 0;
int nImpares = 0;

//Es el problema del productor consumidor con espacio no acotado

pthread_mutex_t mutexPares; //controla el acceso a nPares
pthread_mutex_t mutexImpares; //controla el acceso a nImpares
pthread_cond_t noPares; //variable a esperar cuando no quedan números pares
pthread_cond_t noImpares; //variable a esperar cuando no quedan números impares

void* thread1(){
    
    for(int i = 0; i< N; ++i){
        numPares[i]= 2*i+2;

        pthread_mutex_lock(&mutexPares);
        ++nPares;
        pthread_cond_signal(&noPares);
        pthread_mutex_unlock(&mutexPares);

    }

    pthread_exit(0);
}

void* thread2(){
    for(int i = 0; i< N; ++i){
        numImpares[i]= 2*i+1;

        pthread_mutex_lock(&mutexImpares);
        ++nImpares;
        pthread_cond_signal(&noImpares);
        pthread_mutex_unlock(&mutexImpares);

    }

    pthread_exit(0);
}

void* thread3(){
    for(int i = 0; i< N; ++i){
        pthread_mutex_lock(&mutexImpares);
        while(nImpares == 0){
            pthread_cond_wait(&noImpares, &mutexImpares);
        }

        --nImpares;
        pthread_mutex_unlock(&mutexImpares);

        printf("%d\n", numImpares[i]);

        pthread_mutex_lock(&mutexPares);
        while(nPares == 0){
            pthread_cond_wait(&noPares, &mutexPares);
        }

        --nPares;
        pthread_mutex_unlock(&mutexPares);

        printf("%d\n", numPares[i]);
    }

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutexPares, NULL);
    pthread_mutex_init(&mutexImpares, NULL);
    pthread_cond_init(&noPares, NULL);
    pthread_cond_init(&noImpares, NULL);

    pthread_t th1,th2,th3;
    pthread_create(&th1,NULL, thread1, NULL);
    pthread_create(&th2,NULL, thread2, NULL);
    pthread_create(&th3,NULL, thread3, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);

    pthread_mutex_destroy(&mutexPares);
    pthread_mutex_destroy(&mutexImpares);
    pthread_cond_destroy(&noPares);
    pthread_cond_destroy(&noImpares);

    return 0;
}