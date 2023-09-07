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
	memoria_logger = log_create("memoria.log", "[Memoria]", 0, LOG_LEVEL_INFO);
	memoria_log_obligatorio = log_create("memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);

	memoria_config = config_create(argv[1]);
	if(memoria_config == NULL){
		log_error(memoria_logger, "No se encontro el path");
		config_destroy(memoria_config);
		log_destroy(memoria_logger);
		exit(1);
	}

	leer_config(memoria_config);

	//TODO: verificar como inicializar memoria
	inicializar_memoria();

	fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);



}
void leer_config(t_config* config){
	PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
	PATH_INSTRUCCIONES = config_get_string_value(config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(config, "RETARDO_RESPUESTA");
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
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

int server_escucha(){
	server_name = "Memoria";
	int cliente_socket = esperar_cliente(memoria_logger, server_name, fd_memoria);
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








