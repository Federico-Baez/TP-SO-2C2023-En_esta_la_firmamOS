#include "../include/socket.h"

int iniciar_servidor(t_log* logger, char* ip, char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	listen(socket_servidor, SOMAXCONN);

	log_info(logger, "Listo para escuchar a mi cliente");

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(t_log* logger, const char* name, int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

    log_info(logger, "Cliente conectado a %s", name);

    return socket_cliente;
}
