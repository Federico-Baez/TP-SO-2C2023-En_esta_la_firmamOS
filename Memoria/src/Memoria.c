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

	//memoria_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	memoria_config = config_create("memoria.config");

	if(memoria_config == NULL){
		log_error(memoria_logger, "No se encontro el path \n.");
		config_destroy(memoria_config);
		log_destroy(memoria_logger);
		exit(1);
	}

	leer_config(memoria_config);
	leer_log();

	//TODO: verificar como inicializar memoria
	//inicializar_memoria();

	server_fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);

	//TODO: ¿Conectar a filesystem antes o después de la escucha?
	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);

	//TODO: Se que esta la función de server_escucha pero dejo esto comentado para tener una guía
	//fd_kernel = esperar_cliente(memoria_logger, "Kernel", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "File System", server_fd_memoria);
	//fd_kernel = esperar_cliente(memoria_logger, "CPU", server_fd_memoria);

	while(server_escucha(server_fd_memoria)){
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

}

void finalizar_memoria(){
	log_destroy(memoria_logger);
	log_destroy(memoria_log_obligatorio);
	config_destroy(memoria_config);
}


/*----------------TODO COMUNICACION SOCKETS --------*/
static void procesar_conexion(void *void_args){
	int* args = (int*) void_args;
	int cliente_socket = *args;
	t_list* paquete_recibido;
	op_code cod_op;
	while(cliente_socket != -1){
		if(recv(cliente_socket,&cod_op,sizeof(cod_op),0)!= sizeof(cod_op)){

			log_info(memoria_logger, "El cliente se desconecto del servidor %s .",server_name);
			return;
		}
		switch(cod_op){
			MENSAJE:
				recibir_mensaje(memoria_logger, cliente_socket);
				break;
			PAQUETE:
				t_list* paquete_recibido = recibir_paquete(cliente_socket);
				log_info(memoria_logger, "Se reciben los siguientes paquetes: ");
				list_iterate(paquete_recibido, (void*)iterator);
				break;
			case -1:
				log_error(memoria_logger, "el cliente se desconecto. Terminando servidor");
				break;
		default:
			log_error(memoria_logger, "Operacion desconocida. No quieras meter la pata");

		}

	}
}

void iterator(char *value) {
	log_info(memoria_logger, "%s", value);
}

int server_escucha(int fd_memoria){
	server_name = "Memoria";
	int cliente_socket = esperar_cliente(memoria_logger, server_name, fd_memoria);
	if(cliente_socket != -1){
		pthread_t hilo_cliente;
		int* args = malloc(sizeof(int));
		args = &cliente_socket;
		pthread_create(&hilo_cliente, NULL, (void*) procesar_conexion, (void*)args);
		log_info(memoria_logger, "[THREAD] Creo hilo para atender");
		pthread_detach(hilo_cliente);
		return 1;
	}
	log_info(memoria_logger, "Se activa el servidor %s ", server_name);
	return 0;
}








