#include "../include/fs_kernel.h"

// =========== ATENDER KERNEL ===============


void atender_f_open_de_kernel(t_buffer* un_buffer){
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);
	char* operacion = recibir_string_del_buffer(un_buffer);

	t_paquete* paquete = crear_super_paquete(RESPUESTA_F_OPEN_FK);
	cargar_string_al_super_paquete(paquete, operacion);

	t_fcb* fcb = obtener_fcb(nombre_archivo);

	if(strcmp(operacion , "ABRIR_ARCHIVO") == 0){
		if(fcb != NULL){
			log_info(filesystem_log_obligatorio, "Abrir Archivo: %s", nombre_archivo);
			cargar_int_al_super_paquete(paquete, 1);
			cargar_string_al_super_paquete(paquete, nombre_archivo);
			cargar_int_al_super_paquete(paquete, fcb->tamanio);
		}else{
			cargar_int_al_super_paquete(paquete, -1);
		}
	}else{
		// Este seria el caso de CREAR_ARCHIVO
		crear_fcb(nombre_archivo);
		log_info(filesystem_log_obligatorio, "Crear Archivo: %s", nombre_archivo);
		cargar_int_al_super_paquete(paquete, 0);
	}
	enviar_rta_f_open_a_kernel(paquete);
	free(un_buffer);
}

static void destruir_bloques_libres(t_list* lista_bloques_libres_asignadose){
	void __eliminar_nodo_recurso(uint32_t* un_valor){
		free(un_valor);
	}
	list_destroy_and_destroy_elements(lista_bloques_libres_asignadose, (void*)__eliminar_nodo_recurso);
}

void atender_f_truncate_de_kernel(t_buffer* un_buffer){
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);
	int tamanio_nuevo = recibir_int_del_buffer(un_buffer);
	int pid_process = recibir_int_del_buffer(un_buffer);

	t_fcb* fcb = obtener_fcb(nombre_archivo);
	int tamanio_viejo = fcb->tamanio;
	fcb->tamanio = tamanio_nuevo;

	setear_size_de_una_fcb(fcb, tamanio_nuevo);

	int cantidad_bloques = fcb->tamanio / TAM_BLOQUE;



	char* bloque_inicial = malloc(10);
	sprintf(bloque_inicial, "%d", fcb->bloque_inicial);

	//int diferencia_tamanio = 0;
	int cantidad_bloques_viejos = tamanio_viejo / TAM_BLOQUE;
	int cantidad_bloques_nuevos = tamanio_nuevo / TAM_BLOQUE;

	t_list* lista_bloques_libres_asignados;

	if(tamanio_nuevo > tamanio_viejo){
		if(tamanio_viejo == 0){
			lista_bloques_libres_asignados = obtener_n_cantidad_de_bloques_libres_de_tabla_fat(cantidad_bloques);
			cargar_secuencia_de_bloques_asignados_a_tabla_fat(lista_bloques_libres_asignados);
			fcb->bloque_inicial = *((int*)list_get(lista_bloques_libres_asignados, 0));
			config_set_value(fcb->archivo_fcb, "BLOQUE_INICIAL", bloque_inicial);
			config_save(fcb->archivo_fcb);

		}else if(cantidad_bloques_viejos != cantidad_bloques_nuevos){
			int agrear_nro_bloques = cantidad_bloques_nuevos - cantidad_bloques_viejos;
			asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat(fcb->bloque_inicial, agrear_nro_bloques);
		}
	}else{
		//diferencia_tamanio = tamanio_viejo - tamanio_nuevo;
		if(cantidad_bloques_viejos != cantidad_bloques_nuevos){
			int reducir_nro_bloques = cantidad_bloques_viejos - cantidad_bloques_nuevos;
			reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat(fcb->bloque_inicial, reducir_nro_bloques);
		}
	}

	log_info(filesystem_log_obligatorio, "Truncar Archivo: <%s> - Tamaño: <%d>", nombre_archivo, tamanio_nuevo);
	enviar_rta_f_truncate_a_kernel(pid_process);
	free(un_buffer);
	if(!list_is_empty(lista_bloques_libres_asignados)){
		destruir_bloques_libres(lista_bloques_libres_asignados);
	}
}

void atender_f_read_de_kernel(t_buffer* un_buffer){
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);
	int pid_process = recibir_int_del_buffer(un_buffer);
	int puntero = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);

	t_fcb* fcb = obtener_fcb(nombre_archivo);

	uint32_t bloque_a_leer_fat = obtener_el_nro_bloque_segun_el_la_posicion_del_seek(fcb->bloque_inicial, puntero);
	void* bloque_a_leer = obtener_bloque(nombre_archivo, bloque_a_leer_fat);

	// Se supone que si usamos mmap ya me da el bloque y puedo mandarselo a memoria.
	enviar_contenido_a_memoria(pid_process,dir_fisica, bloque_a_leer);

	log_info(filesystem_log_obligatorio, "Leer Archivo: <%s> - Puntero: <%d> - Memoria: <%d>", nombre_archivo, puntero, dir_fisica);

	// Ahora queda esperar la respuesta de memoria en atender_memoria
	free(un_buffer);
}

void atender_f_write_de_kernel(t_buffer* un_buffer){
//	[PID][DIR_FISICA][NRO_BLOQUE]
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);
	int pid_process = recibir_int_del_buffer(un_buffer);
	int puntero = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);

	t_fcb* fcb = obtener_fcb(nombre_archivo);

	uint32_t bloque_a_escribir_fat = obtener_el_nro_bloque_segun_el_la_posicion_del_seek(fcb->bloque_inicial, puntero);

	log_info(filesystem_logger, "Bloque a escribir fat: <%u>", bloque_a_escribir_fat);

	enviar_solicitud_de_escritura_a_memoria(pid_process, dir_fisica, bloque_a_escribir_fat, nombre_archivo);

	log_info(filesystem_log_obligatorio, "Escribir Archivo: <%s> - Puntero: <%d> - Memoria: <%d>", nombre_archivo, puntero, dir_fisica);

	// Ahora queda esperar la respuesta de memoria en atender_memoria
	// y desde fs_memoria se sigue con el procedimiento
	free(un_buffer);
}



// =========== ENVIAR A KERNEL ===============

void enviar_rta_f_open_a_kernel(t_paquete* un_paquete){
	// Se carga todo en la funcion de f_open porque tiene distintos criterios para hacerlo
	enviar_paquete(un_paquete, fd_kernel);
}

void enviar_rta_f_truncate_a_kernel(int pid_process){
	t_paquete* paquete = crear_super_paquete(RESPUESTA_F_TRUNCATE_FK);
	cargar_string_al_super_paquete(paquete, "Truncate realizado");
	cargar_int_al_super_paquete(paquete, pid_process);
	enviar_paquete(paquete, fd_kernel);
	eliminar_paquete(paquete);
}


// =========== ENVIAR A MEMORIA ===============

void enviar_contenido_a_memoria(int pid_process ,int dir_fisica, void* contenido_leido){
	t_paquete* paquete = crear_super_paquete(CARGAR_INFO_DE_LECTURA_FM);
	cargar_int_al_super_paquete(paquete,pid_process);
	cargar_int_al_super_paquete(paquete,dir_fisica);
	cargar_choclo_al_super_paquete(paquete, contenido_leido, TAM_BLOQUE);
	enviar_paquete(paquete, fd_memoria);
	eliminar_paquete(paquete);
}

void enviar_solicitud_de_escritura_a_memoria(int pid_process, int dir_fisica, uint32_t bloque_a_escribir_fat, char* nombre_archivo){
	t_paquete* paquete = crear_super_paquete(BLOQUE_DE_MEMORIA_A_FILESYSTEM_FM);
	cargar_int_al_super_paquete(paquete,pid_process);
	cargar_int_al_super_paquete(paquete,dir_fisica);
	cargar_int_al_super_paquete(paquete, (int)bloque_a_escribir_fat);
	cargar_string_al_super_paquete(paquete, nombre_archivo);
	enviar_paquete(paquete, fd_memoria);
	eliminar_paquete(paquete);
}


