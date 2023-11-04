#ifndef KERNEL_H_
#define KERNEL_H_

#include "inicializar_kernel.h"
#include "finalizar_kernel.h"
#include "atender_conexiones.h"
#include "consola.h"
#include "k_gestor.h"

t_log* kernel_logger;
t_log* kernel_log_obligatorio;
t_config* kernel_config;

pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;
pthread_t hilo_consola;
pthread_t hilo_filesystem;

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

// ------ Listas ------
t_list* lista_new;
t_list* lista_ready;
t_list* lista_execute;
t_list* lista_blocked;
t_list* lista_exit;

t_list* lista_instructions;
t_list* lista_general;
t_list* lista_recursos;

int process_id = 0;
int procesos_en_core = 0;
int var_pausa = 0;
int var_ticket = 0;

//Para controlar la habilitacion de interrupciones - Algo. Prioridad
bool interrupcion_habilitada = false;

//Estado de CPU
bool CPU_en_uso = false;

//Para dar prioridad a la interrupcion por consola sobre la de quantum
bool batisenal_exit = false;

//--Van juntos para controlar PIORIDADES
bool hay_pcb_elegida = false;
t_pcb* pcb_prioritaria;

// ------ SEMAFOROS ------
sem_t sem_pausa;


// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_lista_general;

pthread_mutex_t mutex_process_id;
pthread_mutex_t mutex_core;
pthread_mutex_t mutex_pausa;
pthread_mutex_t mutex_recurso;
pthread_mutex_t mutex_ticket;
pthread_mutex_t mutex_interrupcion_habilitada;


#endif /* KERNEL_H_ */
