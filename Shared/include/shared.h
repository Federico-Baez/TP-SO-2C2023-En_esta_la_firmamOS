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
#include <commons/temporal.h>
#include <pthread.h>
#include "protocolo.h"
#include "socket.h"

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


typedef struct{
	char* pseudo_c;
    char* fst_param;
    char* snd_param;
}t_instruccion_codigo;

t_list* lista_instrucciones(t_log* logger, char* dir);
cod_instruccion convertir_string_a_instruccion(t_log* logger, const char *str_instruccion);
void liberar_lista_instrucciones(t_list *lista);
void ejecutar_en_un_hilo_nuevo_detach(void (*f)(void*) ,void* struct_arg);
void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg);

#endif /* INCLUDE_SHARED_H_ */

