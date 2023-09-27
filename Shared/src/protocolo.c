#include "../include/protocolo.h"

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(t_log* logger, int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

int* recibir_int(t_log* logger, void* coso)
{
	int* buffer_int = malloc(sizeof(int));
	memcpy(buffer_int,coso,sizeof(int));
	log_info(logger, "Me llego el numero: %d", *buffer_int);
	return buffer_int;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);

	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}


t_list* recibir_paquete_int(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);

	//int* del_punter_coso;

	memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	int* valor = malloc(tamanio);
	memcpy(valor, buffer+desplazamiento, tamanio);
	desplazamiento+=tamanio;
	list_add(valores, valor);

	free(buffer);
	return valores;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_paquete* crear_super_paquete(op_code code_op){
	t_paquete* super_paquete = malloc(sizeof(t_paquete));
	super_paquete->codigo_operacion = code_op;
	crear_buffer(super_paquete);
	return  super_paquete;
}

void cargar_int_al_super_paquete(t_paquete* paquete, int numero){
	paquete->buffer->stream = realloc(paquete->buffer->stream,
										paquete->buffer->size + sizeof(int));
	//t_primitivo coso_int = _INT;
	//memcpy(paquete->buffer->stream + paquete->buffer->size, &coso_int, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));

	paquete->buffer->size += sizeof(int);
}

void cargar_string_al_super_paquete(t_paquete* paquete, char* string){
	int size_string = strlen(string)+1;
	paquete->buffer->stream = realloc(paquete->buffer->stream,
									paquete->buffer->size + sizeof(int) + sizeof(char)*size_string);
	/*Falta completar esta parte*/
	memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*size_string);

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char)*size_string;
}

int recibir_int_del_buffer(t_buffer* coso){
	int valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int));

	int nuevo_size = coso->size - sizeof(int);
	if(nuevo_size < 0){
		printf("\n[ERROR]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		//free(valor_a_devolver);
		return 0;
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int), nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}

char* recibir_string_del_buffer(t_buffer* coso){
	int size_string;
	char* string;
	memcpy(&size_string, coso->stream, sizeof(int));
	string = malloc(sizeof(size_string));
	memcpy(string, coso->stream + sizeof(int), size_string);

	int nuevo_size = coso->size - sizeof(int) - size_string;
	if(nuevo_size < 0){
		printf("\n[ERROR]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		free(string);
		return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int) + size_string, nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return string;
}
