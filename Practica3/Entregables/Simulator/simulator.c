#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define EN_RUTA 0 // autobús en ruta
#define EN_PARADA 1 // autobús en la parada


// estado inicial
int estado = EN_RUTA;
int parada_actual = 0; // parada en la que se encuentra el autobus
int n_ocupantes = 0; // ocupantes que tiene el autobús
int capacidad; //capacidad del autobus
int n_usuarios; // número de usuarios
int n_paradas; // número de paradas

int * n_viajes; // array con el número de viajes que le quedan por realizar a cada usuario


// personas que desean subir en cada parada
int * esperando_parada;
// personas que desean bajar en cada parada
int * esperando_bajar;
// Otras definiciones globales (comunicación y sincronización)

pthread_mutex_t mutAutobus;
pthread_cond_t * paraSubir;
pthread_cond_t * paraBajar;
pthread_cond_t esperaBus;


void Autobus_En_Parada(){
/* Ajustar el estado y bloquear al autobús hasta que no haya pasajeros que
quieran bajar y/o subir la parada actual. Después se pone en marcha */
	pthread_mutex_lock(&mutAutobus);
	estado = EN_PARADA;

	printf("Parada %d\n", parada_actual);

    while(esperando_bajar[parada_actual] || (esperando_parada[parada_actual] && n_ocupantes < capacidad)){
		if(esperando_bajar[parada_actual] != 0) pthread_cond_broadcast(&paraBajar[parada_actual]);
		else pthread_cond_broadcast(&paraSubir[parada_actual]);
		pthread_cond_wait(&esperaBus,&mutAutobus);
	}

	estado = EN_RUTA;
	printf("¡En marcha!\n\n");

	pthread_mutex_unlock(&mutAutobus);
}
void Conducir_Hasta_Siguiente_Parada(){
	sleep(5);

	pthread_mutex_lock(&mutAutobus);
	parada_actual = (parada_actual + 1) % n_paradas;
	pthread_mutex_unlock(&mutAutobus);
}
void Subir_Autobus(int id_usuario, int origen){
	pthread_mutex_lock(&mutAutobus);

	++esperando_parada[origen];

	while(estado == EN_RUTA || parada_actual != origen || esperando_bajar[origen] > 0 || capacidad == n_ocupantes){
		pthread_cond_wait(&paraSubir[origen], &mutAutobus);

		if(capacidad == n_ocupantes)pthread_cond_signal(&esperaBus); //el autobus se ha llenado, se le pasa el control
	}

	++n_ocupantes;
	printf("El usuario %d sube al autobus\n", id_usuario);

	--esperando_parada[origen];
	if(esperando_parada[origen] == 0)pthread_cond_signal(&esperaBus); //no queda nadie por subir, se devuelve el control al autobus
	
	pthread_mutex_unlock(&mutAutobus);
}
void Bajar_Autobus(int id_usuario, int destino){
	pthread_mutex_lock(&mutAutobus);

	++esperando_bajar[destino];

	while(estado == EN_RUTA || parada_actual != destino){
		pthread_cond_wait(&paraBajar[destino], &mutAutobus);
	}
	
	--n_ocupantes;
	printf("El usuario %d baja del autobus\n", id_usuario);
	--esperando_bajar[destino]; 
	if(esperando_bajar[destino] == 0)pthread_cond_signal(&esperaBus);

	pthread_mutex_unlock(&mutAutobus);
}
/* El usuario indicará que quiere bajar en la parada ’destino’, esperará a que
el autobús se pare en dicha parada y bajará. El id_usuario puede utilizarse para
propo*/

void * thread_autobus(void * args) {
	int conduce = 1;
	while (conduce) {

		// esperar a que los viajeros suban y bajen
		Autobus_En_Parada();

		// conducir hasta siguiente parada
		Conducir_Hasta_Siguiente_Parada();

		conduce = 0; //el autobus segura ciruclando mientras a algun usario le quede algun viaje por realizar
		for(int i = 0; !conduce && i< n_usuarios; ++i){
			if(n_viajes[i]) {
				conduce = 1;
			}
		}
	}

	pthread_exit(0);
}

void Usuario(int id_usuario, int origen, int destino) {

// Esperar a que el autobus esté en parada origen para subir
	Subir_Autobus(id_usuario, origen);

// Bajarme en estación destino
	Bajar_Autobus(id_usuario, destino);
}

void * thread_usuario(void * arg) {
	int a,b;
	int id_usuario = *(int *) arg;
	int para = 0;

	while (!para) {
		a=rand() % n_paradas;
		do{
		b=rand() % n_paradas;
		} while(a==b);

		Usuario(id_usuario,a,b);

		--n_viajes[id_usuario];
		if(n_viajes[id_usuario] == 0) para =1;
	}

	free(arg);
	pthread_exit(0);
}


int main(int argc, char *argv[]) {
	srand(time(NULL));
	int i;

	if(argc < 4){
		printf("Usage: ./simulator capacidad numUsuarios numParadas viajesUsuario0 ... viajesUsuarioN\n");
		return -1;
	}

	capacidad = atoi(argv[1]); //capacidad del autobús
	n_usuarios = atoi(argv[2]); // número de usuarios
	n_paradas = atoi(argv[3]); //número de paradas

	if(argc != 4 + n_usuarios){
		printf("Usage: ./ simulator capacidad numUsuarios numParadas viajesUsuario0 ... viajesUsuarioN\n");
		return -1;
	}

	n_viajes = malloc(n_usuarios*sizeof(int)); //número de viajes por usuario
	for(int i = 0; i< n_usuarios; ++i){
		n_viajes[i] = atoi(argv[4+i]);
	}

	esperando_parada = calloc(n_paradas,sizeof(int));
	esperando_bajar = calloc(n_paradas, sizeof(int));

	pthread_mutex_init(&mutAutobus,NULL);

	paraSubir = malloc(n_paradas*sizeof(pthread_cond_t));
	paraBajar = malloc(n_paradas*sizeof(pthread_cond_t));
	for(int i = 0; i< n_paradas; ++i){
		pthread_cond_init(&paraSubir[i], NULL);
		pthread_cond_init(&paraBajar[i], NULL);
	}

	pthread_cond_init(&esperaBus,NULL);

	// Crear el thread Autobus
	pthread_t thBus;
	pthread_create(&thBus, NULL, &thread_autobus, NULL);

	pthread_t thUsuario[n_usuarios];
	for (i = 0; i < n_usuarios; ++i){
		int * arg = malloc(sizeof(int));
		*arg = i;
		pthread_create(&thUsuario[i], NULL, &thread_usuario, arg);
	}

	pthread_join(thBus, NULL);

	for(i = 0; i < n_usuarios; ++i){
		pthread_join(thUsuario[i], NULL);
	}
	return 0;
}


