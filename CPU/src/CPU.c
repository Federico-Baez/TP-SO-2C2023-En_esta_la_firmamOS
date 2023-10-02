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

	//log_info(cpu_logger, "Servidor listo para recibir a Kernel\n");

	pthread_t hilo_cpu_dispatch, hilo_cpu_interrupt, hilo_cpu_memoria;

	pthread_create(&hilo_cpu_memoria, NULL, (void*)atender_cpu_memoria, NULL);
	pthread_detach(hilo_cpu_memoria);

	pthread_create(&hilo_cpu_interrupt, NULL, (void*)atender_cpu_interrupt, NULL);
	pthread_detach(hilo_cpu_interrupt);

	pthread_create(&hilo_cpu_dispatch, NULL, (void*)atender_cpu_dispatch, NULL);
	pthread_join(hilo_cpu_dispatch, NULL);

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

static void atender_mensajes_kernel_v2(t_buffer* buffer, char* tipo_de_hilo){
	int tamanio = recibir_int_del_buffer(buffer);
	char* mensaje = recibir_string_del_buffer(buffer);
	log_info(cpu_logger, "[KERNEL_%s]> [%d]%s", tipo_de_hilo, tamanio, mensaje);
	free(mensaje);
	//free(buffer->stream);
	free(buffer);
}


void atender_cpu_dispatch(){
	fd_kernel_dispatch = esperar_cliente(cpu_logger, "Kernel por dispatch", server_fd_cpu_dispatch);
	gestionar_handshake_como_server(fd_kernel_dispatch, cpu_logger);
	log_info(cpu_logger, "::::::::::: KERNEL CONECTADO POR DISPATCH ::::::::::::");
	while(1){
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		t_buffer* unBuffer;
		//log_info(cpu_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case EJECUTAR_PROCESO_KC:
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
			//
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
			atender_mensajes_kernel_v2(unBuffer, "Dispatch");
			break;
		case -1:
			log_error(cpu_logger, "[DESCONEXION]: KERNEL_Dispatch");
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida KERNEL_Dispatch");
			//free(unBuffer);
			break;
		}
	}
	log_info(cpu_logger, "Saliendo del hilo de CPU_DISPATCH - KERNEL");
}

void atender_cpu_interrupt(){
	fd_kernel_interrupt = esperar_cliente(cpu_logger, "Kernel por interrupt", server_fd_cpu_interrupt);
	gestionar_handshake_como_server(fd_kernel_interrupt, cpu_logger);
		log_info(cpu_logger, "::::::::::: KERNEL CONECTADO POR INTERRUPT ::::::::::::");
		while(1){
			int cod_op = recibir_operacion(fd_kernel_interrupt);
			t_buffer* unBuffer;
			//log_info(cpu_logger, "Se recibio algo de KERNEL");

			switch (cod_op) {
			case FORZAR_DESALOJO_KC:
				unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);
				//
				break;
			case MENSAJES_POR_CONSOLA:
				unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);
				atender_mensajes_kernel_v2(unBuffer, "Interrupt");
				break;
			case -1:
				log_error(cpu_logger, "[DESCONEXION]: KERNEL_Interrupt");
				exit(EXIT_FAILURE);
				break;
			default:
				log_warning(cpu_logger, "Operacion desconocida KERNEL_Interrupt");
				//free(unBuffer);
				break;
			}
		}
		log_info(cpu_logger, "Saliendo del hilo de CPU_INTERRUPT - KERNEL");
}

void atender_cpu_memoria(){
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
	gestionar_handshake_como_cliente(fd_memoria, "MEMORIA", cpu_logger);
	identificarme_con_memoria(fd_memoria, CPU);
	log_info(cpu_logger, "HANDSHAKE CON MEMORIA [EXITOSO]");
	while(1){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		//log_info(cpu_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case PETICION_INFO_RELEVANTE_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case PETICION_DE_INSTRUCCIONES_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case PETICION_DE_EJECUCION_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case CONSULTA_DE_PAGINA_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case -1:
			log_error(cpu_logger, "[DESCONEXION]: MEMORIA");
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida MEMORIA");
			//free(unBuffer);
			break;
		}
	}
	log_info(cpu_logger, "Saliendo del hilo de CPU - MEMORIA");
}
