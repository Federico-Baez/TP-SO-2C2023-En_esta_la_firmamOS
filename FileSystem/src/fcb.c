#include "../include/fcb.h"

void crear_fcb(char* nombre_archivo){
	char* punto_fcb = ".fcb";
	char* path_archivo = malloc(strlen(PATH_FCB) + strlen(nombre_archivo) + strlen(punto_fcb) + 1);
	strcpy(path_archivo, PATH_FCB);
	strcat(path_archivo, nombre_archivo);
	strcat(path_archivo, punto_fcb);

	//Creo el FCB y lo agrego a la lista de structs
	t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));

	nuevo_fcb->nombre = malloc(strlen(nombre_archivo) + 1);
	strcpy(nuevo_fcb->nombre, nombre_archivo);
	nuevo_fcb->tamanio = 0;
	char* text_tamanio_archivo = malloc(10);
	sprintf(text_tamanio_archivo, "%d", nuevo_fcb->tamanio);


	list_add(lista_struct_fcbs, nuevo_fcb);

	nuevo_fcb->archivo_fcb = config_create(path_archivo);

	if(nuevo_fcb->archivo_fcb == NULL){
		FILE* file_fcb = fopen(path_archivo, "a+");
		fclose(file_fcb);

		nuevo_fcb->archivo_fcb = config_create(path_archivo);
	}

	config_set_value(nuevo_fcb->archivo_fcb, "NOMBRE_ARCHIVO", nuevo_fcb->nombre);
	config_set_value(nuevo_fcb->archivo_fcb, "TAMANIO_ARCHIVO", text_tamanio_archivo);
	config_set_value(nuevo_fcb->archivo_fcb, "BLOQUE_INICIAL", "NULL");

	config_save(nuevo_fcb->archivo_fcb);
}

t_fcb* obtener_fcb(char* nombre_archivo){
	if(!list_is_empty(lista_struct_fcbs)){
		for(int i = 0; i < list_size(lista_struct_fcbs); i++){
			t_fcb* fcb_buscado = list_get(lista_struct_fcbs, i);
			if(strcmp(fcb_buscado->nombre, nombre_archivo) == 0){
				return fcb_buscado;
			}
		}
	}
	return NULL;
}

void setear_size_de_una_fcb(t_fcb* una_fcb, int nuevo_size){
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
}

//TODO: fletar de Filesystem.c buscar_fcb, obtener_archivo, logica de ejecutar_f_create
