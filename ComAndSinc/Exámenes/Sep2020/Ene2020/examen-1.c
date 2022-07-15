#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5
#define M 2
#define MBUFFER 10
#define MAXITEMS 70

int suma=0;
int Buffer[MBUFFER];
int nproducciones = 0,nextracciones=0;

pthread_mutex_t mutex;
pthread_cond_t esperaProd;
pthread_cond_t esperaCons;

int indProd = 0, indCons = 0;
int elems = 0;

void * fun1(void * arg){
    int arg1 = (int) arg;
    printf("Hilo tipo 1 con id %ld. Mi argumento de entrada es %d\n", pthread_self(), arg1);

    pthread_mutex_lock(&mutex);

    while(nproducciones < MAXITEMS){
        ++nproducciones;

        while(elems == MBUFFER){
            pthread_cond_wait(&esperaProd, &mutex);
        }

        Buffer[indProd] = arg1;
        indProd = (indProd+1)%MBUFFER;
        ++elems;
        pthread_cond_signal(&esperaCons);

        fprintf(stderr, "Productor %ld, he añadido un item %d\n", pthread_self(), arg1);

        printf("El número total de producciones es %d\n", nproducciones);
        
    }
    
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void * fun2(){

    printf("Hilo tipo 2 con id %ld\n", pthread_self());

    pthread_mutex_lock(&mutex);

    while(nextracciones < MAXITEMS){
        ++nextracciones;

        while(elems == 0){
            pthread_cond_wait(&esperaCons, &mutex);
        }

        suma += Buffer[indCons];
        indCons = (indCons+1)%MBUFFER;
        --elems;
        pthread_cond_signal(&esperaProd);

        fprintf(stderr, "Consumidor %ld, valor extraido %d, acumulado %d, extracciones %d\n", pthread_self(), Buffer[indCons], suma, nextracciones);
    }

    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}


int main(){
    unsigned long int i;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&esperaProd, NULL);
    pthread_cond_init(&esperaCons, NULL);

    pthread_t ths1[N];
    pthread_t ths2[M];

    for(i = 0; i < N; ++i){
        pthread_create(&ths1[i], NULL, fun1, (void *) i+1);
    }

    for(i = 0; i< M; ++i){
        pthread_create(&ths2[i], NULL, fun2, NULL);
    }

    for(i = 0; i < N; ++i){
        pthread_join(ths1[i], NULL);
    }

    int devueltoConsumidores[M];

    for(i = 0; i < M; ++i){
        pthread_join(ths2[i], (void *) &devueltoConsumidores[i]);
    }

    for(i = 0; i < M; ++i){
        printf("%d\n", devueltoConsumidores[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&esperaProd);
    pthread_cond_destroy(&esperaCons);

    return suma;
}