#include "../include/fcb.h"

void crear_fcb(char* nombre_archivo){
	log_warning(filesystem_logger, "inicio crear_fcb");
	char* punto_fcb = ".fcb";
	log_info(filesystem_logger, "AA_1");
	char* path_archivo = malloc(strlen(PATH_FCB) + strlen(nombre_archivo) + strlen(punto_fcb) + 1);
	log_info(filesystem_logger, "AA_2");
	strcpy(path_archivo, PATH_FCB);
	strcat(path_archivo, nombre_archivo);
	strcat(path_archivo, punto_fcb);
	log_info(filesystem_logger, "AA_3");

	//Creo el FCB y lo agrego a la lista de structs
	t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));
	log_info(filesystem_logger, "AA_4");

	nuevo_fcb->nombre = malloc(strlen(nombre_archivo) + 1);
	log_info(filesystem_logger, "AA_5");
	strcpy(nuevo_fcb->nombre, nombre_archivo);
	nuevo_fcb->tamanio = 0;
	char* text_tamanio_archivo = malloc(10);
	log_info(filesystem_logger, "AA_6");
	sprintf(text_tamanio_archivo, "%d", nuevo_fcb->tamanio);

	log_info(filesystem_logger, "AA_7");

	list_add(lista_struct_fcbs, nuevo_fcb);
	log_info(filesystem_logger, "AA_8");

	nuevo_fcb->archivo_fcb = config_create(path_archivo);

	if(nuevo_fcb->archivo_fcb == NULL){
		log_error(filesystem_logger, "No se encontro el path del config\n");
//		config_destroy(nuevo_fcb->archivo_fcb);
		log_info(filesystem_logger, "AA_9");

		FILE* file_fcb = fopen(path_archivo, "a+");
		fclose(file_fcb);
		log_info(filesystem_logger, "AA_10");

		nuevo_fcb->archivo_fcb = config_create(path_archivo);


	}
	log_info(filesystem_logger, "AA_11");

	config_set_value(nuevo_fcb->archivo_fcb, "NOMBRE_ARCHIVO", nuevo_fcb->nombre);
	log_info(filesystem_logger, "AA_12");
	config_set_value(nuevo_fcb->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);
	log_info(filesystem_logger, "AA_13");
	config_set_value(nuevo_fcb->archivo_fcb, "BLOQUE_INICIAL", "NULL");

	log_info(filesystem_logger, "AA_14 Save");
	config_save(nuevo_fcb->archivo_fcb);
	log_warning(filesystem_logger, "fin crear_fcb");
}

t_fcb* obtener_fcb(char* nombre_archivo){
	log_warning(filesystem_logger, "inicio obtener_fcb");
	if(!list_is_empty(lista_struct_fcbs)){
		for(int i = 0; i < list_size(lista_struct_fcbs); i++){
			t_fcb* fcb_buscado = list_get(lista_struct_fcbs, i);
			if(strcmp(fcb_buscado->nombre, nombre_archivo) == 0){
				return fcb_buscado;
			}
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

	for(int i = 0; i < list_size(lista_struct_fcbs); i++){
		t_fcb* archivo_buscado = list_get(lista_struct_fcbs, i);
		if(strcmp(archivo_buscado->nombre, una_fcb->nombre) == 0){
			config_set_value(archivo_buscado->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);
			config_save(archivo_buscado->archivo_fcb);
		}
	}
	log_warning(filesystem_logger, "fin setear_size_de_una_fcb");
}

//TODO: fletar de Filesystem.c buscar_fcb, obtener_archivo, logica de ejecutar_f_create
