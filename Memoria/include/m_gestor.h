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
typedef struct {
    int pid;                        // Identificador del proceso asociado a esta tabla
    t_list* paginas;                // Lista de páginas
    pthread_mutex_t mutex;          // Mutex para sincronización
} tabla_paginas;

typedef struct {
    int base;
    bool libre;
    int pid;
    int nro_pagina;
    pthread_mutex_t mutex;
} marco;


typedef struct {
    bool presente;         // la página está en memoria o en disco
    bool modificado;
    int tamanio_ocupado;
    marco* ptr_marco;// Bit de modificación
    int marco;             // Si está presente, este es el número de marco en memoria
    int pos_en_swap;       // No está presente, esta es la posición en el espacio de intercambio (swap)
    t_temporal* ultimo_uso;        // LRU
	int orden_carga; // Para FIFO
    pthread_mutex_t mutex;
} Pagina;

typedef struct{
	int pid;
	int size;
	char* pathInstrucciones;
	t_list* instrucciones;
}proceso_recibido;

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
extern t_list* instrucciones_para_cpu;

void* buscar_tabla(int pid);

/******************MARCO********************/
marco* crear_marco(int base, bool presente);
Pagina* obtener_pagina_por_marco(marco* un_marco);
Pagina* obtener_pagina_por_marco(marco* un_marco);
/******************************************/
void liberar_paginas(tabla_paginas* una_tabla, int  dirLogica, int tamanio, int pid);
void* buscar_tabla(int pid);

#endif /* M_GESTOR_H_ */
