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
	crear_archivo_de_bloques();
	inicializar_fcbs();
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

		t_archivo_fcb *archivo = malloc(sizeof(t_archivo_fcb));
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

void destruir_lista_fcbs(){
	list_destroy_and_destroy_elements(lista_fcbs, (void*) destruir_archivo);
}

void destruir_archivo(t_archivo_fcb* archivo_fcb){
	config_destroy(archivo_fcb->archivo_fcb);
	free(archivo_fcb);
	archivo_fcb = NULL;
}

void crear_archivo_de_bloques(){
	for(int i=1; i <= CANT_BLOQUES_TOTAL; i++){
		t_bloque* bloque = malloc(sizeof(t_bloque));
		bloque->id_bloque = i;
		bloque->esta_libre = 1;
		bloque->contenido = NULL;
		list_add(lista_bloques, bloque);
	}

	FILE* archivo_bloques = fopen(PATH_BLOQUES, "a+b");

	if(archivo_bloques != NULL){
		for(int i=1; i <= CANT_BLOQUES_TOTAL; i++){
			t_bloque* bloque_a_persistir = list_get(tabla_fat, i);
			fwrite(bloque_a_persistir->contenido, TAM_BLOQUE, 1, archivo_bloques);
		}

		fclose(archivo_bloques);
	}


	/*int fd_arch_bloques = open(PATH_BLOQUES, O_CREAT | O_RDWR);
	tamanio_particion_swap = TAM_BLOQUE * CANT_BLOQUES_SWAP;
	tamanio_particion_bloques = TAM_BLOQUE * (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP);
	buffer_swap = mmap(NULL, tamanio_particion_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_arch_bloques, 0);
	buffer_bloques = mmap(NULL, tamanio_particion_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd_arch_bloques, 0);

	if(fd_arch_bloques == -1){
		log_error(filesystem_logger, "Error al crear el archivo de bloques");
		exit(1);
	}

	close(fd_arch_bloques);*/
}

void crear_fat(){
	//PRIMER BLOQUE FAT
	t_bloque_fat* bloque = malloc(sizeof(t_bloque_fat));
	bloque->id_bloque = 0;
	bloque->puntero_siguiente = UINT32_MAX;
	bloque->esta_libre = 0;
	bloque->eof = NULL;
	list_add(tabla_fat, bloque);

	for(int i=1; i <= (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP); i++){
		t_bloque_fat* bloque = malloc(sizeof(t_bloque_fat));
		bloque->id_bloque = i;
		bloque->puntero_siguiente = 0;
		bloque->esta_libre = 1;
		bloque->eof = NULL;
		list_add(tabla_fat, bloque);
	}

	FILE* archivo_fat = fopen(PATH_FAT, "a+b");

	if(archivo_fat != NULL){
		for(int i=0; i < (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP); i++){
			t_bloque_fat* bloque_a_persistir = list_get(tabla_fat, i);
			fwrite(bloque_a_persistir->puntero_siguiente, sizeof(uint32_t), 1, archivo_fat);
		}

		fclose(archivo_fat);
	}

	/*int fd_fat = open(PATH_FAT, O_CREAT | O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);
	buffer_tabla_fat = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fat, 0);

	if(fd_fat == -1){
		log_error(filesystem_logger, "Error al crear el archivo FAT");
		exit(1);
	}

	close(fd_fat);*/
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

//--------------------OPERACIONES------------------------

void ejecutar_f_open(char* nombre_archivo){
	log_info(filesystem_log_obligatorio, "Abrir Archivo: %s", nombre_archivo);
	t_archivo_fcb* archivo_fcb = buscar_fcb(nombre_archivo);
	if(archivo_fcb != NULL){
		int tamanio_fcb = config_get_int_value(archivo_fcb, "TAMANIO_ARCHIVO");
		enviar_tamanio_fcb(tamanio_fcb, fd_kernel);
	}else{
		enviar_mensaje("El archivo solicitado no existe", fd_kernel);
	}
}

void ejecutar_f_create(char* nombre_archivo){
	log_info(filesystem_log_obligatorio, "Crear Archivo: %s", nombre_archivo);
	char* path_archivo = malloc(strlen(PATH_FCB) + strlen(nombre_archivo));
	strcpy(path_archivo, PATH_FCB);
	strcat(path_archivo, nombre_archivo);

	t_fcb *nuevo_fcb = malloc(sizeof(t_fcb));
	nuevo_fcb->nombre = malloc(strlen(nombre_archivo));
	strcpy(nuevo_fcb->nombre, nombre_archivo);
	nuevo_fcb->tamanio = 0;
	char* text_tamanio_archivo = malloc(10);
	sprintf(text_tamanio_archivo, "%d", nuevo_fcb->tamanio);

	FILE* file_fcb = fopen(path_archivo, "a+");

	t_archivo_fcb *archivo_fcb = malloc(sizeof(t_archivo_fcb));
	archivo_fcb->nombre = malloc(strlen(nombre_archivo));
	strcpy(archivo_fcb->nombre, nombre_archivo);
	archivo_fcb->archivo_fcb = config_create(path_archivo);

	config_set_value(archivo_fcb->archivo_fcb, "NOMBRE_ARCHIVO", nuevo_fcb->nombre);
	config_set_value(archivo_fcb->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);

	config_save(archivo_fcb->archivo_fcb);

	list_add(lista_fcbs, archivo_fcb);
	fclose(file_fcb);

	enviar_mensaje("OK", fd_kernel);
}

void ejecutar_f_truncate(char* nombre_archivo, int tamanio_nuevo){
	log_info(filesystem_log_obligatorio, "Truncar Archivo: %s - Tamaño: %d", nombre_archivo, tamanio_nuevo);
	t_archivo_fcb* fcb = obtener_archivo(nombre_archivo);
	int tamanio_viejo = config_get_int_value(fcb->archivo_fcb, "TAMANIO_ARCHIVO");
	char* texto_tamanio_nuevo_archivo = malloc(10);
	sprintf(texto_tamanio_nuevo_archivo, "%d", tamanio_nuevo);

	int tamanio_restante = tamanio_nuevo;
	if(tamanio_viejo == 0){
		asignar_bloque_primer_truncate(fcb);
		tamanio_restante = tamanio_nuevo - TAM_BLOQUE;
	}

	log_info(filesystem_logger, "El tamanio nuevo es %s", texto_tamanio_nuevo_archivo);

	config_set_value(fcb->archivo_fcb, "TAMANIO_ARCHIVO", texto_tamanio_nuevo_archivo);
	config_save(fcb->archivo_fcb);

	log_info(filesystem_logger, "El tamanio viejo era %d", tamanio_viejo);

	if(tamanio_restante > tamanio_viejo){
		// AMPLIAR
		div_t cuenta_bloques_agregar = div((tamanio_restante - tamanio_viejo), TAM_BLOQUE);
		int cantidad_bloques_a_agregar = cuenta_bloques_agregar.quot;
		if(cuenta_bloques_agregar.rem != 0){
			cantidad_bloques_a_agregar++;
		}

		log_info(filesystem_logger, "La cantidad de bloques a agregar en %s es %d", nombre_archivo, cantidad_bloques_a_agregar);
		asignar_bloques(cantidad_bloques_a_agregar, fcb);

	} else if(tamanio_restante < tamanio_viejo){
		// REDUCIR
		div_t cuenta_bloque_viejo = div(tamanio_viejo - 1, TAM_BLOQUE);
		div_t cuenta_bloque_nuevo = div(tamanio_restante - 1, TAM_BLOQUE);
		int cantidad_bloques_a_sacar = cuenta_bloque_viejo.quot - cuenta_bloque_nuevo.quot;

		log_info(filesystem_logger, "La cantidad de bloques a sacar en %s es %d", nombre_archivo, cantidad_bloques_a_sacar);
		sacar_bloques(cantidad_bloques_a_sacar, fcb);
	} else{
		log_info(filesystem_logger, "No hay que sacar ni agregar ningun bloque");
	}

	//TODO: modificar tabla fat
}

void ejecutar_f_read(char* nombre_archivo, int dir_fisica, int posicion_a_leer, int pid){
	log_info(filesystem_log_obligatorio, "Leer Archivo: %s - Puntero: %d - Memoria: %d", nombre_archivo, posicion_a_leer, dir_fisica);
	t_archivo_fcb* archivo_fcb = obtener_archivo(nombre_archivo);

	//TODO: Leer la información correspondiente de los bloques a partir del puntero recibido
	char* datos_leidos = "Datos leidos"/*leer_datos(archivo_fcb, posicion_a_leer)*/;

	//Esta información se deberá enviar a la Memoria para ser escrita a partir de la dirección física recibida por parámetro
	enviar_para_escribir_valor_leido(datos_leidos, dir_fisica, pid, fd_memoria);
	recibir_fin_de_escritura(fd_memoria);
}

void ejecutar_f_write(char* nombre_archivo, int dir_fisica, int posicion_a_escribir, int pid){
	log_info(filesystem_log_obligatorio, "Escribir Archivo: %s - Puntero: %d - Memoria: %d", nombre_archivo, posicion_a_escribir, dir_fisica);
	t_archivo_fcb* archivo_fcb = obtener_archivo(nombre_archivo);

	//Solicitar a Memoria la información que se encuentra a partir de la dirección física
	enviar_para_leer_valor(dir_fisica, pid, fd_memoria);
	char* datos_a_escribir = recibir_valor_leido(fd_memoria);

	//TODO: Escribir los datos en los bloques correspondientes del archivo a partir del puntero recibido.
	//escribir_datos(archivo_fcb, posicion_a_escribir, datos_a_escribir);
}

void asignar_bloques(int cant_bloques, t_archivo_fcb* fcb){
	uint32_t bloque_inicial = config_get_int_value(fcb->archivo_fcb, "BLOQUE_INICIAL");
	//char* nombre_archivo = config_get_string_value(fcb->archivo_fcb, "NOMBRE_ARCHIVO");

	t_bloque_fat* bloque_actual = ultimo_bloque_archivo(bloque_inicial, fcb);

	for(int i = cant_bloques; i > 0; i--){
		t_bloque_fat* nuevo_bloque_libre = buscar_bloque_libre();
		bloque_actual->puntero_siguiente = nuevo_bloque_libre->id_bloque;
		list_replace(tabla_fat, bloque_actual->puntero_siguiente, nuevo_bloque_libre);

		log_info(filesystem_logger, "El nuevo bloque asignado es %d", nuevo_bloque_libre->id_bloque);

		bloque_actual = nuevo_bloque_libre;
	}

	//TODO: modificar archivo fat
}

void sacar_bloques(int cant_bloques, t_archivo_fcb* fcb){
	uint32_t bloque_inicial = config_get_int_value(fcb->archivo_fcb, "BLOQUE_INICIAL");
	//char* nombre_archivo = config_get_string_value(fcb->archivo_fcb, "NOMBRE_ARCHIVO");

	t_bloque_fat* bloque_a_sacar = malloc(sizeof(uint32_t));

	for(int i = cant_bloques; i > 0; i--){
		bloque_a_sacar = ultimo_bloque_archivo(bloque_inicial, fcb);
		bloque_a_sacar->puntero_siguiente = 0;
		bloque_a_sacar->esta_libre = 1;
		list_replace(tabla_fat, bloque_a_sacar->id_bloque, bloque_a_sacar);
		//TODO: modificar archivo fat
	}
}

t_list* obtener_bloques_de_archivo(uint32_t bloque_inicial, t_archivo_fcb* fcb){
	char* nombre_archivo = config_get_string_value(fcb->archivo_fcb, "NOMBRE_ARCHIVO");
	t_bloque_fat* bloque_actual = list_get(tabla_fat, bloque_inicial);
	t_list* bloques_de_archivo = list_create();

	list_add(bloques_de_archivo, bloque_actual);

	for(int i=1; bloque_actual->puntero_siguiente != UINT32_MAX; i++){
		log_info(filesystem_log_obligatorio, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque FS: %d", nombre_archivo, i, bloque_inicial);
		usleep(RETARDO_ACCESO_BLOQUE);
		bloque_actual = list_get(tabla_fat, bloque_actual->puntero_siguiente);
		list_add(bloques_de_archivo, bloque_actual);
	}

	return bloques_de_archivo;
}

t_bloque_fat* ultimo_bloque_archivo(uint32_t bloque_inicial, t_archivo_fcb* fcb){
	t_list* bloques_de_archivo = obtener_bloques_de_archivo(bloque_inicial, fcb);
	return list_get(bloques_de_archivo, list_size(bloques_de_archivo)-1);
}

t_archivo_fcb* buscar_fcb(char* nombre_archivo){
	for(int i = 0; i < list_size(lista_fcbs); i++){
		t_archivo_fcb* archivo_buscado = list_get(lista_fcbs, i);

		if(strcmp(archivo_buscado->nombre, nombre_archivo) == 0){
			return archivo_buscado;
		}
	}
	return NULL;
}

t_archivo_fcb* obtener_archivo(char* nombre_archivo){
	for(int i = 0; i < list_size(lista_fcbs); i++){
		t_archivo_fcb* archivo_buscado = list_get(lista_fcbs, i);

		if(strcmp(archivo_buscado->nombre, nombre_archivo) == 0){
			return archivo_buscado;
		}
	}
	return NULL;
}

void asignar_bloque_primer_truncate(t_archivo_fcb* fcb){
	t_bloque_fat* bloque_inicial = buscar_bloque_libre();
	char* text_bloque_inicial = malloc(10);
	sprintf(text_bloque_inicial, "%d", bloque_inicial->id_bloque);
	config_set_value(fcb->archivo_fcb, "BLOQUE_INICIAL", text_bloque_inicial);
	config_save(fcb->archivo_fcb);
}

t_bloque_fat* buscar_bloque_libre(){ //busca un bloque libre y lo ocupa
	for(int i = 0; i < (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP); i++){
		t_bloque_fat* bloque = list_get(tabla_fat, i);
		if(bloque->esta_libre){
			bloque->puntero_siguiente = UINT32_MAX;
			bloque->esta_libre = 0;
			list_replace(tabla_fat, bloque->id_bloque, bloque);
			//TODO: modificar archivo fat, el TODO va aca?

			log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: %d - Valor: %d", i, UINT32_MAX);
			return bloque;
		}
	}

	log_error(filesystem_logger, "No hay bloques libres");
	return NULL;
}
