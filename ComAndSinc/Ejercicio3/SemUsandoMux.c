#include <pthread.h>
#include <stdio.h>

//Seguramente esto esté mal, el profe hace otra cosa. Cual es el problema?

typedef struct{
    int valor;

    pthread_mutex_t mutex;
    pthread_cond_t espera;
} sem_t;

void sem_init(sem_t * sem, int valor){
    sem->valor = valor;
    pthread_mutex_init(&(sem->mutex), NULL);
    ptread_cond_init(&(sem->espera));
}

void sem_destroy(sem_t * sem){
    pthread_mutex_destroy(&(sem->mutex));
    pthread_cond_destroy(&(sem->espera));
}

void sem_wait(sem_t * sem){
    pthread_mutex_lock(&(sem->mutex));
    --(sem->valor);

    while(sem->valor < 0){
        pthread_cond_wait(&(sem->espera), &(sem->mutex));
    }

    pthread_mutex_unlock(&(sem->mutex));
}

void sem_post(sem_t * sem){
    pthread_mutex_lock(&(sem->mutex));
    ++(sem->valor);

    if(sem->valor > 0){
        pthread_cond_signal(&(sem->espera), &(sem->mutex));
    }

    pthread_mutex_unlock(&(sem->mutex));
}