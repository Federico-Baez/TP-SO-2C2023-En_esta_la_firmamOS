/*
 * socket.h
 *
 *  Created on: Sep 4, 2023
 *      Author: utnso
 */

#ifndef INCLUDE_SOCKET_H_
#define INCLUDE_SOCKET_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include <commons/collections/queue.h>
#define BACKLOG 100

int iniciar_servidor(t_log* logger, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* name, int socket_servidor);
int crear_conexion(char *ip, char* puerto);
void liberar_conexion(int socket_cliente);

#endif /* INCLUDE_SOCKET_H_ */
