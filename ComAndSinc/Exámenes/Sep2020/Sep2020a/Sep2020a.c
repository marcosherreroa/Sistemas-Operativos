#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//Variables compartidas. A 1 cuando el hilo ya ha mostrado su mensaje
int tA = 0;
int tB = 0;
int tC = 0;
int tD = 0;

//Mecanismos de sincronizacion
pthread_mutex_t mut;
pthread_cond_t cBC;
pthread_cond_t cD;
pthread_cond_t cE;

void * A(){
    printf("Soy el hilo A\n");

    pthread_mutex_lock(&mut);

    tA = 1;
    pthread_cond_broadcast(&cBC);

    pthread_mutex_unlock(&mut);
    pthread_exit(0);
}

void * B(){
    pthread_mutex_lock(&mut);

    while(!tA){
        pthread_cond_wait(&cBC, &mut);
    }

    pthread_mutex_unlock(&mut);

    printf("Soy el hilo B\n");

    pthread_mutex_lock(&mut);
    tB = 1;

    if(tC){
        pthread_cond_signal(&cD);
    }

    pthread_mutex_unlock(&mut);
    pthread_exit(0);
}

void * C(){
    pthread_mutex_lock(&mut);

    while(!tA){
        pthread_cond_wait(&cBC, &mut);
    }

    pthread_mutex_unlock(&mut);

    printf("Soy el hilo C\n");

    pthread_mutex_lock(&mut);

    tC = 1;

    if(tB){
        pthread_cond_signal(&cD);
    }
    pthread_mutex_unlock(&mut);
    pthread_exit(0);
}

void * D(){
    pthread_mutex_lock(&mut);

    while(!tB || !tC){
        pthread_cond_wait(&cD, &mut);
    }

    pthread_mutex_unlock(&mut);

    printf("Soy el hilo D\n");

    pthread_mutex_lock(&mut);

    tD = 1;
    pthread_cond_signal(&cE);

    pthread_mutex_unlock(&mut);
    pthread_exit(0);
}

void * E(){
    pthread_mutex_lock(&mut);

    while(!tD){
        pthread_cond_wait(&cE,&mut);
    }

    pthread_mutex_unlock(&mut);

    printf("Soy el hilo E\n");
    pthread_exit(0);
}

int main(){
    pthread_t thA, thB, thC, thD, thE;

    pthread_create(&thA, NULL, A, NULL);
    pthread_create(&thB, NULL, B, NULL);
    pthread_create(&thC, NULL, C, NULL);
    pthread_create(&thD, NULL, D, NULL);
    pthread_create(&thE, NULL, E, NULL);

    pthread_join(thA,NULL);
    pthread_join(thB,NULL);
    pthread_join(thC,NULL);
    pthread_join(thD,NULL);
    pthread_join(thE,NULL);

    return 0;
}