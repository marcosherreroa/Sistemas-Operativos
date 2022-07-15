#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define COMMAND_SIZE 1000
#define N_ARGSAUTOB 3
#define N_ARGSUSUAR 2

#define EN_RUTA 0 // autobús en ruta
#define EN_PARADA 1 // autobús en la parada

#define LINEAL 0
#define CIRCULAR 1

typedef struct Autob {
	char * nombre;
	int capacidad;
	int estado; //EN_RUTA o EN_PARADA
	int parada_actual;
	int n_ocupantes;

	pthread_mutex_t mutex; //mutex que protege ls atributos del autobus
	pthread_cond_t esperaBus; //para controlar la esperar del autobus en la parada

	int * esperando_bajar; //número de personas que desean bajar de este autobus en cada parada
	pthread_cond_t * paraBajar;
} Autobus;

typedef struct Usuar {
   char * nombre;
   Autobus * autob; //autobus en el que esta subido. NULL si no esta en ninguno
   int n_viajes; //numero de viajes que le quedan por realizar
   pthread_mutex_t mutex;
} Usuario;

typedef struct Parad { 
   char * nombre;
   Autobus * autobEnParada; // NULL si no hay
   int esperando_parada; //numero de personas esperando en la parada
   pthread_mutex_t mutex; //protege el campo esperando parada
   pthread_cond_t paraSubir;
} Parada;

Autobus ** arrayAutobuses; //array de punteros a autobuses
int n_autobuses = 0; // número de autobuses
int espAutobus = 1; //tamaño del array arrayAutobus

Usuario ** arrayUsuarios; //array de punteros a usuarios
int n_usuarios = 0; // número de usuarios
int espUsuarios = 1; //tamaño del array arrayUsuario

Parada ** arrayParadas; //array de punteros a paradas
int n_paradas; //número de paradas

void Autobus_En_Parada(Autobus * autob){
/* Ajustar el estado y bloquear al autobús hasta que no haya pasajeros que
quieran bajar y/o subir la parada actual. Después se pone en marcha */

	pthread_mutex_lock(&autob->mutex);
	
	
	Parada * parada = arrayParadas[autob->parada_actual];
	pthread_mutex_lock(&parada->mutex);

	if(parada-> autobEnParada != NULL){ // ya hay un autobus en ella, nos la saltamos
		printf("El autobus %s se salta la parada %s porque esta ocupada\n", autob->nombre, parada->nombre);
		pthread_mutex_unlock(&parada->mutex);
	}

	else{ // se para
		autob->estado = EN_PARADA;
		parada->autobEnParada = autob;
		printf("Autobus %s en parada %s\n", autob->nombre, parada->nombre);
		pthread_mutex_unlock(&parada->mutex);

		pthread_mutex_lock(&parada->mutex);
	
		while(autob->esperando_bajar[autob->parada_actual] || (parada->esperando_parada && autob->n_ocupantes < autob->capacidad)){
			if(autob->esperando_bajar[autob->parada_actual] != 0)pthread_cond_broadcast(&autob->paraBajar[autob->parada_actual]);
			else pthread_cond_broadcast(&parada->paraSubir);

			pthread_mutex_unlock(&parada->mutex);
			pthread_cond_wait(&autob->esperaBus,&autob->mutex);
			pthread_mutex_lock(&parada->mutex);
		}

		pthread_mutex_unlock(&parada->mutex);
		
		pthread_mutex_lock(&parada->mutex);
		autob->estado = EN_RUTA;
		parada->autobEnParada = NULL;
		printf("El autobus %s sale de la parada %s\n\n", autob->nombre, parada->nombre);
		pthread_mutex_unlock(&parada->mutex);

		}

	pthread_mutex_unlock(&autob->mutex);
}

void Conducir_Hasta_Siguiente_Parada(Autobus * autob){
	sleep(5);

	pthread_mutex_lock(&autob->mutex);
	autob->parada_actual = (autob->parada_actual + 1) % n_paradas;
	pthread_mutex_unlock(&autob->mutex);
}

void Subir_Autobus(Usuario * usuar, int origen){
	Parada * parada = arrayParadas[origen];

	pthread_mutex_lock(&parada->mutex);
	++parada->esperando_parada;

	int sube = 0;
	while(!sube){
	
		while(parada -> autobEnParada == NULL){
			pthread_cond_wait(&parada->paraSubir, &parada->mutex);
		}

		Autobus * autob = parada->autobEnParada;
		pthread_mutex_lock(&autob->mutex);

	    if(autob->n_ocupantes < autob->capacidad){
			++autob->n_ocupantes;
			--parada->esperando_parada;
			usuar -> autob = autob;
			printf("El usuario %s sube al autobus %s en la parada %s\n", usuar->nombre, autob->nombre, parada->nombre);
			sube = 1;

			if(parada->esperando_parada == 0)pthread_cond_signal(&autob->esperaBus); //no queda nadie por subir, se devuelve el control al autobus

			pthread_mutex_unlock(&autob->mutex);
		}
		else {
			pthread_cond_signal(&autob->esperaBus); //el autobus se ha llenado, se le pasa el control
			pthread_mutex_unlock(&autob->mutex);
			pthread_cond_wait(&parada-> paraSubir, &parada->mutex);
		}

	}
	
	pthread_mutex_unlock(&parada->mutex);
}

void Bajar_Autobus(Usuario * usuar, int destino){
	Autobus * autob = usuar->autob;

	pthread_mutex_lock(&autob->mutex);
	++autob->esperando_bajar[destino];

	while(autob->estado == EN_RUTA || autob->parada_actual != destino){
		pthread_cond_wait(&autob->paraBajar[destino], &autob->mutex);
	}
	
	--autob->n_ocupantes;
	printf("El usuario %s baja del autobus %s en la parada %s\n", usuar->nombre, autob->nombre, arrayParadas[destino]->nombre);
	--autob->esperando_bajar[destino]; 
	if(autob->esperando_bajar[destino] == 0)pthread_cond_signal(&autob->esperaBus);  //no queda nadie por bajar, se devuelve el control al autobus

	pthread_mutex_unlock(&autob->mutex);
}
/* El usuario indicará que quiere bajar en la parada ’destino’, esperará a que
el autobús se pare en dicha parada y bajará*/

void * thread_autobus(void * arg) {
	while (1) {

		// esperar a que los viajeros suban y bajen
		Autobus_En_Parada(arg);

		// conducir hasta siguiente parada
		Conducir_Hasta_Siguiente_Parada(arg);

	}

	pthread_exit(0);
}

void fUsuario(Usuario * usuar, int origen, int destino) {

// Esperar a que el autobus esté en parada origen para subir
	Subir_Autobus(usuar, origen);

// Bajarme en estación destino
	Bajar_Autobus(usuar , destino);
}

void * thread_usuario(void * arg) {
	int a,b;
	Usuario * usuar = (Usuario *) arg;
	int para = 0;

	while (!para) {
		a=rand() % n_paradas;
		do{
		b=rand() % n_paradas;
		} while(a==b);

		sleep(5);
		fUsuario(usuar,a,b);

		--usuar->n_viajes;
		if(usuar->n_viajes == 0) para = 1;

	}

	printf("El usuario %s ha terminado los viajes que tenia previstos\n", usuar->nombre);
	pthread_exit(0);
}

 Autobus* crearAutobus(char * saveptr){
	char * args [N_ARGSAUTOB];
	//char * delim = " ";
	int j = 0;

	while(j < N_ARGSAUTOB && (args[j] = strtok_r(NULL," \n",&saveptr)) != NULL) ++j;

	if (j < N_ARGSAUTOB){
		printf("-addAutobus: número incorrecto de argumentos. Uso: -addAutob nombre capacidad paradaInicial\n\n");
		return NULL;
	}

    int paradaInicial = atoi(args[2]);

	if(paradaInicial < 0 || paradaInicial >= n_paradas){
		printf("-addAutobus: el argumento paradaInicial ha de estar en el rango de 0 a %d\n\n",n_paradas-1);
		return NULL;
	}
	
	Autobus* autob = malloc(sizeof(Autobus));

	int l = strlen(args[0]);
	autob-> nombre = malloc(l+1);
	snprintf(autob->nombre,l+1,"%s",args[0]);

	autob ->capacidad = atoi(args[1]);
	autob -> parada_actual = paradaInicial;
	autob ->estado = EN_PARADA;
	autob ->n_ocupantes = 0;
	autob->esperando_bajar = calloc(n_paradas, sizeof(int));

	pthread_mutex_init(&autob->mutex, NULL);
	pthread_cond_init(&autob->esperaBus,NULL);
	autob->paraBajar = malloc(n_paradas*sizeof(pthread_cond_t));
	for(int i = 0; i< n_paradas; ++i){
		pthread_cond_init(&autob->paraBajar[i],NULL);
	}

	return autob;
}

Usuario * crearUsuario(char * saveptr){
	char * args [N_ARGSUSUAR];
	int j = 0;

	while(j < N_ARGSUSUAR && (args[j] = strtok_r(NULL," \n",&saveptr)) != NULL) ++j;

	if (j < N_ARGSUSUAR){
		printf("-addUsuario: número incorrecto de argumentos. Uso: -addUsuar nombre nViajes\n\n");
		return NULL;
	}

	int n_viajes = atoi(args[1]);
	if(n_viajes <= 0){
		printf("-addUsuario: el argumento n_viajes ha de ser positivo\n");
		return NULL;
	}
	
	Usuario * usuar = malloc(sizeof(Usuario));
	
	int l = strlen(args[0]);
	usuar-> nombre = malloc(l+1);
	snprintf(usuar->nombre,l+1,"%s",args[0]);

	usuar->autob = NULL;
	usuar->n_viajes = n_viajes;

	//pthread_mutex_init(&usuar->mutex, NULL);

	return usuar;
}

void destruirAutobus(Autobus * autob){
	free(autob->nombre);
	free(autob->esperando_bajar);

	pthread_mutex_destroy(&autob->mutex);
	pthread_cond_destroy(&autob->esperaBus);
	for(int i = 0; i< n_paradas; ++i){
		pthread_cond_destroy(&autob->paraBajar[i]);
	}
	free(autob->paraBajar);

	free(autob);
}

void destruirUsuario(Usuario * usuar){
	free(usuar->nombre);
	free(usuar);
}

int main(int argc, char * argv[]) {
	srand(time(NULL));
	int running;
	int pausa = 0;


	if(argc < 2){
		printf("Argumento necesario: numero de paradas\n\n");
		return -1;
	}
	
	n_paradas = atoi(argv[1]);
	arrayParadas = malloc(n_paradas*sizeof(Parada*));
	for(int i = 0; i< n_paradas; ++i){
		arrayParadas[i] = malloc(sizeof(Parada));

		int needed = snprintf(NULL, 0, "%d",i);
		arrayParadas[i]->nombre = malloc(needed+1);
		snprintf(arrayParadas[i]->nombre,needed+1, "%d",i);

		arrayParadas[i]->autobEnParada = NULL;
		arrayParadas[i]->esperando_parada = 0;
		pthread_mutex_init(&arrayParadas[i]->mutex, NULL);
		pthread_cond_init(&arrayParadas[i]->paraSubir,NULL);
	}

	arrayAutobuses = malloc(espAutobus*sizeof(Autobus*));
	arrayUsuarios = malloc(espUsuarios*sizeof(Usuario*));
	
	pthread_t ** thAutobuses;
	pthread_t ** thUsuarios;
	thAutobuses = malloc(espAutobus*sizeof(pthread_t *));
	thUsuarios = malloc(espUsuarios*sizeof(pthread_t *));

    char * comando = NULL;
	char * saveptr = NULL;
	size_t l = 0;

	running = 1;

	while(running){
		getline(&comando, &l, stdin);
		saveptr = NULL;

		char * palabra = strtok_r(comando," \n", &saveptr);

		if(strcmp(palabra,"-addAutobus") == 0){
			Autobus * autob = crearAutobus(saveptr);

			if(autob != NULL){
				
				if(espAutobus == n_autobuses){ //duplicamos el array
					espAutobus *=2;
					Autobus ** auxAutobuses = malloc(espAutobus*sizeof(Autobus*));
					pthread_t ** auxthAutobuses = malloc(espAutobus*sizeof(pthread_t*));

					for(int j = 0; j < n_autobuses; ++j){
						auxAutobuses[j] = arrayAutobuses[j];
						auxthAutobuses[j] = thAutobuses[j];
					}

					free(arrayAutobuses);
					free(thAutobuses);
					arrayAutobuses = auxAutobuses;
					thAutobuses = auxthAutobuses;
				}
				
				++n_autobuses;
				arrayAutobuses[n_autobuses-1] = autob;
				thAutobuses[n_autobuses-1] = malloc(sizeof(pthread_t));
				if(pausa) pthread_mutex_lock(&arrayAutobuses[n_autobuses-1]->mutex);
				printf("Añadido autobus de nombre %s y capacidad %d en la parada %d\n\n", autob->nombre, autob->capacidad, autob->parada_actual);
				fflush(NULL);
				pthread_create(thAutobuses[n_autobuses-1],NULL, thread_autobus, arrayAutobuses[n_autobuses-1]);				
			}
		}

		else if(strcmp(palabra,"-addUsuario") == 0){
			Usuario * usuar = crearUsuario(saveptr);

			if(usuar != NULL){

				if(espUsuarios == n_usuarios){ //duplicamos el array
					espUsuarios *=2;
					Usuario ** auxUsuarios = malloc(espUsuarios*sizeof(Usuario*));
					pthread_t ** auxthUsuarios = malloc(espUsuarios*sizeof(pthread_t*));

					for(int j = 0; j < n_usuarios; ++j){
						auxUsuarios[j] = arrayUsuarios[j];
						auxthUsuarios[j] = thUsuarios[j];
					}

					free(arrayUsuarios);
					free(thUsuarios);
					arrayUsuarios = auxUsuarios;
					thUsuarios = auxthUsuarios;
				}	
			

			++n_usuarios;
			arrayUsuarios[n_usuarios-1] = usuar;
			thUsuarios[n_usuarios-1] = malloc(sizeof(pthread_t));
			printf("Añadido usuario de nombre %s con %d viajes\n\n", usuar->nombre, usuar->n_viajes);
			fflush(NULL);
			pthread_create(thUsuarios[n_usuarios-1],NULL, thread_usuario, arrayUsuarios[n_usuarios-1]);
			}
		}

		else if(strcmp(palabra, "-pause") == 0){
			if (pausa) printf("La simulacion ya esta pausada\n\n");
			else{
				for(int i = 0; i< n_autobuses; ++i){
					pthread_mutex_lock(&arrayAutobuses[i]->mutex);
				}

				printf("Simulacion pausada\n\n");
				pausa = 1;
			}
		}

		else if(strcmp(palabra, "-resume") == 0){
			if(!pausa) printf("La simulacion no esta pausada\n\n");
			else{
				for(int i = 0; i< n_autobuses;++i){
					pthread_mutex_unlock(&arrayAutobuses[i]->mutex);
				}

				pausa = 0;
			}
		}

		else if(strcmp(palabra,"-help") == 0){
			printf("|||Lista de comandos|||\n");
			printf("-addAutobus nombre capacidad paradaInicial : añade un autobus\n");
			printf("-pause: pausa la simulacion\n");
			printf("-resume: reanuda la simulacion\n");
			printf("-addUsuario nombre nViajes: añade un usuario\n");
			printf("-help : muestra la lista de comandos\n");
			printf("-exit: termina la simulación\n\n");
		}
		
		else if(strcmp(palabra,"-exit") == 0){
			printf("Simulacion terminada\n\n");
			running = 0;
		}

		else{
			printf("Comando desconocido. Escriba -help para obtener una lista de los comandos disponibles\n\n");
		}
	}

	free(comando);

	if(pausa){
		for(int i = 0; i< n_autobuses; ++i){
			pthread_mutex_unlock(&arrayAutobuses[i]->mutex);
		}

		pausa = 0;
	}

	for(int i = 0; i< n_autobuses; ++i){
		pthread_cancel(*thAutobuses[i]);
		pthread_join(*thAutobuses[i], NULL);
		free(thAutobuses[i]);
	}

	free(thAutobuses);

	for(int i = 0; i< n_usuarios; ++i){
		pthread_cancel(*thUsuarios[i]);
		pthread_join(*thUsuarios[i], NULL);
		free(thUsuarios[i]);
	}

	free(thUsuarios);

	for(int i = 0; i< n_autobuses; ++i){
		destruirAutobus(arrayAutobuses[i]);
	}
	free(arrayAutobuses);

	for(int i = 0; i < n_usuarios; ++i){
		destruirUsuario(arrayUsuarios[i]);
	}
	free(arrayUsuarios);

	for(int i = 0; i < n_paradas; ++i){
		free(arrayParadas[i]->nombre);
		pthread_mutex_destroy(&arrayParadas[i]->mutex);
		pthread_cond_destroy(&arrayParadas[i]->paraSubir);
		free(arrayParadas[i]);
	}

	free(arrayParadas);

	return 0;
}


