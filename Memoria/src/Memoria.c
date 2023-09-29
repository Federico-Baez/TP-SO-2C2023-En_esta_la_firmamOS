/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "../include/Memoria.h"

int main(int argc, char** argv) {
	memoria_logger = log_create("Memoria.log", "[Memoria]", 1, LOG_LEVEL_INFO);
	memoria_log_obligatorio = log_create("Memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);

	memoria_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//char* config_path = "../memoria.config";
	//memoria_config = config_create(config_path);

	if(memoria_config == NULL){
		log_error(memoria_logger, "No se encontro el path \n.");
		config_destroy(memoria_config);
		log_destroy(memoria_logger);
		exit(1);
	}

	leer_config(memoria_config);
	leer_log();
	lista_instrucciones(memoria_logger, PATH_INSTRUCCIONES);

	//TODO: verificar como inicializar memoria

	inicializar_memoria();

	//inicializar_memoria();


	server_fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);

	//TODO: ¿Conectar a filesystem antes o después de la escucha?
	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);

	//TODO: Se que esta la función de server_escucha pero dejo esto comentado para tener una guía
	//fd_kernel = esperar_cliente(memoria_logger, "Kernel", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "File System", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "CPU", server_fd_memoria);

	while(server_escucha()){
		log_info(memoria_logger, "Se abre servidor de Memoria");
	}

	finalizar_memoria();

	return EXIT_SUCCESS;
}
void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(config, "PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
	PATH_INSTRUCCIONES = config_get_string_value(config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(config, "RETARDO_RESPUESTA");
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
}

void leer_log(){
	log_info(memoria_logger, "IP_MEMORIA: %s", IP_MEMORIA);
	log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
	log_info(memoria_logger, "IP_FILESYSTEM: %s", IP_FILESYSTEM);
	log_info(memoria_logger, "PUERTO_FILESYSTEM: %s", PUERTO_FILESYSTEM);
	log_info(memoria_logger, "TAM_MEMORIA: %d",TAM_MEMORIA);
	log_info(memoria_logger, "TAM_PAGINA: %d",TAM_PAGINA);
	log_info(memoria_logger, "PATH_INSTRUCCIONES: %s",PATH_INSTRUCCIONES);
	log_info(memoria_logger, "RETARDO_RESPUESTA: %d",RETARDO_RESPUESTA);
	log_info(memoria_logger, "ALGORITMO DE ASIGNACION: %s \n",ALGORITMO_REEMPLAZO);

}
void inicializar_memoria(){
	log_info(memoria_logger, "Inicianlizando memoria");
	espacio_usuario = malloc(TAM_MEMORIA);
	lst_marco = list_create();
	int cant_marcos = TAM_MEMORIA/TAM_PAGINA;


	for(int i=0;i<= cant_marcos;i++){
		Marco* nuevo_marco  = crear_marco(TAM_PAGINA*i, true);

		list_add(lst_marco,nuevo_marco);

	}

}


Marco* crear_marco(int base, bool presente){
	Marco *nuevo_marco = malloc(sizeof(Marco));
	nuevo_marco->base = base;
	nuevo_marco->presente = presente;
	return nuevo_marco;
}
/*
tabla_paginas* crear_tabla_paginas(int pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
	log_debug(memoria_log_obligatorio,"[PAG]: Creo tabla de paginas PID %d", pid);
	char* spid[4];
	string_from_format(spid, "%d", pid);
	nueva_tabla->page = list_create();

	//es una variable que deberia de bloquear
	dictionary_put(tablas,spid, nueva_tabla);
	// aca deberia de desbloquear este recurso
	return nueva_tabla;


}
*/
void finalizar_memoria(){
	log_destroy(memoria_logger);
	log_destroy(memoria_log_obligatorio);
	config_destroy(memoria_config);
}



/*----------------TODO COMUNICACION SOCKETS --------*/
static void procesar_conexion(void *void_args){
	int* args = (int*) void_args;
	int cliente_socket = *args;
//	int valor1, valor2;
//	char* myString;
//	char* unChoclo;
//	t_buffer* myBuffer = malloc(sizeof(t_buffer));
//	int size;
//	t_list* paquete_recibido;
	//op_code cod_op;
	int cod_op;
	while(cliente_socket != -1){
		if(recv(cliente_socket,&cod_op,sizeof(cod_op),MSG_WAITALL)!= sizeof(cod_op)){

			log_info(memoria_logger, "El cliente se desconecto del servidor %s .",server_name);
			return;
		}
		switch(cod_op){
		case MENSAJE:
			recibir_mensaje(memoria_logger, cliente_socket);
			break;
		case PAQUETE:
			//int* numero_xd = recibir_int(logger, coso)

			t_list* paquete_recibido = recibir_paquete_int(cliente_socket);
			//t_list* paquete_recibido = recibir_paquete(cliente_socket);
			log_info(memoria_logger, "Se reciben los siguientes paqubetes: ");
			list_iterate(paquete_recibido, (void*)iterator);

			break;
		case ADMINISTRAR_PAGINA_MEMORIA:
			log_info(memoria_logger, "Se crea la pagina en memoria");
			int valor_m, valor2_m;
			char* otro_dato;
			char* dato_m;
			t_buffer* myBuffer = malloc(sizeof(t_buffer));
			int size;
			myBuffer->stream = recibir_buffer(&size, cliente_socket);
			myBuffer->size = size;

			valor_m = recibir_int_del_buffer(myBuffer);
			otro_dato = recibir_string_del_buffer(myBuffer);
			dato_m = (char*)recibir_choclo_del_buffer(myBuffer);
			valor2_m = recibir_int_del_buffer(myBuffer);

			log_info(memoria_logger, "Recibido exitoso:%d | %s | %s | %d", valor_m, otro_dato, dato_m, valor2_m);

			free(myBuffer->stream);
			free(myBuffer);
			break;
		case -1:
			log_error(memoria_logger, "el cliente se desconecto. Terminando servidor");
			break;
		case PRUEBAS:

//			myBuffer->stream = recibir_buffer(&size, fd_kernel);
//			myBuffer->size = size;
//
//			valor1 = recibir_int_del_buffer(myBuffer);
//			myString = recibir_string_del_buffer(myBuffer);
//			unChoclo = (char*)recibir_choclo_del_buffer(myBuffer);
//			valor2 = recibir_int_del_buffer(myBuffer);
//
//			log_info(memoria_logger, "Recibido exitoso:%d | %s | %s | %d", valor1, myString, unChoclo, valor2);
//
//			free(myBuffer->stream);
//			free(myBuffer);

			break;
		case HANDSHAKE:
			char* saludo;
			t_buffer* buffer_handshake = malloc(sizeof(t_buffer));
			int size_handshake;
			buffer_handshake->stream = recibir_buffer(&size_handshake, cliente_socket);
			buffer_handshake->size = size_handshake;

			saludo = recibir_string_del_buffer(buffer_handshake);
			log_info(memoria_logger, "%s CONECTADO !!!!!!!", saludo);

			free(buffer_handshake->stream);
			free(buffer_handshake);

			//////

			op_code operacion_handshake = HANDSHAKE;
			t_paquete* paquete_handshake = crear_super_paquete(operacion_handshake);
			cargar_string_al_super_paquete(paquete_handshake, "MEMORIA");
			enviar_paquete(paquete_handshake, cliente_socket);
			eliminar_paquete(paquete_handshake);

			break;
		default:
			log_error(memoria_logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

	}
}

void iterator(int *value) {
	log_info(memoria_logger, "%d", *value);
}

int server_escucha(){
	server_name = "Memoria";
	int cliente_socket = esperar_cliente(memoria_logger, server_name, server_fd_memoria );
	if(cliente_socket != -1){
		pthread_t hilo_cliente;
		int *args = malloc(sizeof(int));
		args = &cliente_socket;
		pthread_create(&hilo_cliente, NULL, (void*) procesar_conexion, (void*)args);
		log_info(memoria_logger, "[THREAD] Creo hilo para atender");
		pthread_detach(hilo_cliente);
		return 1;
	}
	log_info(memoria_logger, "Se activa el servidor %s ", server_name);
	return 0;
}

void enviar_instrucciones_a_cpu(){
	lista_instrucciones(memoria_logger, PATH_INSTRUCCIONES);
}








