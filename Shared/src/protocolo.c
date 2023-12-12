#include "../include/protocolo.h"

void handhsake_modules(int conexion, char* mensaje){
	t_paquete* un_paquete = crear_super_paquete(ESTRUCTURA_INICIADA_MK);

	cargar_string_al_super_paquete(un_paquete, mensaje);
	enviar_paquete(un_paquete, conexion);
	eliminar_paquete(un_paquete);
}

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

int recibir_operacion(int socket_cliente) // @suppress("No return")
{
	int cod_op;
//	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
//		return cod_op;
//	else
//	{
//		printf("Error al recibir el codigo de operacion\n");
//		close(socket_cliente);
//		return -1;
//	}

	int bytesRecibidos = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if(bytesRecibidos > 0) {
	    if(bytesRecibidos != sizeof(int)) {
	        printf("Recibidos %d bytes, esperaba %zu bytes\n", bytesRecibidos, sizeof(int));
	    }
	    return cod_op;
	} else if (bytesRecibidos == 0) {
		// El cliente cerró la conexión
		printf("El cliente cerró la conexión.\n");
		close(socket_cliente);
		return -1;
	} else {
		// Ocurrió un error al recibir datos
		printf("Error al recibir el codigo de operacion\n");
		close(socket_cliente);
		return -2;
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
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int));
		memcpy(paquete->buffer->stream, &numero, sizeof(int));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
											paquete->buffer->size + sizeof(int));
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
	}

	paquete->buffer->size += sizeof(int);
}

void cargar_string_al_super_paquete(t_paquete* paquete, char* string){
	int size_string = strlen(string)+1;

	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char)*size_string);
		memcpy(paquete->buffer->stream, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char)*size_string);

	}else {
		paquete->buffer->stream = realloc(paquete->buffer->stream,
										paquete->buffer->size + sizeof(int) + sizeof(char)*size_string);
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*size_string);

	}
	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char)*size_string;
}

void cargar_choclo_al_super_paquete(t_paquete* paquete, void* choclo, int size){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + size);
		memcpy(paquete->buffer->stream, &size, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), choclo, size);
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
												paquete->buffer->size + sizeof(int) + size);

		memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), choclo, size);
	}

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += size;
}

int recibir_int_del_buffer(t_buffer* coso){
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int));

	int nuevo_size = coso->size - sizeof(int);
	if(nuevo_size == 0){
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_INT]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int), nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}

char* recibir_string_del_buffer(t_buffer* coso){
	//Fomrato de charGPT
// if(coso->size < sizeof(int)) {
//        perror("[ERROR]: Buffer demasiado pequeño para contener el tamaño de la cadena");
//        exit(EXIT_FAILURE);
//    }
//
//    int size_string;
//    char* string;
//    memcpy(&size_string, coso->stream, sizeof(int));
//
//    // Verifica que el tamaño de la cadena sea válido y que el buffer sea suficiente
//    if(size_string <= 0 || coso->size < sizeof(int) + size_string) {
//        perror("[ERROR]: Tamaño de cadena inválido o buffer demasiado pequeño para contener la cadena");
//        exit(EXIT_FAILURE);
//    }
//
//    string = malloc(size_string + 1);  // +1 para el '\0' al final
//    if (!string) {
//        perror("[ERROR]: No se pudo asignar memoria para la cadena");
//        exit(EXIT_FAILURE);
//    }
//
//    memcpy(string, coso->stream + sizeof(int), size_string);
//    string[size_string] = '\0';  // Asegurarte de que la cadena esté terminada en null
//
//    // ... el resto del código para manejar la reducción del tamaño del buffer ...
//
//    return string;

    //----------------- Formato Inicial----------------------------
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_string;
	char* string;
	memcpy(&size_string, coso->stream, sizeof(int));
	//string = malloc(sizeof(size_string));
	string = malloc(size_string);
	memcpy(string, coso->stream + sizeof(int), size_string);

	int nuevo_size = coso->size - sizeof(int) - size_string;
	if(nuevo_size == 0){
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return string;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_STRING]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		free(string);
		//return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int) + size_string, nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return string;
}

void* recibir_choclo_del_buffer(t_buffer* coso){
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_choclo;
	void* choclo;
	memcpy(&size_choclo, coso->stream, sizeof(int));
	choclo = malloc(size_choclo);
	memcpy(choclo, coso->stream + sizeof(int), size_choclo);

	int nuevo_size = coso->size - sizeof(int) - size_choclo;
	if(nuevo_size == 0){
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return choclo;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_CHICLO]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		//free(choclo);
		//return "";
		exit(EXIT_FAILURE);
	}
	void* nuevo_choclo = malloc(nuevo_size);
	memcpy(nuevo_choclo, coso->stream + sizeof(int) + size_choclo, nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_choclo;
	coso->size = nuevo_size;

	return choclo;
}

t_buffer* recibiendo_super_paquete(int conexion){
	t_buffer* unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream = recibir_buffer(&size, conexion);
	unBuffer->size = size;
	return unBuffer;
}


void enviar_handshake(int conexion){
	void* coso_a_enviar = malloc(sizeof(int));
	int saludar = HANDSHAKE;
	memcpy(coso_a_enviar, &saludar, sizeof(int));
	send(conexion, coso_a_enviar, sizeof(int),0);
	free(coso_a_enviar);
}

void gestionar_handshake_como_cliente(int conexion, char* modulo_destino, t_log* logger){
	enviar_handshake(conexion);
	int respuesta_handshake = recibir_operacion(conexion);
	if(respuesta_handshake != 1){
		log_error(logger, "Error en handshake con %s", modulo_destino);
		exit(EXIT_FAILURE);
	}
}

void gestionar_handshake_como_server(int conexion, t_log* logger){
	int code_op = recibir_operacion(conexion);
	printf("codigo de operacion: %d como handhaske servidor \n", code_op);
	switch (code_op) {
		case HANDSHAKE:
			void* coso_a_enviar = malloc(sizeof(int));
			int respuesta = 1;
			memcpy(coso_a_enviar, &respuesta, sizeof(int));
			send(conexion, coso_a_enviar, sizeof(int),0);
			free(coso_a_enviar);

			break;
		case -1:
			log_error(logger, "Desconexion en HANDSHAKE\n");
			exit(EXIT_FAILURE);
			break;
		default:
			log_error(logger, "ERROR EN HANDSHAKE: Operacion desconocida\n");
			exit(EXIT_FAILURE);
			break;
	}
}

//Usado por los clientes de memoria para que se identifiquen
void identificarme_con_memoria(int conexion, modulo_code modulo_cliente){
	t_paquete* paquete = crear_super_paquete(IDENTIFICACION);
	int modulo_id_ = modulo_cliente;
	cargar_int_al_super_paquete(paquete, modulo_id_);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

// Manejo del path_size MEMORIA
void send_path_memoria(int fd_modulo, char* path,int size){
	t_paquete* paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
	cargar_string_al_super_paquete(paquete, path);
	cargar_int_al_super_paquete(paquete, size);

	enviar_paquete(paquete,fd_modulo);
	eliminar_paquete(paquete);
}

void send_enviar_path_memoria(int fd_memoria, char* path, int size, int process_id){
	t_paquete* paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
	cargar_string_al_super_paquete(paquete, path);
	cargar_int_al_super_paquete(paquete, size);
	cargar_int_al_super_paquete(paquete, process_id);

	enviar_paquete(paquete, fd_memoria);
	eliminar_paquete(paquete);
}


void enviar_tamanio_fcb(int tamanio_fcb, int fd_modulo){
	t_paquete* paquete_con_tamanio = crear_super_paquete(PAQUETE);
	cargar_int_al_super_paquete(paquete_con_tamanio, tamanio_fcb);
	enviar_paquete(paquete_con_tamanio, fd_modulo);
	eliminar_paquete(paquete_con_tamanio);
}

void enviar_para_escribir_valor_leido(char* valor, int dir_fisica, int pid, int fd_modulo){
	t_paquete* paquete = crear_super_paquete(CARGAR_INFO_DE_LECTURA_FM); //TODO: Supongo que el op code es el de PEDIDO_ESCRITURA_FM
	cargar_string_al_super_paquete(paquete, valor);
	cargar_int_al_super_paquete(paquete, dir_fisica);
	cargar_int_al_super_paquete(paquete, pid);
	enviar_paquete(paquete, fd_modulo);
}

void recibir_fin_de_escritura(int fd_modulo){
	op_code cop = recibir_operacion(fd_modulo);
	if(cop != MENSAJE/*RPTA_CARGAR_INFO_DE_LECTURA_MF*/){ //TODO: El op code deberia ser algo así como FIN_ESCRITURA_MF
		return;
	}
	t_list* paquete = recibir_paquete(fd_modulo);
	op_code* cop_paquete = list_get(paquete, 0);
	free(cop_paquete);
	list_destroy(paquete);
}

void enviar_para_leer_valor(int dir_fisica, int pid, int fd_modulo){
	t_paquete* paquete = crear_super_paquete(GUARDAR_INFO_FM); //TODO: Supongo que el op code es el de PEDIDO_LECTURA_FM
	cargar_int_al_super_paquete(paquete, dir_fisica);
	cargar_int_al_super_paquete(paquete, pid);
	enviar_paquete(paquete, fd_modulo);
}

char* recibir_valor_leido(int fd_modulo){
	op_code cop = recibir_operacion(fd_modulo);

	if(cop != MENSAJE){ //TODO: El op code deberia ser algo así como FIN_LECTURA_MF
		return NULL;
	}

	t_list* paquete = recibir_paquete(fd_modulo);
	char* valor = list_get(paquete, 0);
	return valor;
}





