#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "m_gestor.h"
#include "fs_memoria.h"
#include "fs_kernel.h"

#define IP_FILESYSTEM "127.0.0.1"

const uint32_t EOF_FS = UINT32_MAX;

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

int tamanio_archivo_bloques;
int tamanio_fat;
t_list* lista_struct_fcbs;
t_list* lista_configs_fcbs;

int fd_archivoTablaFAT;
int fd_archivoBloques;

void* bitmap_swap;
t_bitarray* bitmapSWAP;

uint32_t* tablaFatEnMemoria;
void* archivoBloquesEnMemoria;
void* bloquesFATEnMemoria;
void* bloquesSwapEnMemoria;

void leer_config(t_config* config);
void iterator(char* value);

void inicializar_archivos();
void crear_fat();
void inicializar_archivo_de_bloques();
void mapear_bloques_swap(int fd);
void mapear_bloques_de_archivo(int fd);
void destruir_fcb(t_fcb* fcb);
void destruir_archivo_fcb(t_archivo_fcb* archivo_fcb);
void destruir_listas_fcbs();
void finalizar_filesystem();

void atender_filesystem_kernel(void);
void atender_memoria(void);
void atender_mensajes_kernel(t_buffer* buffer);

#endif /* FILESYSTEM_H_ */
