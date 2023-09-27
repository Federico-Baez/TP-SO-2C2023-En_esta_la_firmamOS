#include <stdio.h>
#include <stdlib.h>

#include "../include/CPU.h"

int main(int argc, char** argv) {
	cpu_logger = log_create("cpu.log", "[CPU]", 1, LOG_LEVEL_INFO);
	cpu_log_obligatorio = log_create("cpu_log_obligatorio.log", "[CPU - Log obligatorio]", 1, LOG_LEVEL_INFO);

	cpu_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//cpu_config = config_create("cpu.config");

	if(cpu_config == NULL){
		log_error(cpu_logger, "No se encontro el path del config\n");
		config_destroy(cpu_config);
		log_destroy(cpu_logger);
		log_destroy(cpu_log_obligatorio);
		exit(2);
	}

	leer_config(cpu_config);

	server_fd_cpu_dispatch = iniciar_servidor(cpu_logger, IP_CPU, PUERTO_ESCUCHA_DISPATCH);
	server_fd_cpu_interrupt = iniciar_servidor(cpu_logger, IP_CPU, PUERTO_ESCUCHA_INTERRUPT);
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	log_info(cpu_logger, "Servidor listo para recibir a Kernel\n");

	fd_kernel_dispatch = esperar_cliente(cpu_logger, "Kernel por dispatch", server_fd_cpu_dispatch);
	fd_kernel_interrupt = esperar_cliente(cpu_logger, "Kernel por interrupt", server_fd_cpu_interrupt);

	t_list* lista;

	//HANDSHAKE A TRAVES DE DISPATCH
	while (1) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cpu_logger, fd_kernel_dispatch);
				break;
			case PAQUETE:
				lista = recibir_paquete(fd_kernel_dispatch);
				log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(cpu_logger, "el cliente se desconecto. Terminando servidor");
				return EXIT_FAILURE;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
	}

	//HANDSHAKE A TRAVES DE INTERRUPT
	while (1) {
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cpu_logger, fd_kernel_interrupt);
				break;
			case PAQUETE:
				lista = recibir_paquete(fd_kernel_interrupt);
				log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(cpu_logger, "el cliente se desconecto. Terminando servidor");
				return EXIT_FAILURE;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
	}

	return EXIT_SUCCESS;
}

void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
}


void iterator(char* value) {
	log_info(cpu_logger,"%s", value);
}
