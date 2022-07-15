#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define NPASAJEROS 10
#define CAPACIDAD 5

//Estados
#define SUBIR 0
#define BAJAR 1
#define NO 2 //ni subir ni bajar


int estado = NO;
int n_ocupantes = 0;
int n_pasajerosAtendidos = 0;

sem_t mutex; //protege el acceso a estado y n_ocupantes
sem_t mutexPAtend; //protege el acceso a n_pasajerosAtendidos

sem_t colaSubir; //esperan los pasajeros para subir
sem_t colaBajar; //esperan los pasajeros para bajar
sem_t esperaCoche; //espera el coche a que suban/bajen los pasajeros

void load(){
    sem_wait(&mutex);
    estado = SUBIR;
    printf("Ya pueden subir los siguientes pasajeros\n");
    
    for(int i = 0; i < CAPACIDAD; ++i){
        sem_post(&colaSubir);
    }

    if(n_ocupantes < CAPACIDAD){
        sem_post(&mutex);
        sem_wait(&esperaCoche);
        sem_wait(&mutex);
    }

    estado = NO;
    sem_post(&mutex);
}

void run(){
    printf("Coche en viaje\n");

    sleep(2);

    printf("Coche parado\n");
}

void unload(){
    sem_wait(&mutex);
    estado = BAJAR;
    printf("Ya pueden bajar los pasajeros\n");

    for(int i = 0; i< n_ocupantes; ++i){
        sem_post(&colaBajar);
    }

    if(n_ocupantes > 0){
        sem_post(&mutex);
        sem_wait(&esperaCoche);
        sem_wait(&mutex);
    }

    estado = NO;
    sem_post(&mutex);
}

void * Coche(){
    sem_wait(&mutexPAtend);
    while(n_pasajerosAtendidos < NPASAJEROS){
        sem_post(&mutexPAtend);
        load();    

        run();

        unload();
        sem_wait(&mutexPAtend);
    }

    sem_post(&mutexPAtend);

    printf("Todos los pasajeros han sido atendidos\n");

    pthread_exit(0);
}

void board(int ind){
    sem_wait(&mutex);

    if(estado != SUBIR || n_ocupantes == CAPACIDAD){
        sem_post(&mutex);
        sem_wait(&colaSubir);
        sem_wait(&mutex);
    }

    ++n_ocupantes;
    printf("El pasajero %d sube al coche\n", ind);

    if(n_ocupantes == CAPACIDAD){
        sem_post(&esperaCoche);
    }
    
    sem_post(&mutex);
}

void unboard(int ind){
    sem_wait(&mutex);

    if(estado != BAJAR){
        sem_post(&mutex);
        sem_wait(&colaBajar);
        sem_wait(&mutex);
    }

    --n_ocupantes;
    printf("El pasajero %d baja del coche\n", ind);

    if(n_ocupantes == 0){
        sem_post(&esperaCoche);
    }
    
    sem_post(&mutex);
}

void * Pasajero(void * arg){
    int ind = * (int*) arg;

    board(ind);

    unboard(ind);

    sem_wait(&mutexPAtend);
    ++n_pasajerosAtendidos;
    sem_post(&mutexPAtend);

    pthread_exit(0);
}

int main(){
    sem_init(&mutex, 0, 1);
    sem_init(&mutexPAtend, 0, 1);
    sem_init(&esperaCoche, 0, 1);
    sem_init(&colaSubir, 0, 0);
    sem_init(&colaBajar, 0, 0);

    pthread_t thCoche;
    pthread_create(&thCoche,NULL, Coche, NULL);

    pthread_t thPasajeros[NPASAJEROS];

    int *arg;
    for(int i = 0; i< NPASAJEROS; ++i){
        arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&thPasajeros[i],NULL,Pasajero,(void*)arg);
    }

    pthread_join(thCoche,NULL);
    for(int i = 0; i< NPASAJEROS; ++i){
        pthread_join(thPasajeros[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&mutexPAtend);
    sem_destroy(&esperaCoche);
    sem_destroy(&colaSubir);
    sem_destroy(&colaBajar);

    return 0;
}