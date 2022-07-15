#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5
#define iteraciones 1000

//Solucion del problema de los filosofos con camarero, implementado con mutex y variables de condicion

int palilloCogido[N]; // a 1 si está cogido

pthread_mutex_t mutPalillos;
pthread_cond_t condPalillos;
pthread_mutex_t camarero;


void cogerPalillosMonitor(int i){
    pthread_mutex_lock(&camarero);
    pthread_mutex_lock(&mutPalillos);

    while(palilloCogido[i] || palilloCogido[(i+1)%N]){
        pthread_cond_wait(&condPalillos, &mutPalillos);
    }

    palilloCogido[i] = 1;
    palilloCogido[(i+1)%N] = 1;

    pthread_mutex_unlock(&mutPalillos);
    pthread_mutex_unlock(&camarero);
}

void dejarPalillosMonitor(int i){
    pthread_mutex_lock(&mutPalillos);

    palilloCogido[i] = 0;
    palilloCogido[(i+1)%N] = 0;
    pthread_cond_broadcast(&condPalillos);

    pthread_mutex_unlock(&mutPalillos);
}

void * filosofo(void * i){
    for(int j = 0; j< iteraciones; ++j){
        printf("Filosofo %d: Estoy pensando\n", *(int*) i);
        fflush(NULL);
        cogerPalillosMonitor(*(int*)i);
        printf("Filosofo %d: Estoy comiendo\n", * (int*) i);
        fflush(NULL);
        dejarPalillosMonitor(* (int*)i);
    }

    free(i);
    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutPalillos, NULL);
    pthread_cond_init(&condPalillos, NULL);
    pthread_mutex_init(&camarero, NULL);

    pthread_t filosofos[N];
    for(int i = 0; i< N; ++i){
        int * j = malloc(sizeof(int));
        *j = i;
        pthread_create(&filosofos[i],NULL,filosofo, j);
    }

    for(int i = 0; i< N; ++i){
        pthread_join(filosofos[i],NULL);
    }

    pthread_mutex_destroy(&mutPalillos);
    pthread_cond_destroy(&condPalillos);
    pthread_mutex_destroy(&camarero);
    return 0;
}