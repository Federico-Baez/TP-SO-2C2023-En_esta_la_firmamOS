#ifndef CPU_H_
#define CPU_H_

#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <shared.h>
#include <pthread.h>
#include <protocolo.h>
#include <socket.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>

#define IP_FILESYSTEM "127.0.0.1"

t_log* filesystem_logger;
t_log* filesystem_log_obligatorio;
t_config* filesystem_config;

int server_fd_filesystem;
int fd_memoria;
int fd_kernel;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA;
char* PATH_FAT;
char* PATH_BLOQUES;
char* PATH_FCB;
int CANT_BLOQUES_TOTAL;
int CANT_BLOQUES_SWAP;
int TAM_BLOQUE;
int RETARDO_ACCESO_BLOQUE;
int RETARDO_ACCESO_FAT;

typedef struct{
	char* nombre;
	t_config* archivo_fcb;
}t_archivo;

typedef struct{
	char* nombre;
	int tamano;
	int bloque_inicial;
}t_fcb;

typedef struct{
    char* direccion;
    uint32_t tamanio;
}t_fat;

int tamanio_particion_swap;
int tamanio_particion_bloques;
int tamanio_fat;
t_list* lista_fcbs;
void* buffer_swap;
void* buffer_bloques;
//void* buffer_fat;

void leer_config(t_config* config);
void iterator(char* value);

void atender_filesystem_kernel(void);
void atender_memoria(void);
void atender_mensajes_kernel(t_buffer* buffer);


#endif /* CPU_H_ */
