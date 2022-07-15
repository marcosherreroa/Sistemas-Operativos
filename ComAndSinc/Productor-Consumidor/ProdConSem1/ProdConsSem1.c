#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

//Solucion del problema productor-consumidor con semáforos para el caso en que solo hay un productor y un consumidor

#define MAX_BUFFER 10
#define DATOS_A_PRODUCIR 100

int buf [MAX_BUFFER];

sem_t elems, huecos;

void* Productor(){
    printf("Soy el hilo %ld y soy productor\n", pthread_self());
    fflush(NULL);

    int pos = 0;

    for (int i = 0; i  < DATOS_A_PRODUCIR; ++i){
        int dato = i;

        sem_wait(&huecos); //espera a que haya huecos libres para poder insertar el nuevo dato

        buf[pos] = dato;
        pos = (pos+1)%MAX_BUFFER;
        printf("Hilo %ld : Acabo de producir el dato %d\n", pthread_self(), i);
        fflush(NULL);

        sem_post(&elems); //comunica que hay un nuevo elemento disponible para consumir
    }

    pthread_exit(0);
}

void* Consumidor(){
    printf("Soy el hilo %ld y soy consumidor\n", pthread_self());
    fflush(NULL);

    int pos = 0;

    for(int i = 0; i < DATOS_A_PRODUCIR; ++i){
        sem_wait(&elems); //espera a que haya elementos que consumir

        int dato = buf[pos];
        pos = (pos+1)%MAX_BUFFER;

        sem_post(&huecos); //comunica que hay un hueco libre para un nuevo elemento

        printf("Hilo %ld : Acabo de consumir el dato %d\n", pthread_self(), dato);
        fflush(NULL);
    }

    pthread_exit(0);
}

int main(){
    sem_init(&elems, 0, 0);
    sem_init(&huecos, 0, MAX_BUFFER);

    pthread_t th1, th2;
    pthread_create(&th1, NULL, Productor, NULL);
    pthread_create(&th2, NULL, Consumidor, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    sem_destroy(&elems);
    sem_destroy(&huecos);

    //To allow other threads to continue execution, the main thread should terminate by calling pthread_exit() rather than exit(3).
    // es decir, poniendo pthread_exit(0) en vez de return 0, podemos ahorrarnos los join

    return 0;
}

