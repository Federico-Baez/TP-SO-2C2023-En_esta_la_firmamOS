#ifndef K_GESTOR_H_
#define K_GESTOR_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>

#include <protocolo.h>
#include <shared.h>
#include <socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef enum{
	FIFO,
	ROUNDROBIN,
	PRIORIDADES
}t_algoritmo;

typedef enum{
	NEW,//=0
	READY,//=1
	EXEC,//=2
	BLOCKED,//=3
	EXIT//=4
}est_pcb;

typedef enum{
	SUCCESS,  // 0
	INVALID_RESOURCE,  // 1
	INVALID_WRITE // 2
}t_motivo_exit;

typedef enum{
	RECURSO,
	ARCHIVO,
	OTRO
}t_motivo_block;


typedef struct{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
}t_registros_CPU;

// Hay que agregar dos enum, uno que sea motivo_bloqueo y otro motivo_finalizacion.
typedef struct{
	int pid;
	int ticket;
	int program_counter;
	int prioridad;
	int size;
	char* path;
	est_pcb estado;
	t_motivo_exit motivo_exit;
	t_motivo_block motivo_block;   // Para DEADLOCKS
	t_registros_CPU* registros_CPU;
	t_list* lista_recursos_pcb;
	pthread_mutex_t mutex_lista_recursos;  // --> No se si es necesaria
	t_list* archivos_abiertos;  // Aca guardo el struct t_archivo_ab, que va a contener un puntero al archivo y su modo de apertura
}t_pcb;

typedef struct{
	char* recurso_name;
	int instancias;  // Instancias del recurso
	t_list* lista_bloqueados;   // Procesos a la espera de una instancia
	t_pcb* pcb_asignado;	// Procesos con isntancias asignadas
	pthread_mutex_t mutex_bloqueados;
}t_recurso;

typedef struct{
	t_pcb* pcb_a_sleep;
	int tiempo_en_block;
}t_sleep;

typedef struct{
	int pid_process;
	int numero_pagina;
}t_page_fault;

typedef struct{
	int locked; // Si es " = 1 ", quiere decir que existe lock de lectura
 	t_list* lista_participantes; // Todos aquellos que posean un lock de lectura
	pthread_mutex_t mutex_lista_asiganada;  // El mutex para la lista de participantes
}t_lock_lectura;

typedef struct{
	int locked; // Si es " = 1 ", quiere decir que existe lock de escritura
	t_pcb* pcb; // El pcb que posee el lock
}t_lock_escritura;

typedef struct{
	char* nombre_archivo;
	t_lock_escritura* lock_escritura; // 'w' para escritura
	t_lock_lectura* lock_lectura;  // 'r' para lectura
	t_list* cola_block_procesos; // Procesos en espera para acceder al archivo
	pthread_mutex_t mutex_cola_block;
	int size; // tamaño del archivo
}t_archivo;

typedef struct{
//	t_archivo* archivo_abierto;
	char* nombre_archivo; // Creo que voy a usar esto para que no haya tantos punteros
	char* modo_apertura;  // Puede ser 'r' para lectura, o 'w' para escritura
	int lock_otorgado; // 1 en caso de true, 0 en caso de false.
	int puntero;  // Puntero al byte del archivo que vamos a acceder
}t_archivo_abierto_pcb;


extern t_log* kernel_logger;
extern t_log* kernel_log_obligatorio;
extern t_config* kernel_config;

extern pthread_t hilo_cpu_dispatch;
extern pthread_t hilo_cpu_interrupt;
extern pthread_t hilo_memoria;
extern pthread_t hilo_consola;
extern pthread_t hilo_filesystem;

extern int fd_filesystem;
extern int fd_cpu_dispatcher;
extern int fd_cpu_interrupt;
extern int fd_memoria;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_FILESYSTEM;
extern char* PUERTO_FILESYSTEM;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern t_algoritmo ALGORITMO_PLANIFICACION;
extern int QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION_INI;

extern int process_id;
extern int procesos_en_core;
extern int var_pausa;
extern int var_ticket;


// ------ Listas ------
extern t_list* lista_new;
extern t_list* lista_ready;
extern t_list* lista_execute;
extern t_list* lista_blocked;
extern t_list* lista_exit;

extern t_list* cola_blocked_fs;

extern t_list* lista_instructions;
extern t_list* lista_general;
extern t_list* lista_recursos;
extern t_list* lista_archivos_abiertos;


extern bool flag_exit;

//Para controlar la habilitacion de interrupciones - Algo. Prioridad
extern bool interrupcion_habilitada;

//Estado de CPU
extern bool CPU_en_uso;

//Para dar prioridad a la interrupcion por consola sobre la de quantum y en el hilo de interrupt si se finaliza proceso por DESALOJO PRO CONSOLA
extern bool flag_finalizar_proceso;

// Para determinar si se recibio un proceso desalojado en cpu_dispatch
extern bool flag_proceso_desalojado;

//--Van juntos para controlar PIORIDADES
extern bool hay_pcb_elegida;
extern t_pcb* pcb_prioritaria;

// Para controlar si existe o no un archivo
extern int flag_existe_archivo;

// ------ SEMAFOROS ------
extern sem_t sem_pausa;
extern sem_t sem_enviar_interrupcion;
extern sem_t sem_estructura_iniciada;
extern sem_t sem_estructura_liberada;

extern sem_t sem_f_open_FS;
extern sem_t sem_nuevo_en_block;

// ------ PTHREAD_MUTEX ------
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_lista_exec;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_lista_exit;
extern pthread_mutex_t mutex_lista_general;

extern pthread_mutex_t mutex_cola_blocked_fs;

extern pthread_mutex_t mutex_process_id;
extern pthread_mutex_t mutex_core;
extern pthread_mutex_t mutex_pausa;
extern pthread_mutex_t mutex_recurso;
extern pthread_mutex_t mutex_ticket;
extern pthread_mutex_t mutex_enviar_interrupcion;

extern pthread_mutex_t mutex_flag_exit;

extern pthread_mutex_t mutex_flag_proceso_desalojado;

// Este es para chequear en INTERRUPT que sea por DESALOJO_POR_CONSOLA
extern pthread_mutex_t mutex_flag_finalizar_proceso;
// Para manejar los PAGE FAULT de 1 en 1
extern pthread_mutex_t mutex_manejo_page_fault;

extern pthread_mutex_t mutex_existe_archivo;

extern pthread_mutex_t mutex_peticion_fs;


void public_imprimir_procesos_por_estado_v0();
void public_imprimir_procesos_por_estado_v1();

#endif /* K_GESTOR_H_ */
