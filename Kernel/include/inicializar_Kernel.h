#ifndef INICIALIZAR_KERNEL_H_
#define INICIALIZAR_KERNEL_H_

#include "finalizar_kernel.h"
#include "atender_conexiones.h"
#include "consola.h"
#include "inicializar_estructuras.h"
#include "k_gestor.h"
#include "manejo_deadblocks.h"


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

t_list* cola_blocked_fs;  // Esta el la cola de block que tienen los archivos, aca van a estar todos los procesos blockeados a la espera de un lock

t_list* lista_instructions;
t_list* lista_general;
t_list* lista_recursos;
t_list* lista_archivos_abiertos;

int process_id = 0;
int procesos_en_core = 0;
int var_pausa = 0;
int var_ticket = 0;

bool flag_exit = false;
//Para controlar la habilitacion de interrupciones - Algo. Prioridad
bool interrupcion_habilitada = false;

//Estado de CPU
bool CPU_en_uso = false;

//Para dar prioridad a la interrupcion por consola sobre la de quantum
bool flag_finalizar_proceso = false;

// Para determinar si se recibio un proceso desalojado en cpu_dispatch
bool flag_proceso_desalojado = false;

//--Van juntos para controlar PIORIDADES
bool hay_pcb_elegida = false;
t_pcb* pcb_prioritaria;

// Para controlar si existe o no un archivo
int flag_existe_archivo = 0;

// ------ SEMAFOROS ------
sem_t sem_pausa;
sem_t sem_enviar_interrupcion;
sem_t sem_estructura_iniciada;
sem_t sem_estructura_liberada;

sem_t sem_f_open_FS;
sem_t  sem_nuevo_en_block;

// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_lista_general;

pthread_mutex_t mutex_cola_blocked_fs;

pthread_mutex_t mutex_process_id;
pthread_mutex_t mutex_core;
pthread_mutex_t mutex_pausa;
pthread_mutex_t mutex_recurso;
pthread_mutex_t mutex_ticket;
pthread_mutex_t mutex_enviar_interrupcion;

pthread_mutex_t mutex_flag_exit;

pthread_mutex_t mutex_flag_proceso_desalojado;
// Este es para chequear en INTERRUPT que sea por DESALOJO_POR_CONSOLA
 pthread_mutex_t mutex_flag_finalizar_proceso;
// Para manejar los PAGE FAULT de 1 en 1
 pthread_mutex_t mutex_manejo_page_fault;

 pthread_mutex_t mutex_existe_archivo;

 pthread_mutex_t mutex_peticion_fs;

#endif /* INICIALIZAR_KERNEL_H_ */
