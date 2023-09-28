#include <stdio.h>
#include <stdlib.h>

#include "../include/FileSystem.h"

int main(int argc, char** argv) {
	filesystem_logger = log_create("filesystem.log", "[File System]", 1, LOG_LEVEL_INFO);
	filesystem_log_obligatorio = log_create("filesystem_log_obligatorio.log", "[File System - Log obligatorio]", 1, LOG_LEVEL_INFO);

	filesystem_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//filesystem_config = config_create("filesystem.config");

	if(filesystem_config == NULL){
		log_error(filesystem_logger, "No se encontro el path del config\n");
		config_destroy(filesystem_config);
		log_destroy(filesystem_logger);
		log_destroy(filesystem_log_obligatorio);
		exit(2);
	}

	leer_config(filesystem_config);

	server_fd_filesystem = iniciar_servidor(filesystem_logger, IP_FILESYSTEM, PUERTO_ESCUCHA);
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	log_info(filesystem_logger, "Servidor listo para recibir a Kernel\n");

	fd_kernel = esperar_cliente(filesystem_logger, "Kernel", server_fd_filesystem);

	/*Recibiendo por funcionalidades practicas de serializacion */
	int valor1, valor2;
	char* myString;
	char* unChoclo;
	int cod_op = recibir_operacion(fd_kernel);
	switch(cod_op){
	case PRUEBAS:
		t_buffer* myBuffer = malloc(sizeof(t_buffer));
		int size;
		myBuffer->stream = recibir_buffer(&size, fd_kernel);
		myBuffer->size = size;

		valor1 = recibir_int_del_buffer(myBuffer);
		myString = recibir_string_del_buffer(myBuffer);
		unChoclo = (char*)recibir_choclo_del_buffer(myBuffer);
		valor2 = recibir_int_del_buffer(myBuffer);

		log_info(filesystem_logger, "Recibido exitoso:%d | %s | %s | %d", valor1, myString, unChoclo, valor2);

		free(myBuffer->stream);
		free(myBuffer);

		break;
	default:
		log_warning(filesystem_logger,"Operacion desconocida. No quieras meter la pata");
		break;
	}

	return EXIT_SUCCESS;
}

void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	PUERTO_ESCUCHA = config_get_string_value(config,"PUERTO_ESCUCHA");
	PATH_FAT = config_get_string_value(config,"PATH_FAT");
	PATH_BLOQUES = config_get_string_value(config,"PATH_BLOQUES");
	PATH_FCB = config_get_string_value(config,"PATH_FCB");
	CANT_BLOQUES_TOTAL = config_get_int_value(config,"CANT_BLOQUES_TOTAL");
	CANT_BLOQUES_SWAP = config_get_int_value(config,"CANT_BLOQUES_SWAP");
	TAM_BLOQUE = config_get_int_value(config,"TAM_BLOQUE");
	RETARDO_ACCESO_BLOQUE = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
	RETARDO_ACCESO_FAT = config_get_int_value(config,"RETARDO_ACCESO_FAT");
}

void finalizar_filesystem(){
	log_destroy(filesystem_logger);
	log_destroy(filesystem_log_obligatorio);
	config_destroy(filesystem_config);
	liberar_conexion(fd_kernel);
}

void iterator(char* value) {
	log_info(filesystem_logger,"%s", value);
}

