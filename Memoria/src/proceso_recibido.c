#include "../include/proceso_recibido.h"

void agregar_proceso_a_listado(t_buffer* unBuffer, t_list* lst_procesos_recibido){
	proceso_recibido* un_proceso = malloc(sizeof(proceso_recibido));
	if (un_proceso == NULL) {
	        perror("Error al reservar memoria para el proceso");
	        exit(EXIT_FAILURE);
	}
	un_proceso->pathInstrucciones = recibir_string_del_buffer(unBuffer);
	un_proceso->size = recibir_int_del_buffer(unBuffer);
	un_proceso->pid =recibir_int_del_buffer(unBuffer);
	un_proceso->instrucciones= leer_archivo_y_cargar_instrucciones(un_proceso->pathInstrucciones);
	un_proceso->tabla_paginas = list_create();
	log_info(memoria_logger, "Recibi el proceso con los siguientes datos archivo: %s, el tamanio: %d y el pid: %d ",un_proceso->pathInstrucciones, un_proceso->size, un_proceso->pid);
	list_add(lst_procesos_recibido, un_proceso);

	//Logica para descubrir la cantidad de paginas necesarias
	int cantidad_paginas_necesarias = (int)ceil(un_proceso->size / TAM_PAGINA);

	//Luego agrego una Pagina a la lista de TABLA Paginas segun la cantidad
	for(int i=0; i<cantidad_paginas_necesarias; i++){
		Pagina* una_pagina = malloc(sizeof(Pagina));
		una_pagina->nro_pagina = i;
		una_pagina->pid_proceso = un_proceso->pid;
		list_add(un_proceso->tabla_paginas, una_pagina);
	}

	// Asignarle la data a cada Pagina
	asignar_marcos_a_cada_pagina(un_proceso);

	//Avisarle a FILESYSTEM que separa cierta cantidad de bloques
	solicitar_asignacion_bloques_SWAP_a_FS(un_proceso->pid, cantidad_paginas_necesarias);

	handhsake_modules(fd_kernel,"[MEMORIA]>Proceso cargado en Memoria OK");
}

void liberar_proceso(proceso_recibido* proceso) {
	t_paquete* un_paquete = crear_super_paquete(LIBERAR_PAGINAS_FM);
	int cant_paginas = list_size(proceso->tabla_paginas);
	cargar_int_al_super_paquete(un_paquete, cant_paginas);

    for (int i = 0; i < list_size(proceso->instrucciones); i++) {
        char* instruccion = list_get(proceso->instrucciones, i);
        free(instruccion);
    }
    list_destroy(proceso->instrucciones);
    free(proceso->pathInstrucciones);

	pthread_mutex_lock(&mutex_lst_marco);
    for(int i=0; i<cant_paginas; i++){
    	Pagina* una_pagina = list_get(proceso->instrucciones, i);
    	una_pagina->ptr_marco->libre = true;
    	cargar_int_al_super_paquete(un_paquete, una_pagina->pos_en_swap);
    	free(una_pagina);
    }
	pthread_mutex_unlock(&mutex_lst_marco);
    list_destroy(proceso->tabla_paginas);

    //Enviar Lista de bloques a FS para que marque como libre a ciertos bloques
    enviar_paquete(un_paquete, fd_filesystem);
    eliminar_paquete(un_paquete);

    free(proceso);
}

void liberar_listado_procesos(t_list* lst_procesos) {
    for (int i = 0; i < list_size(lst_procesos); i++) {
        proceso_recibido* proceso = list_get(lst_procesos, i);
        liberar_proceso(proceso);
    }
    list_destroy(lst_procesos);
}

proceso_recibido* obtener_proceso_por_id(int pid, t_list* lst_procesos){
	bool buscar_el_pid(void* proceso){
		return ((proceso_recibido*)proceso)->pid == pid;
	}
	proceso_recibido* un_proceso = list_find(lst_procesos, buscar_el_pid);
	return un_proceso;
}



t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo) {
    FILE* archivo = fopen(path_archivo, "rt");
    t_list* instrucciones = list_create();
    char* instruccion_formateada = NULL;
    int i = 0;

    if (archivo == NULL) {
        perror("No se encontró el archivo");
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(char));
    while (fgets(linea_instruccion, 256, archivo)) {
    	//Comprobar si el ultimo caracter del string capturado tiene un salto delinea
    	//Si lo tiene hay que sacarlo
    	//[0][1][2][3][4]["\n"]["\0"] -> Size:6
    	int size_linea_actual = strlen(linea_instruccion);
    	if(size_linea_actual > 2){
    		if(linea_instruccion[size_linea_actual - 1] == '\n'){
				char* linea_limpia = string_new();
				string_n_append(&linea_limpia, linea_instruccion, size_linea_actual - 1);
				free(linea_instruccion);
				linea_instruccion = linea_limpia;
    		}
    	}
    	//-----------------------------------------------

        char** l_instrucciones = string_split(linea_instruccion, " ");

        log_info(memoria_logger, "Intruccion: [%s]", linea_instruccion);

        while (l_instrucciones[i]) {
            i++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->pseudo_c = strdup(l_instrucciones[0]);
        pseudo_cod->fst_param = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->snd_param = (i > 2) ? strdup(l_instrucciones[2]) : NULL;

        if (i == 3) {
            instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param);
        } else {
            instruccion_formateada = strdup(pseudo_cod->pseudo_c);
        }

//        log_info(memoria_logger, "Se carga la instrucción [%d] %s", (int)strlen(instruccion_formateada),instruccion_formateada);
        list_add(instrucciones, instruccion_formateada);

        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }
        free(l_instrucciones);
        free(pseudo_cod->pseudo_c);
		if(pseudo_cod->fst_param) free(pseudo_cod->fst_param);
		if(pseudo_cod->snd_param) free(pseudo_cod->snd_param);
		free(pseudo_cod);
        i = 0; // Restablece la cuenta para la próxima iteración
    }

    fclose(archivo);
    free(linea_instruccion);
    return instrucciones;
}

void liberar_memoria_de_instrucciones(t_list* instrucciones){
	list_destroy_and_destroy_elements(instrucciones, free);
}

char* obtener_instruccion_por_indice(int indice_instruccion, t_list* instrucciones){

	char* instruccion_actual;
	return (indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones))
			? instruccion_actual = list_get(instrucciones,indice_instruccion) : NULL;

}



void cargar_bloques_asignados_en_proceso(int pid, t_list* lista_de_bloques_asignados){
	//
	proceso_recibido* un_proceso = obtener_proceso_por_id(pid, list_procss_recibidos);
	int cant_bloques = list_size(lista_de_bloques_asignados);

	//Comprobando que el tamaño de La lista de Marcos sea la misma que el de la tabla de paginas
	if(cant_bloques == list_size(un_proceso->tabla_paginas)){

		//Itermaos para agregar la info de la lista de marcos a la tabla de paginas
		for(int i=0; i<cant_bloques; i++){
			Pagina* n_pagina = list_get(un_proceso->tabla_paginas, i);
			int* n_bloque = list_get(lista_de_bloques_asignados, i);
			n_pagina->pos_en_swap = *n_bloque;
		}
	}
	else {
		log_info(memoria_logger, "No coinciden la cantidad de marcos con la tabla de paginas");
		exit(EXIT_FAILURE);
	}
}
















