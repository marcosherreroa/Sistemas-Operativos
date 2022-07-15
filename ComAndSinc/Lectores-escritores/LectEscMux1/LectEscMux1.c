#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 10

//Solución del problema de los lectores-escritores con mutex. Da prioridad a los lectores
//Esta mal



//Recurso compartido
int data = 0;

int nlectores = 0; //numero de lectores actualmente leyendo

pthread_mutex_t mutexlectores; //controla el acceso a la variable nlectores (1: no hay nadie accediendo, 0: hay alguien accediendo(solo puede haver uno))
pthread_mutex_t mutexrecurso; //controla el acceso al recurso (1: no hay nadie accediendo, 0: hay alguien accediendo (1 escritor o varios lectores))

void * Lector(){
    printf("Soy el hilo %ld y soy un lector\n", pthread_self());

    pthread_mutex_lock(&mutexlectores); //intenta acceder a la variable compartida nlectores para incrementarla

    ++nlectores;

    if(nlectores == 1){ //si es el primer lector, tiene que reservar el recurso antes de usarlo
        pthread_mutex_lock(&mutexrecurso);// espera a que este libre el recurso (salga un escritor, si es que lo hay)
    }

    //una vez ya ha sido reservado por un lector, el recurso queda reservado para todos. Solo se dejara accesible a los escritores cuando el numero de lectores
    // baje a 0

    pthread_mutex_unlock(&mutexlectores); //libera el acceso a la variable nlectores

    printf("Hilo %ld: He leído %d\n", pthread_self(), data);
    fflush(NULL);

    pthread_mutex_lock(&mutexlectores); //intenta acceder a la variable compartida nlectores para reducirla

    --nlectores;

    if(nlectores == 0){ // si ya no quedan lectores leyendo, hay que liberar el recurso para los escritores
        pthread_mutex_unlock(&mutexrecurso);
    }

    pthread_mutex_unlock(&mutexlectores); //libera el acceso a la variable nlectores

    pthread_exit(0);
}

void * Escritor(){
    printf("Soy el hilo %ld y soy un escritor\n", pthread_self());

    pthread_mutex_lock(&mutexrecurso); //espera a que esté libre el recurso (salgan todos los lectores o el escritor que lo esté usando)

    ++data;
    printf("Hilo %ld: He escrito %d\n", pthread_self(), data);
    fflush(NULL);

    pthread_mutex_unlock(&mutexrecurso); // libera el recurso para los lectores u otro escritor

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutexlectores, NULL);
    pthread_mutex_init(&mutexrecurso, NULL);

    pthread_t thlect [N];
    pthread_t thesc [N];
    
    for(int i = 0; i < N; ++i){
        pthread_create(&thlect[i],NULL,Lector,NULL);
        pthread_create(&thesc[i],NULL,Escritor,NULL);
    }
   
    for(int i = 0; i < N; ++i){
        pthread_join(thlect[i], NULL);
        pthread_join(thesc[i], NULL);
    }

    pthread_mutex_destroy(&mutexlectores);
    pthread_mutex_destroy(&mutexrecurso);

    exit(0);
}

