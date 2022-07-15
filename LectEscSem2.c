#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define N 10

//Solución del problema de los lectores-escritores con semaforos. Da prioridad a los escritores


//Recurso compartido
int data = 0;

int nlectores = 0; //numero de lectores actualmente leyendo
int nescritores = 0; //numero de escritores compitiendo por el recurso. Mientras haya alguno, no podra haber ningun lector compitiendo

sem_t semlectores; //controla el acceso a la variable nlectores (1: no hay nadie accediendo, 0: hay alguien accediendo(solo puede haber uno))
sem_t semescritores; //controla el acceso a la variable nescritores (1: no hay nadie accediendo, 0: hay alguien accediendo (solo puede haber uno))
sem_t semEsperaLectores; //controla que los lectores no se adelanten a los escritores que ya estan intentando acceder al recurso
sem_t semrecurso; //controla el acceso al recurso (1: no hay nadie accediendo, 0: hay alguien accediendo (1 escritor o varios lectores))

void * Lector(){
    printf("Soy el hilo %ld y soy un lector\n", pthread_self());

    sem_wait(&semEsperaLectores); //antes de entrar al recurso, espera a que no haya escritores que compitiendo por el
    sem_post(&semEsperaLectores); //otros lectores y escritores han de poder entrar a competir ahora, mientras que este esta esperando a hacerse con el
                                 // recurso para los lectores
    //No estoy seguro de esto

    sem_wait(&semlectores); //intenta acceder a la variable compartida nlectores para incrementarla
    
    ++nlectores;

    if(nlectores == 1){ //si es el primer lector, tiene que reservar el recurso antes de usarlo
        sem_wait(&semrecurso);// espera a que este libre el recurso (salga un escritor, si es que lo hay)
    }

    //una vez ya ha sido reservado por un lector, el recurso queda reservado para todos. Solo se dejara accesible a los escritores cuando el numero de lectores
    // baje a 0

    sem_post(&semlectores); //libera el acceso a la variable nlectores

    printf("Hilo %ld: He leido %d\n", pthread_self(), data); //acceso al recurso
    fflush(NULL);

    sem_wait(&semlectores); //intenta acceder a la variable compartida nlectores para reducirla

    --nlectores;

    if(nlectores == 0){ // si ya no quedan lectores leyendo, hay que liberar el recurso para los escritores
        sem_post(&semrecurso);
    }

    sem_post(&semlectores); //libera el acceso a la variable nlectores

    pthread_exit(0);
}

void * Escritor(){
    printf("Soy el hilo %ld y soy un escritor\n", pthread_self());

    sem_wait(&semescritores);// intenta acceder a la variable compartida nescritores para incrementarla

    ++nescritores;

    if(nescritores == 1){//primer escritor pidiendo acceso ha de impedir que los lectores que vengan adelanten a ningun escritor.
        sem_wait(&semEsperaLectores);
    }

    sem_post(&semescritores);//libera el acceso a la variable nescritores. Importante el orden de los wait

    sem_wait(&semrecurso); //espera a que esté libre el recurso (salgan todos los lectores o el escritor que lo esté usando)

    sem_wait(&semescritores); //intenta acceder a la variable compartida nescritores para reducirla

    --nescritores;

    if(nescritores == 0){ //si ya no quedan escritores compitiendo, se deja a los lectores competir. Se deja a los lectores empezar a posicionarse antes de que
        sem_post(&semEsperaLectores);
    }

    sem_post(&semescritores);//libera el acceso a la variable nescritores. Importante el orden de los wait

    ++data;
    printf("Hilo %ld: He escrito %d\n", pthread_self(), data);
    fflush(NULL);

    sem_post(&semrecurso); // libera el recurso para los lectores u otro escritor

    pthread_exit(0);
}

int main(){
    sem_init(&semlectores,0, 1);
    sem_init(&semescritores, 0,1);
    sem_init(&semEsperaLectores, 0, 1);
    sem_init(&semrecurso,0,1);

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

    sem_destroy(&semlectores);
    sem_destroy(&semescritores);
    sem_destroy(&semEsperaLectores);
    sem_destroy(&semrecurso);

    return 0;
}

