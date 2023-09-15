#ifndef INCLUDE_PROTOCOLO_H_
#define INCLUDE_PROTOCOLO_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include"shared.h"

typedef enum{
	MENSAJE,
	PAQUETE,
	SUPER_PAQUETE,

}op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

/******************TODO: revisar los MENSAJES*************/
void enviar_mensaje(char* mensaje, int socket_cliente);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* logger, int socket_cliente);
void crear_buffer(t_paquete* paquete);

/******************TODO: revisar los PAQUETES*************/
t_list* recibir_paquete(int);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int bytes);
void eliminar_paquete(t_paquete* paquete);

/******************TODO: revisar los EMPAQUETACION*************/


/******************TODO: revisar los SENDS*************/


/******************TODO: revisar los RECVS*************/


/******************TODO: revisar los DESTROY*************/

#endif /* INCLUDE_PROTOCOLO_H_ */
