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

t_log* kernel_logger;
t_log* kernel_log_obligatorio;
t_config* kernel_config;

int process_id = 1;

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

// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_list_new;
pthread_mutex_t mutex_list_ready;

void leer_config(t_config* config);
void finalizar_kernel();
void asignar_planificador_cp(char* algoritmo_planificacion);

void leer_consola(void);
void atender_esta_prueba(t_buffer* unBuffer);
void atender_experimentos_xd(void);

void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();

// ------ Inicializar variables ------
void iniciar_semaforos();
void iniciar_pthread();
void iniciar_listas();

// ------ Proceso ------
void inicializar_estructura(int fd_memoria, char* path, int size, t_pcb* pcb);


// ------ PCB ------
t_pcb* iniciar_pcb(int prioridad);
void agregar_pcb_lista(t_pcb* pcb, t_list* list_estados, pthread_mutex_t mutex_list_new);
void transferir_from_new_to_ready();



#endif /* KERNEL_H_ */
