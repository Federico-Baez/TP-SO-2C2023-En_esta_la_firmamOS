/*
 * Kernel.h
 *
 *  Created on: Sep 7, 2023
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <shared.h>
#include <pthread.h>
#include <protocolo.h>
#include <socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <semaphore.h>

typedef enum{
	FIFO,
	ROUNDROBIN,
	PRIORIDADES
}t_algoritmo;

// ------ Listas ------
t_list* list_new;
t_list* list_ready;
t_list* list_execute;
t_list* list_blocked;
t_list* list_recursos;

t_log* kernel_logger;
t_log* kernel_log_obligatorio;
t_config* kernel_config;

int process_id = 1;

// ------ Direcciones Sockets ------
int fd_filesystem;
int fd_cpu_dispatcher;
int fd_cpu_interrupt;
int fd_memoria;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
t_algoritmo ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION_INI;

// ------ SEMAFOROS ------
sem_t sem_init_pcb;
sem_t sem_grado_multiprogramacion;
sem_t sem_list_ready;
sem_t sem_iniciar_estructuras_memoria;

// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_list_new;
pthread_mutex_t mutex_list_ready;
pthread_mutex_t mutex_list_exec;
pthread_mutex_t mutex_list_blocked;

void leer_config(t_config* config);
void finalizar_kernel();
void asignar_planificador_cp(char* algoritmo_planificacion);

void atender_esta_prueba(t_buffer* unBuffer);
void atender_experimentos_xd(void);

void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();

void leer_consola();

// ------ Inicializar variables ------
void iniciar_semaforos();
void iniciar_pthread();
void iniciar_listas();

// ------ Proceso ------
void inicializar_ejecucion();
void inicializar_estructura(int fd_memoria, char* path, int size, t_pcb* pcb);
void ejecutar_proceso();
t_pcb* elegir_proceso_segun_algoritmo();
t_pcb* remover_proceso_lista(t_list* list_estado, pthread_mutex_t mutex);
t_pcb* obtener_proceso_segun_prioridad();
bool maxima_prioridad(t_pcb* pcb1, t_pcb* pcb2);
void ejecutar(t_pcb* pcb);
char* lista_pids_en_Ready();
void atender_motivo_block(t_pcb* pcb);


// ------ Cambios de ESTADO ------
void transferir_from_new_to_ready();
void transferir_from_ready_to_exec();
void transferir_from_actual_to_siguiente(t_list* list_actual, pthread_mutex_t mutex_actual, t_list* list_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente);

// ------ PCB ------
t_pcb* iniciar_pcb(int prioridad);
void agregar_pcb_lista(t_pcb* pcb, t_list* list_estados, pthread_mutex_t mutex_list);
char* estado_to_string(est_pcb estado);
t_pcb* recv_pcb(t_buffer* paquete_pcb);

// ------ RECURSOS ------
void iniciar_recursos();

#endif /* KERNEL_H_ */

