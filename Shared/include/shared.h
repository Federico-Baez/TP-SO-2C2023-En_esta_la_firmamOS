/*
 * Shared.h
 *
 *  Created on: Sep 4, 2023
 *      Author: utnso
 */

#ifndef INCLUDE_SHARED_H_
#define INCLUDE_SHARED_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

#include "socket.h"



typedef enum{
	CUALQUIERA
}t_instruccion;


//Estados del procesos en la planificacion
typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	EXIT
}est_proceso;



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
	t_instruccion*registros;
	//  <--- archivos_abiertos;
}t_contexto_ejecucion;



#endif /* INCLUDE_SHARED_H_ */
