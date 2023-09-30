#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <protocolo.h>
#include <socket.h>
#include <shared.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <protocolo.h>
#include <stdlib.h>

#include <unistd.h>


typedef struct{
	int base;
	int pid;  //Pedirle el pid a Kernel
	bool modificado;
	bool presente;
	int numero_pagina;
	int pos_en_swap;
}Marco;
t_list* lst_marco;


typedef struct{
	int tamamo_usado;
	bool en_memoria;
	int ultimo_uso;//Para el algoritmo LRU
	Marco* puntero_marco;

}Pagina;
typedef struct{
	Pagina* page;
}tabla_paginas;


t_list* lst_pagina;


typedef enum{
	FIFO,
	LRU
}t_algoritmo;


/*
 *
 * MEMORIA VIRTUAL
 */

FILE* disco;

//config
char* IP_MEMORIA;
char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
char* ALGORITMO_REEMPLAZO;
//¿no debería ser un string para leerlo en el config?

t_log* memoria_logger;
t_log* memoria_log_obligatorio;
t_config* memoria_config;
char* server_name;
int socket_server;
int fd_kernel;
int fd_cpu;
int fd_filesystem;
int server_fd_memoria;
void* espacio_usuario;

t_dictionary* tablas;



/*----------------TODO INIT ------------------------*/
void leer_config();
void finalizar_memoria();



/*----------------COMUNICACION SOCKETS --------*/

void saludar_cliente(void *void_args);
void identificar_modulo(t_buffer* unBuffer, int conexion);
void atender_mensajes_kernel(t_buffer* buffer);

//static void  procesar_conexion(void *void_args);
void inicializar_memoria();
void iterator(int *value);
void leer_log();
int server_escucha();


/************TODO INICIAR LA TABLA DE PAGINAS***************/
tabla_paginas* crear_tabla_paginas(int pid);
Marco* crear_marco(int base, bool presente);



#endif /* MEMORIA_H_ */
