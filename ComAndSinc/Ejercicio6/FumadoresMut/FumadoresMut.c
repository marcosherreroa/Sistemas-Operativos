#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int enMesa [3];

pthread_mutex_t mutex;
pthread_cond_t esperaAgente;
pthread_cond_t esperaFumador [3];

void * Agente(){
    pthread_mutex_lock(&mutex);

    while(1){
        for(int i = 0; i < 3; ++i){
            enMesa[(i+1)%3] = 1;
            enMesa[(i+2)%3] = 1;

            pthread_cond_signal(&esperaFumador[i]);
            while(enMesa[(i+1)%3] || enMesa[(i+2)%3]){
                pthread_cond_wait(&esperaAgente, &mutex);
            }

        }

        sleep(1);
    }

    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void * Fumador(void * arg){
    int ingred = (int) arg;

    while(1){

        pthread_mutex_lock(&mutex);

        while(!enMesa[(ingred+1)%3] || !enMesa[(ingred+2)%3]){
            pthread_cond_wait(&esperaFumador[ingred],&mutex);
        }

        enMesa[(ingred+1)%3] = 0;
        enMesa[(ingred+2)%3] = 0;

        printf("El fumador %d fuma\n",ingred);

        pthread_cond_signal(&esperaAgente);

        pthread_mutex_unlock(&mutex);

    }

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&esperaAgente, NULL);
    
    for(int i = 0; i < 3; ++i) pthread_cond_init(&esperaFumador[i], NULL);

    pthread_t thAgente;
    pthread_t thFumador[3];

    pthread_create(&thAgente,NULL, Agente, NULL);

    for(int i = 0; i < 3; ++i) pthread_create(&thFumador[i], NULL, Fumador, i);

    pthread_join(thAgente, NULL);

    pthread_cond_destroy(&esperaAgente);
}