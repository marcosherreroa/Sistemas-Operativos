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

pthread_mutex_t mutex; //protege el acceso a estado y n_ocupantes
pthread_mutex_t mutexPAtend; //protege el acceso a n_pasajerosAtendidos
pthread_cond_t esperaSubir; //esperan los pasajeros para subir
pthread_cond_t esperaBajar; //esperan los pasajeros para bajar
pthread_cond_t esperaCoche; //espera el coche cuando esta parado y aun no se ha llenado

void load(){
    pthread_mutex_lock(&mutex);
    estado = SUBIR;
    printf("Ya pueden subir los siguientes pasajeros\n");

    pthread_cond_broadcast(&esperaSubir);

    if(n_ocupantes < CAPACIDAD){
        pthread_cond_wait(&esperaCoche, &mutex);
    }

    estado = NO;
    pthread_mutex_unlock(&mutex);
}

void run(){
    printf("Coche en viaje\n");

    sleep(2);

    printf("Coche parado\n");
}

void unload(){
    pthread_mutex_lock(&mutex);
    estado = BAJAR;
    printf("Ya pueden bajar los pasajeros\n");

    pthread_cond_broadcast(&esperaBajar);

    if(n_ocupantes > 0){
        pthread_cond_wait(&esperaCoche, &mutex);
    }

    estado = NO;
    pthread_mutex_unlock(&mutex);
}

void * Coche(){
    pthread_mutex_lock(&mutexPAtend);
    while(n_pasajerosAtendidos < NPASAJEROS){
        pthread_mutex_unlock(&mutexPAtend);
        load();    

        run();

        unload();
        pthread_mutex_lock(&mutexPAtend);
    }

    pthread_mutex_unlock(&mutexPAtend);

    printf("Todos los pasajeros han sido atendidos\n");

    pthread_exit(0);
}

void board(int ind){
    pthread_mutex_lock(&mutex);

    while(estado != SUBIR || n_ocupantes == CAPACIDAD){
        pthread_cond_wait(&esperaSubir, &mutex);
    }

    ++n_ocupantes;
    printf("El pasajero %d sube al coche\n", ind);

    if(n_ocupantes == CAPACIDAD){
        pthread_cond_signal(&esperaCoche);
    }
    
    pthread_mutex_unlock(&mutex);
}

void unboard(int ind){
    pthread_mutex_lock(&mutex);

    while(estado != BAJAR){
        pthread_cond_wait(&esperaBajar, &mutex);
    }

    --n_ocupantes;
    printf("El pasajero %d baja del coche\n", ind);

    if(n_ocupantes == 0){
        pthread_cond_signal(&esperaCoche);
    }
    
    pthread_mutex_unlock(&mutex);
}

void * Pasajero(void * arg){
    int ind = * (int*) arg;

    board(ind);

    unboard(ind);

    pthread_mutex_lock(&mutexPAtend);
    ++n_pasajerosAtendidos;
    pthread_mutex_unlock(&mutexPAtend);

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexPAtend, NULL);
    pthread_cond_init(&esperaCoche, NULL);
    pthread_cond_init(&esperaSubir, NULL);
    pthread_cond_init(&esperaBajar, NULL);

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

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexPAtend);
    pthread_cond_destroy(&esperaCoche);
    pthread_cond_destroy(&esperaSubir);
    pthread_cond_destroy(&esperaBajar);

    return 0;
}