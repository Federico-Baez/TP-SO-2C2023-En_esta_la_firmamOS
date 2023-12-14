#ifndef M_GESTOR_H_
#define M_GESTOR_H_

#include <protocolo.h>
#include <socket.h>
#include <shared.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/string.h>

#include <pthread.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <math.h>

extern const uint32_t EOF_FS;

typedef struct{
	char* nombre;
	t_config* archivo_fcb;
}t_archivo_fcb;

typedef struct{
	char* nombre;
	int tamanio;
	int bloque_inicial;
}t_fcb;

extern int tamanio_archivo_bloques;
extern int tamanio_fat;
extern t_list* lista_struct_fcbs;
extern t_list* lista_configs_fcbs;

extern int fd_archivoTablaFAT;
extern int fd_archivoBloques;

extern void* bitmap_swap;
extern t_bitarray* bitmapSWAP;

extern uint32_t* tablaFatEnMemoria;
extern void* archivoBloquesEnMemoria;
extern void* bloquesFATEnMemoria;
extern void* bloquesSwapEnMemoria;

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
