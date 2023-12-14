#ifndef M_GESTOR_H_
#define M_GESTOR_H_

#include <protocolo.h>
#include <socket.h>
#include <shared.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <commons/string.h>

#include <pthread.h>

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <math.h>


const uint32_t EOF_FS = UINT32_MAX;


typedef struct{
	char* nombre;
	t_config* archivo_fcb;
}t_archivo_fcb;

typedef struct{
	char* nombre;
	int tamanio;
	int bloque_inicial;
}t_fcb;

typedef struct{
	int id_bloque;
	int esta_libre;
	uint32_t puntero_siguiente;
	int eof; //es necesario esto?
}t_bloque_fat;

/*typedef struct{
	int id_bloque;
	int pid;
	int esta_libre;
	int nro_pagina;
}t_bloque;*/

typedef struct{
	int id_bloque;
	int esta_libre;
	void* contenido; //pid con pagina o contenido de archivo
}t_bloque;

extern t_list* tabla_fat;
extern t_list* lista_bloques;

extern int tamanio_particion_swap;
extern int tamanio_particion_bloques;
extern int tamanio_fat;
extern t_list* lista_struct_fcbs;
extern t_list* lista_configs_fcbs;

extern void* buffer_swap;
extern uint32_t* buffer_bloques;
extern uint32_t* buffer_tabla_fat;

//=============================================

extern t_log* filesystem_logger;
extern t_log* filesystem_log_obligatorio;
extern t_config* filesystem_config;

extern int server_fd_filesystem;
extern int fd_memoria;
extern int fd_kernel;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA;
extern char* PATH_FAT;
extern char* PATH_BLOQUES;
extern char* PATH_FCB;
extern int CANT_BLOQUES_TOTAL;
extern int CANT_BLOQUES_SWAP;
extern int TAM_BLOQUE;
extern int RETARDO_ACCESO_BLOQUE;
extern int RETARDO_ACCESO_FAT;







#endif /* M_GESTOR_H_ */
