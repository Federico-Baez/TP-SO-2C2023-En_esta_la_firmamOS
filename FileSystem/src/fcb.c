#include "../include/fcb.h"

void crear_fcb(char* nombre_archivo){
	log_warning(filesystem_logger, "inicio crear_fcb");
	char* path_archivo = malloc(strlen(PATH_FCB) + strlen(nombre_archivo));
	strcpy(path_archivo, PATH_FCB);
	strcat(path_archivo, nombre_archivo);

	//Creo el FCB y lo agrego a la lista de structs
	t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));
	nuevo_fcb->nombre = malloc(strlen(nombre_archivo));
	strcpy(nuevo_fcb->nombre, nombre_archivo);
	nuevo_fcb->tamanio = 0;
	char* text_tamanio_archivo = malloc(10);
	sprintf(text_tamanio_archivo, "%d", nuevo_fcb->tamanio);

	list_add(lista_struct_fcbs, nuevo_fcb);

	//si hago config_create sin hacer esto, me devuelve NULL
	FILE* file_fcb = fopen(path_archivo, "a+");

	//Creo el config correspondiente al FCB y lo agrego a la lista de configs
	t_archivo_fcb* archivo_fcb = malloc(sizeof(t_archivo_fcb));
	archivo_fcb->nombre = malloc(strlen(nombre_archivo));
	strcpy(archivo_fcb->nombre, nombre_archivo);
	archivo_fcb->archivo_fcb = config_create(path_archivo);

	config_set_value(archivo_fcb->archivo_fcb, "NOMBRE_ARCHIVO", nuevo_fcb->nombre);
	config_set_value(archivo_fcb->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);

	config_save(archivo_fcb->archivo_fcb);
	list_add(lista_configs_fcbs, archivo_fcb);

	fclose(file_fcb);
	log_warning(filesystem_logger, "fin crear_fcb");
}

t_fcb* obtener_fcb(char* nombre_archivo){
	log_warning(filesystem_logger, "inicio obtener_fcb");
	for(int i = 0; i < list_size(lista_struct_fcbs); i++){
		t_fcb* fcb_buscado = list_get(lista_struct_fcbs, i);

		if(strcmp(fcb_buscado->nombre, nombre_archivo) == 0){
			log_warning(filesystem_logger, "fin obtener_fcb");
			return fcb_buscado;
		}
	}
	log_warning(filesystem_logger, "fin obtener_fcb");
	return NULL;
}

void setear_size_de_una_fcb(t_fcb* una_fcb, int nuevo_size){
	log_warning(filesystem_logger, "inicio setear_size_de_una_fcb");
	//nomrbe_De_archivo.fcb <- Setear en el archivo concreto
	//Se puede setear el size de la lista de fcbs aca o afuera en la logica general

	char* text_tamanio_archivo = malloc(10);
	sprintf(text_tamanio_archivo, "%d", nuevo_size);

	for(int i = 0; i < list_size(lista_configs_fcbs); i++){
		t_archivo_fcb* archivo_buscado = list_get(lista_configs_fcbs, i);

		if(strcmp(archivo_buscado->nombre, una_fcb->nombre) == 0){
			config_set_value(archivo_buscado->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);
			config_save(archivo_buscado->archivo_fcb);
		}
	}
	log_warning(filesystem_logger, "fin setear_size_de_una_fcb");
}

//TODO: fletar de Filesystem.c buscar_fcb, obtener_archivo, logica de ejecutar_f_create
