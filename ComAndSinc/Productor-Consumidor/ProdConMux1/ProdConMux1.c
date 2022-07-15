#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

//Solucion del problema productor-consumidor con mutex para el caso en que solo hay un productor y un consumidor

#define MAX_BUFFER 10
#define DATOS_A_PRODUCIR 100

//Variables globales compartidas
int buf [MAX_BUFFER];
int nelementos = 0;

pthread_mutex_t mutex; //mutex para controlar el acceso al buffer compartido

pthread_cond_t lleno; //variable de condicion a esperar cuando el buf esta lleno
pthread_cond_t vacio; // variable de condicion a esperar cunado el buf esta vacio

void* Productor(){
    printf("Soy el hilo %ld y soy productor\n", pthread_self());
    fflush(NULL);

    int pos = 0;

    for (int i = 0; i  < DATOS_A_PRODUCIR; ++i){
        int dato = i;

        pthread_mutex_lock(&mutex); //acceder a la variable compartida nelementos
        
        while(nelementos == MAX_BUFFER){ //espera a que haya huecos libres para poder insertar el nuevo dato
            pthread_cond_wait(&lleno, &mutex);
        }

        buf[pos] = dato;
        pos = (pos+1)%MAX_BUFFER;

        printf("Hilo %ld : Acabo de producir el dato %d\n", pthread_self(), i);
        fflush(NULL);

        ++nelementos;

        if(nelementos == 1){ // si el buffer estaba vacio, avisamos de que ya no lo esta
            pthread_cond_signal(&vacio);
        }

        pthread_mutex_unlock(&mutex); //sale de la seccion critica
    }

    pthread_exit(0);
}

void* Consumidor(){
    printf("Soy el hilo %ld y soy consumidor\n", pthread_self());
    fflush(NULL);

    int pos = 0;

    for(int i = 0; i < DATOS_A_PRODUCIR; ++i){
        pthread_mutex_lock(&mutex); //acceder a la variable compartida nelementos

        while(nelementos == 0){//espera a que haya elementos que consumir
            pthread_cond_wait(&vacio, &mutex);
        }

        int dato = buf[pos];
        pos = (pos+1)%MAX_BUFFER;
        --nelementos;

        if(nelementos == MAX_BUFFER -1 ){ //si el buffer estaba lleno, avisamos de que ya no lo esta
            pthread_cond_signal(&lleno);
        }

        pthread_mutex_unlock(&mutex);

        printf("Hilo %ld : Acabo de consumir el dato %d\n", pthread_self(), dato);
        fflush(NULL);
    }

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&lleno, NULL);
    pthread_cond_init(&vacio, NULL);

    pthread_t th1, th2;
    pthread_create(&th1, NULL, Productor, NULL);
    pthread_create(&th2, NULL, Consumidor, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&lleno);
    pthread_cond_destroy(&vacio);

    //To allow other threads to continue execution, the main thread should terminate by calling pthread_exit() pero si me he cargado el mutex ira mal

    return 0;
}

