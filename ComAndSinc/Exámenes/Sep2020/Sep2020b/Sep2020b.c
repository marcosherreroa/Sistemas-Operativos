#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Variables compartidas. 
//A 1 cuando el hilo ya ha mostrado su mensaje
int tA = 0;
int tB = 0;
int tC = 0;
int tD = 0;

//para conocer en qué orden se ha ejecutado
int ord = 0;


//Mecanismos de sincronizacion

pthread_mutex_t muttX; //acceso a las variables tX
pthread_cond_t cBC; //esperar a que termine A
pthread_cond_t cD; //esperar a que terminen B y C
pthread_cond_t cE; // esperar a que termine D ( y C, pero esto ya esta incluido en que termine D)

pthread_mutex_t mutord; //acceso a la variable ord


void * A(void * arg){
    char * array = (char *) arg;

    pthread_mutex_lock(&mutord);
    printf("Soy el hilo A\n");
    array[ord] = 'A';
    ++ord;
    pthread_mutex_unlock(&mutord);

    pthread_mutex_lock(&muttX);
    tA = 1;
    pthread_cond_broadcast(&cBC);
    pthread_mutex_unlock(&muttX);

    pthread_exit(0);
}

void * B(void * arg){
    char * array = (char *) arg;

    pthread_mutex_lock(&muttX);
    while(!tA){
        pthread_cond_wait(&cBC, &muttX);
    }
    pthread_mutex_unlock(&muttX);

    pthread_mutex_lock(&mutord);
    printf("Soy el hilo B\n");
    array[ord] = 'B';
    ++ord;
    pthread_mutex_unlock(&mutord);
    
    pthread_mutex_lock(&muttX);
    tB = 1;
    if(tC){
        pthread_cond_signal(&cD);
    }
    pthread_mutex_unlock(&muttX);

    pthread_exit(0);
}

void * C(void * arg){
    char * array = (char *) arg;

    pthread_mutex_lock(&muttX);
    while(!tA){
        pthread_cond_wait(&cBC, &muttX);
    }
    pthread_mutex_unlock(&muttX);

    pthread_mutex_lock(&mutord);
    printf("Soy el hilo C\n");
    array[ord] = 'C';
    ++ord;
    pthread_mutex_unlock(&mutord);

    pthread_mutex_lock(&muttX);
    tC = 1;
    if(tB){
        pthread_cond_signal(&cD);
    }
    pthread_mutex_unlock(&muttX);

    pthread_exit(0);
}

void * D(void * arg){
    char * array = (char *) arg;

    pthread_mutex_lock(&muttX);
    while(!tB || !tC){
        pthread_cond_wait(&cD, &muttX);
    }
    pthread_mutex_unlock(&muttX);

    pthread_mutex_lock(&mutord);
    printf("Soy el hilo D\n");
    array[ord]= 'D';
    ++ord;
    pthread_mutex_unlock(&mutord);

    pthread_mutex_lock(&muttX);
    tD = 1;
    pthread_cond_signal(&cE);
    pthread_mutex_unlock(&muttX);

    pthread_exit(0);
}

void * E(void * arg){
    char * array = (char *) arg;

    pthread_mutex_lock(&muttX);
    while(!tD){
        pthread_cond_wait(&cE,&muttX);
    }
    pthread_mutex_unlock(&muttX);

    pthread_mutex_lock(&mutord);
    printf("Soy el hilo E\n");
    array[ord] = 'E';
    ++ord;

    pthread_mutex_unlock(&mutord);
    pthread_exit(0);
}

int main(){
    pthread_t thA, thB, thC, thD, thE;
    char * array = malloc(5*sizeof(char));

    pthread_create(&thA, NULL, A, array);
    pthread_create(&thB, NULL, B, array);
    pthread_create(&thC, NULL, C, array);
    pthread_create(&thD, NULL, D, array);
    pthread_create(&thE, NULL, E, array);

    pthread_join(thA,NULL);
    pthread_join(thB,NULL);
    pthread_join(thC,NULL);
    pthread_join(thD,NULL);
    pthread_join(thE,NULL);

    printf("Array: ");
    for(int i = 0; i < 5; ++i){
        printf(" %c ", array[i]);
    }
    printf("\n");

    return 0;
}