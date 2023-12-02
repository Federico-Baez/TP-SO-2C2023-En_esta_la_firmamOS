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

	//log_info(filesystem_logger, "Servidor listo para recibir a Kernel\n");

	pthread_t hilo_kernel, hilo_memoria;

	pthread_create(&hilo_memoria, NULL, (void*)atender_memoria, NULL);
	pthread_detach(hilo_memoria);
	//pthread_join(hilo_memoria, NULL);

	pthread_create(&hilo_kernel, NULL, (void*)atender_filesystem_kernel, NULL);
	pthread_join(hilo_kernel, NULL);

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

void inicializar_fcbs(){
	DIR *directorio_archivos = opendir(PATH_FCB);
	struct dirent *fcb;

	if(directorio_archivos == NULL){
		log_error(filesystem_logger, "No se pudo abrir el directorio de fcbs");
		exit(1);
	}

	while((fcb = readdir(directorio_archivos)) != NULL){
		if (strcmp(fcb->d_name, ".") == 0 || strcmp(fcb->d_name, "..") == 0){
			continue;
		}
		log_info(filesystem_logger, "Lei esto del directorio: %s", fcb->d_name);

		t_archivo *archivo = malloc(sizeof(t_archivo));
		archivo->nombre = malloc(strlen(fcb->d_name));
		strcpy(archivo->nombre, fcb->d_name);

		char* path_archivo = malloc(strlen(PATH_FCB) + strlen(fcb->d_name));
		strcpy(path_archivo, PATH_FCB);
		strcat(path_archivo, fcb->d_name);
		archivo->archivo_fcb = config_create(path_archivo);

		list_add(lista_fcbs, archivo);
	}

	closedir(directorio_archivos);
}

void crear_archivo_de_bloques(){
	int fd = open(PATH_BLOQUES, O_CREAT | O_RDWR);
	tamanio_archivo_bloques = TAM_BLOQUE * CANT_BLOQUES_TOTAL;
	buffer_bloques = mmap(NULL, tamanio_archivo_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(fd == -1){
		log_error(filesystem_logger, "Hubo un problema creando el archivo de bloques");
		exit(1);
	}

	close(fd);
}

void crear_fat(){
	int fd = open(PATH_FAT, O_CREAT | O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);
	buffer_fat = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(fd == -1){
		log_error(filesystem_logger, "Hubo un problema creando el archivo fat");
		exit(1);
	}

	close(fd);
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

void atender_mensajes_kernel(t_buffer* buffer){
	int tamanio = recibir_int_del_buffer(buffer);
	char* mensaje = recibir_string_del_buffer(buffer);
	log_info(filesystem_logger, "[KERNEL]> [%d]%s", tamanio, mensaje);
	free(mensaje);
	//free(buffer->stream);
	free(buffer);
}


void atender_filesystem_kernel(){
	fd_kernel = esperar_cliente(filesystem_logger, "Kernel", server_fd_filesystem);
	gestionar_handshake_como_server(fd_kernel, filesystem_logger);
	log_info(filesystem_logger, "::::::::::: KERNEL CONECTADO ::::::::::::");
	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_kernel);
		t_buffer* unBuffer;
		//log_info(filesystem_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case SYSCALL_KF:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			//
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			atender_mensajes_kernel(unBuffer);
			break;
		case -1:
			log_error(filesystem_logger, "[DESCONEXION]: KERNEL");
			//control_key = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(filesystem_logger, "Operacion desconocida");
			//free(unBuffer);
			break;
		}
	}
	log_info(filesystem_logger, "Saliendo del hilo de FILESYSTEM - KERNEL");
}

void atender_memoria(){
	gestionar_handshake_como_cliente(fd_memoria, "MEMORIA", filesystem_logger);
	identificarme_con_memoria(fd_memoria, FILESYSTEM);
	log_info(filesystem_logger, "HANDSHAKE CON MEMORIA [EXITOSO]");

	int control_key = 1;
	while(control_key){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		log_info(filesystem_logger, "Se recibio algo de MEMORIA");

		switch (cod_op) {
		case SYSCALL_KF:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_mensajes_kernel(unBuffer);
			break;
		case -1:
			log_error(filesystem_logger, "[DESCONEXION]: KERNEL");
			control_key = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(filesystem_logger, "Operacion desconocida");
			free(unBuffer);
			break;
		}
	}
	log_info(filesystem_logger, "Saliendo del hilo de FILESYSTEM - MEMORIA");
}

