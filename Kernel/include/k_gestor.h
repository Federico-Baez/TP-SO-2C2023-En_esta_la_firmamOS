#ifndef K_GESTOR_H_
#define K_GESTOR_H_

#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <pthread.h>

#include <protocolo.h>
#include <shared.h>
#include <socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


typedef struct{
	int PID;
	int prioridad;
	void* tabla_de_archivos_abiertos;
	char* motivo_desalojo;
	t_registros_CPU* registros_CPU;
}t_PCB;








#endif /* K_GESTOR_H_ */
