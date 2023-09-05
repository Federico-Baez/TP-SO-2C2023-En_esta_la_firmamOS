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
	logger = log_create("memoria.log", "[Memoria]", 0, LOG_LEVEL_INFO);
	config = config_create(argv[1]);
	if(config=NULL){
		log_error(logger, "No se encontro el path");
		config_destroy(config);
		log_destroy(logger);
		exit(1);
	}
	log_obligatorio = log_create("memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);
	leer_config();

	//TODO: verificar como inicializar memoria
	inicializar_memoria();

	fd_memoria = iniciar_servidor(logger, IP_MEMORIA, PUERTO_ESCUCHA);



}
void leer_config(){

}
void inicializar_memoria(){

}



/*----------------TODO COMUNICACION SOCKETS --------*/
static void procesar_conexion(void *void_args){
	int* args = (int*) void_args;
	int cliente_socket = *args;
	t_list* paquete_recibido;
	op_code cod_op;
	while(cliente_socket != -1){
		if(recv(cliente_socket,&cod_op,sizeof(cod_op),0)!= sizeof(cod_op)){

			log_info(logger, "El cliente se desconecto del servidor %s .",server_name);
			return;
		}
		switch(cod_op){
			MENSAJE:
				recibir_mensaje(logger, cliente_socket);
				break;
			PAQUETE:
				t_list* paquete_recibido = recibir_paquete(cliente_socket);
				log_info(logger, "Se reciben los siguientes paquetes: ");
				list_iterate(paquete_recibido, (void*)iterator);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				break;
		default:
			log_error(logger, "Operacion desconocida. No quieras meter la pata");

		}

	}
}

void iterator(char *value) {
	log_info(logger, "%s", value);
}

int server_escucha(){
	server_name = "Memoria";
	int cliente_socket = esperar_cliente(logger, server_name, fd_memoria);
	if(cliente_socket != -1){
		pthread_t hilo_cliente;
		int* args = malloc(sizeof(int));
		args = &cliente_socket;
		pthread_create(&hilo_cliente, NULL, (void*) procesar_conexion, (void*)args);
		pthread_detach(hilo_cliente);
		return 1;
	}
	return 0;
}








