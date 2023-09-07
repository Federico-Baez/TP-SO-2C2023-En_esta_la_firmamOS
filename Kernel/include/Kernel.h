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
#include <shared.h>
#include <pthread.h>
#include <protocolo.h>
#include <socket.h>
#include <stdlib.h>


typedef enum{
	FIFO,
	ROUNDROBIN,
	PRIORIDADES
}t_algoritmo;



int fd_filesystem;
int fd_cpu_dispatcher;
int fd_cpu_interrupt;
int fd_memoria;

char* IP_MEMORIA;
int PUERTO_MEMORIA;
char* IP_FILESYSTEM;
int PUERTO_FILESYSTEM;
char* IP_CPU;
int PUERTO_CPU_DISPATCH;
int PUERTO_CPU_INTERRUPT;
t_algoritmo ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
int* INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION_INI;

t_log* kernel_logger;
t_log* kernel_log_obligatorio;
t_config* kernel_config;



#endif /* KERNEL_H_ */
