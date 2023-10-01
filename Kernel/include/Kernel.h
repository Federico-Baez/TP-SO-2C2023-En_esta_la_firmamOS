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


typedef enum{
	FIFO,
	ROUNDROBIN,
	PRIORIDADES
}t_algoritmo;

t_log* kernel_logger;
t_log* kernel_log_obligatorio;
t_config* kernel_config;

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

void leer_config(t_config* config);
void finalizar_kernel();
void asignar_planificador_cp(char* algoritmo_planificacion);

void leer_consola(void);
void atender_esta_prueba(t_buffer* unBuffer);


void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();



#endif /* KERNEL_H_ */
