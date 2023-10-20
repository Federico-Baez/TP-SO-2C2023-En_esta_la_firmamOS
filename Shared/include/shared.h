/*
 * Shared.h
 *
 *  Created on: Sep 4, 2023
 *      Author: utnso
 */

#ifndef INCLUDE_SHARED_H_
#define INCLUDE_SHARED_H_


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "protocolo.h"
#include "socket.h"



typedef struct{
	char* nombre_recurso;
	int instancias;
	t_list* procesos_que_solicitan;
	t_list* procesos_que_retienen;
}t_recurso;

typedef struct{
	uint32_t AX;
	uint32_t BX;
	uint32_t CD;
	uint32_t DX;
}t_instruccion;


//Estados del procesos en la planificacion
typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	EXIT
}est_pcb;

// Motivos de vuelta al Kernel
typedef enum{
	FINALIZACION,
	PETICION,
	BLOCK,
	INTERRUPCION
}t_vuelta;

//Manejo de instrucciones del Kernel
typedef enum{
	SET,
	ADD,
	SUB,
	SUM,
	MOV_IN,
	MOV_OUT,
	SLEEP,
	JNZ,
	WAIT,
	SIGNAL,
	F_OPEN,
	F_CLOSE,
	F_SEEK,
	F_TRUNCATE,
	F_READ,
	F_WRITE,
	EXIT_P
}cod_instruccion;


//Contexto de ejecucion
typedef struct{
	int pid;
	int program_counter;
	int prioridad;
	t_instruccion* registros;
	est_pcb estado;
	t_vuelta* motivo_vuelta;
	//  <--- archivos_abiertos;
	//  <--- abria que agregar las intruccioens enviadas por memoria?
}t_pcb;

typedef struct{
	char* pseudo_c;
    char* fst_param;
    char* snd_param;
}t_instruccion_codigo;

typedef struct{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
	int program_counter;
}t_registros_CPU;

t_list* lista_instrucciones(t_log* logger, char* dir);
cod_instruccion convertir_string_a_instruccion(t_log* logger, const char *str_instruccion);
void liberar_lista_instrucciones(t_list *lista);

// ------ PCB ------
t_pcb* crear_pcb(int pid, int prioridad);
void cambiar_estado_pcb(t_pcb* pcb, est_pcb nuevo_estado);
void pcb_destroy(t_pcb* pcb);


#endif /* INCLUDE_SHARED_H_ */

