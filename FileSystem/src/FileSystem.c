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

void inicializar_archivos(){
	crear_fat();
	inicializar_archivo_de_bloques();
}

void crear_fat(){
	fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	if (fd_archivoTablaFAT == -1) {
		fd_archivoTablaFAT = open(PATH_FAT, O_CREAT | O_RDWR);
		ftruncate(fd_archivoTablaFAT, tamanio_fat);
	}

	tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}

	tablaFatEnMemoria[0] = EOF_FS;
}

void inicializar_archivo_de_bloques(){
	fd_archivoBloques = open(PATH_BLOQUES, O_RDWR);
	tamanio_archivo_bloques = CANT_BLOQUES_TOTAL * TAM_BLOQUE;

	if(fd_archivoBloques == -1){
		fd_archivoBloques = open(PATH_BLOQUES, O_CREAT | O_RDWR);
		ftruncate(fd_archivoBloques, tamanio_archivo_bloques);
	}

	mapear_bloques_swap(fd_archivoBloques);
	mapear_bloques_de_archivo(fd_archivoBloques);
}

void mapear_bloques_swap(int fd){
	bitmapSWAP = bitarray_create_with_mode(bitmap_swap, CANT_BLOQUES_SWAP/8, LSB_FIRST);

	bloquesSwapEnMemoria = mmap(NULL, CANT_BLOQUES_SWAP*TAM_BLOQUE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (bloquesSwapEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear los bloques SWAP");
		exit(1);
	}
}

void mapear_bloques_de_archivo(int fd){
	bloquesFATEnMemoria = mmap(NULL, (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP)*TAM_BLOQUE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CANT_BLOQUES_SWAP*TAM_BLOQUE);

	if (bloquesFATEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear los bloques de archivo");
		exit(1);
	}
}

void destruir_fcb(t_fcb* fcb){
	free(fcb);
	fcb = NULL;
}

void destruir_archivo_fcb(t_archivo_fcb* archivo_fcb){
	config_destroy(archivo_fcb->archivo_fcb);
	free(archivo_fcb);
	archivo_fcb = NULL;
}

void destruir_listas_fcbs(){
	list_destroy_and_destroy_elements(lista_struct_fcbs, (void*) destruir_fcb);
	list_destroy_and_destroy_elements(lista_configs_fcbs, (void*) destruir_archivo_fcb);
}

void finalizar_filesystem(){
	destruir_listas_fcbs();

	close(fd_archivoTablaFAT);
	close(fd_archivoBloques);

	list_destroy(lista_struct_fcbs);
	list_destroy(lista_configs_fcbs);
	bitarray_destroy(bitmapSWAP);

	log_destroy(filesystem_logger);
	log_destroy(filesystem_log_obligatorio);
	config_destroy(filesystem_config);

	liberar_conexion(fd_kernel);
	liberar_conexion(fd_memoria);
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

	while(1){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(fd_kernel);
		//log_info(filesystem_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {

		case MANEJAR_F_OPEN_KF:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			ejecutar_en_un_hilo_nuevo_detach(atender_f_open_de_kernel, unBuffer);
			break;
		case MANEJAR_F_TRUNCATE_KF:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			ejecutar_en_un_hilo_nuevo_detach(atender_f_truncate_de_kernel, unBuffer);
			break;
		case MANEJAR_F_READ_KF:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			ejecutar_en_un_hilo_nuevo_detach(atender_f_read_de_kernel, unBuffer);
			break;
		case MANEJAR_F_WRITE_KF:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			ejecutar_en_un_hilo_nuevo_detach(atender_f_write_de_kernel, unBuffer);
			break;
		case MENSAJES_POR_CONSOLA:
		    unBuffer = recibiendo_super_paquete(fd_kernel);
		    atender_mensajes_kernel(unBuffer);
		    break;
		case -1:
			log_error(filesystem_logger, "[DESCONEXION]: KERNEL");
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(filesystem_logger, "Operacion desconocida");
			//free(unBuffer);
			break;
		}

		free(unBuffer); // [Ojo] controlar esto porque puede romper si libero antes
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
		case PETICION_ASIGNACION_BLOQUE_SWAP_FM:
			//[int pid][int catn_bloques]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_asignacion_de_bloques_por_creacion_de_proceso(unBuffer);

			break;
		case LIBERAR_PAGINAS_FM:
			//[int cant_bloq_swap][int][int]...[int]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_peticion_de_libarar_bloques_swap(unBuffer);

			break;
		case GUARDAR_MARCO_EN_SWAP_FM:
			//[int pos_swap][coid* pagina]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_peticion_de_guardar_marco_en_swap(unBuffer);

			break;
		case LECTURA_MARCO_DE_SWAP_MF:
			//[int pid][int nro_pag][int pos_swap]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_pedido_de_lectura_de_pag_swap(unBuffer);

			break;
		case BLOQUE_DE_MEMORIA_A_FILESYSTEM_FM: //F_WRITE
			//[int pid][int nro_bloque][void* marco_bloque][char* nombre_archivo]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_recepcion_de_marco_bloque_de_memoria_por_f_write_de_kernel(unBuffer);

			break;
		case RPTA_BLOQUE_DE_FILESYSTEM_A_MEMORIA_FM: //F_READ
			//[int pid][char* rpta]
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_rpta_de_memoria_a_fs_por_lectura_de_marco_por_dir_fisica(unBuffer);

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

		free(unBuffer);
	}
	log_info(filesystem_logger, "Saliendo del hilo de FILESYSTEM - MEMORIA");
}
