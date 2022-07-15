#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define N 10

//Solución del problema de los lectores-escritores con mutex. Da prioridad a los escritores.
//Cuando los semáforos utilizados son de valor 1, las osluciones son anáálogas con mutex


//Recurso compartido
int data = 0;

int nlectores = 0; //numero de lectores actualmente leyendo
int nescritores = 0; //numero de escritores compitiendo por el recurso. Mientras haya alguno, no podra haber ningun lector compitiendo

pthread_mutex_t mutlectores; //controla el acceso a la variable nlectores (1: no hay nadie accediendo, 0: hay alguien accediendo(solo puede haber uno))
pthread_mutex_t mutescritores; //controla el acceso a la variable nescritores (1: no hay nadie accediendo, 0: hay alguien accediendo (solo puede haber uno))
pthread_mutex_t mutEsperaLectores; //controla que los lectores no se adelanten a los escritores que ya estan intentando acceder al recurso
pthread_mutex_t mutrecurso; //controla el acceso al recurso (1: no hay nadie accediendo, 0: hay alguien accediendo (1 escritor o varios lectores))

void * Lector(){
    printf("Soy el hilo %ld y soy un lector\n", pthread_self());

    pthread_mutex_lock(&mutEsperaLectores); //antes de entrar al recurso, espera a que no haya escritores que compitiendo por el
    pthread_mutex_unlock(&mutEsperaLectores); //otros lectores y escritores han de poder entrar a competir ahora, mientras que este esta esperando a hacerse con el
                                 // recurso para los lectores
    //No estoy seguro de esto

    pthread_mutex_lock(&mutlectores); //intenta acceder a la variable compartida nlectores para incrementarla
    
    ++nlectores;

    if(nlectores == 1){ //si es el primer lector, tiene que reservar el recurso antes de usarlo
        pthread_mutex_lock(&mutrecurso);// espera a que este libre el recurso (salga un escritor, si es que lo hay)
    }

    //una vez ya ha sido reservado por un lector, el recurso queda reservado para todos. Solo se dejara accesible a los escritores cuando el numero de lectores
    // baje a 0

    pthread_mutex_unlock(&mutlectores); //libera el acceso a la variable nlectores

    printf("Hilo %ld: He leido %d\n", pthread_self(), data); //acceso al recurso
    fflush(NULL);

    pthread_mutex_lock(&mutlectores); //intenta acceder a la variable compartida nlectores para reducirla

    --nlectores;

    if(nlectores == 0){ // si ya no quedan lectores leyendo, hay que liberar el recurso para los escritores
        pthread_mutex_unlock(&mutrecurso);
    }

    pthread_mutex_unlock(&mutlectores); //libera el acceso a la variable nlectores

    pthread_exit(0);
}

void * Escritor(){
    printf("Soy el hilo %ld y soy un escritor\n", pthread_self());

    pthread_mutex_lock(&mutescritores);// intenta acceder a la variable compartida nescritores para incrementarla

    ++nescritores;

    if(nescritores == 1){//primer escritor pidiendo acceso ha de impedir que los lectores que vengan adelanten a ningun escritor.
        pthread_mutex_lock(&mutEsperaLectores);
    }

    pthread_mutex_unlock(&mutescritores);//libera el acceso a la variable nescritores. Importante el orden de los wait

    pthread_mutex_lock(&mutrecurso); //espera a que esté libre el recurso (salgan todos los lectores o el escritor que lo esté usando)

    pthread_mutex_unlock(&mutescritores); //intenta acceder a la variable compartida nescritores para reducirla

    --nescritores;

    if(nescritores == 0){ //si ya no quedan escritores compitiendo, se deja a los lectores competir. Se deja a los lectores empezar a posicionarse antes de que
        pthread_mutex_unlock(&mutEsperaLectores);
    }

    pthread_mutex_unlock(&mutescritores);//libera el acceso a la variable nescritores. Importante el orden de los wait

    ++data;
    printf("Hilo %ld: He escrito %d\n", pthread_self(), data);
    fflush(NULL);

    pthread_mutex_unlock(&mutrecurso); // libera el recurso para los lectores u otro escritor

    pthread_exit(0);
}

int main(){
    pthread_mutex_init(&mutlectores,NULL);
    pthread_mutex_init(&mutescritores, NULL);
    pthread_mutex_init(&mutEsperaLectores, NULL);
    pthread_mutex_init(&mutrecurso,NULL);

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

    pthread_mutex_destroy(&mutlectores);
    pthread_mutex_destroy(&mutescritores);
    pthread_mutex_destroy(&mutEsperaLectores);
    pthread_mutex_destroy(&mutrecurso);

    return 0;
}

