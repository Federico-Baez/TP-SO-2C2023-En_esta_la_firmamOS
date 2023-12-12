/*
 * m_gestor.h
 *
 *  Created on: Oct 28, 2023
 *      Author: utnso
 */

#ifndef M_GESTOR_H_
#define M_GESTOR_H_
#include <protocolo.h>
#include <socket.h>
#include <shared.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <protocolo.h>
#include <stdlib.h>
#include <unistd.h>

#include <math.h>

typedef enum {
	MARCO_LIMPIO,
	MARCO_VICTIMA
}tipo_marco;

typedef struct{
	int pid;
	int size;
	char* pathInstrucciones;
	t_list* instrucciones;
	t_list* tabla_paginas;
	pthread_mutex_t mutex_TP;
}t_proceso;

typedef struct {
	t_proceso* proceso;
	int nro_pagina;
}frame_info;

typedef struct {
    int nro_marco;
    int base;
    bool libre;
    frame_info* info_new;
    frame_info* info_old;

    int orden_carga;
    t_temporal* ultimo_uso;
} t_marco;

typedef struct {
	int nro_pagina; //Set al inicio
	int nro_marco;
	bool presente;	//Set al inicio
	bool modificado;//Set al inicio
	int pos_en_swap;
} t_pagina;

extern pthread_mutex_t mutex_lst_marco;
extern pthread_mutex_t mutex_espacio_usuario;
extern pthread_mutex_t mutex_ord_carga_global;
extern int ordenCargaGlobal;
//-------------------



extern t_list* list_procss_recibidos;
extern t_list* list_instruciones;


typedef enum{
	FIFO,
	LRU
}t_algoritmo;
extern t_list* list_pagina;
extern t_list* lst_marco;

extern FILE* disco;

//config
extern char* IP_MEMORIA;
extern char* PUERTO_ESCUCHA;
extern char* IP_FILESYSTEM;
extern char* PUERTO_FILESYSTEM;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;
extern char* ALGORITMO_REEMPLAZO;


extern t_log* memoria_logger;
extern t_log* memoria_log_obligatorio;
extern t_config* memoria_config;
extern char* server_name;
extern int socket_server;
extern int fd_kernel;
extern int fd_cpu;
extern int fd_filesystem;
extern int server_fd_memoria;
extern void* espacio_usuario;


extern t_dictionary* tablas;
//extern t_list* instrucciones_para_cpu;



#endif /* M_GESTOR_H_ */
