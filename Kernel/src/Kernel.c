/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "../include/Kernel.h"

int main(int argc, char** argv) {
	kernel_logger = log_create("memoria.log", "[Memoria]", 0, LOG_LEVEL_INFO);
	kernel_log_obligatorio = log_create("memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);

	kernel_config = config_create(argv[1]);
	if(kernel_config == NULL){
		log_error(kernel_logger, "No se encontro el path \n");
		config_destroy(kernel_config);
		log_destroy(kernel_logger);
		exit(1);
	}

	leer_config(kernel_logger);
}


void leer_config(t_config* config){
	IP_MEMORIA=config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA=config_get_int_value(config,"PUERTO_MEMORIA");
	IP_FILESYSTEM=config_get_string_value(config,"IP_FILESYSTEM");
	PUERTO_FILESYSTEM=config_get_int_value(config,"PUERTO_FILESYSTEM");
	IP_CPU=config_get_string_value(config,"IP_CPU");
	PUERTO_CPU_DISPATCH=config_get_int_value(config,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT=config_get_int_value(config,"PUERTO_CPU_INTERRUPT");
	char* algoritmo_planificacion =config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	asignar_planificador_cp(algoritmo_planificacion);
	QUANTUM=config_get_int_value(config,"QUANTUM");
	RECURSOS=config_get_array_value(config, "RECURSOS");
	INSTANCIAS_RECURSOS= config_get_array_value(config, "INSTANCIAS_RECURSOS");
	GRADO_MULTIPROGRAMACION_INI=config_get_int_value(config,"GRADO_MULTIPROGRAMACION_INI");

}

void asignar_planificador_cp(char* algoritmo_planificacion){
	if (strcmp(algoritmo_planificacion, "FIFO") == 0) {
			ALGORITMO_PLANIFICACION = FIFO;
		} else if (strcmp(algoritmo_planificacion, "RR") == 0) {
			ALGORITMO_PLANIFICACION = ROUNDROBIN;
		} else if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
			ALGORITMO_PLANIFICACION = PRIORIDADES;
		} else {
			log_error(kernel_logger, "No se encontro el algoritmo de planificacion de corto plazo");
		}
}

bool conectarse_a_modulos(){
	pthread_t  conexion_fs;
	pthread_t  conexion_cpu_dispatcher;
	pthread_t  conexion_cpu_interrupt;

	fd_cpu_dispatcher = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	//TODO EN EL CASO DE TENER UNA INTERRUPCION SE DEBE DE CREAR OTRA CONEXION?
	pthread_create(&conexion_cpu_dispatcher, NULL, (void*)procesar_conexion, (void*) &fd_cpu_dispatcher);
	pthread_create(&conexion_cpu_interrupt, NULL, (void*)procesar_conexion, (void*) &fd_cpu_interrupt);

	pthread_detach(conexion_cpu_dispatcher);
	pthread_detach(conexion_cpu_interrupt);


	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	pthread_create(&conexion_fs, NULL, (void*)procesar_conexion, (void*) &fd_filesystem);
	pthread_detach(conexion_fs);


	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	return fd_cpu_dispatcher != -1 && fd_cpu_interrupt != -1 && fd_filesystem != -1 && fd_memoria != -1;

}

void procesar_conexion(){}





