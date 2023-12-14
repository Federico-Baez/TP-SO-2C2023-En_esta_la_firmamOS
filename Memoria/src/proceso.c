#include "../include/proceso.h"


t_proceso* crear_proceso(int pid, int size, char* path_instruc){
	t_proceso* proceso_nuevo = malloc(sizeof(t_proceso));
	proceso_nuevo->pid = pid;
	proceso_nuevo->size = size;
	proceso_nuevo->pathInstrucciones = path_instruc;
	proceso_nuevo->instrucciones = NULL;
	proceso_nuevo->tabla_paginas = list_create();
	pthread_mutex_init(&(proceso_nuevo->mutex_TP), NULL);

	//Cargando instrucciones
	proceso_nuevo->instrucciones = leer_archivo_y_cargar_instrucciones(proceso_nuevo->pathInstrucciones);

	//Creando Paginas necesarias y Asignar marcos
//	crear_paginas_y_asignar_marcos_para_un_proceso_nuevo(proceso_nuevo);

	//Solo crear paginas y crear tabla de paginas
	solo_crear_y_setear_tabla_de_paginas(proceso_nuevo);

	//Logg Obligatorio
	logg_crear_tabla_de_paginas(proceso_nuevo->pid, list_size(proceso_nuevo->tabla_paginas));

	return proceso_nuevo;
}

void eliminar_proceso(t_proceso* un_proceso){
	free(un_proceso->pathInstrucciones);

	//Eliminar lista de instrucciones
	eliminar_lista_de_instrucciones(un_proceso->instrucciones);

	//Avisa a FileSystem que libere las posiciones en swap correspondientes
	enviar_a_fs_orden_de_liberacion_de_posiciones_swap(un_proceso);

	//Eliminar Tabla de Paginas - y liberar marcos correspondientes
	eliminar_tabla_de_paginas(un_proceso);

	//Elminar MUTEX
	pthread_mutex_destroy(&(un_proceso->mutex_TP));

	free(un_proceso);
}

void eliminar_lista_de_instrucciones(t_list* lista_instrucciones){
	void _destruis_instruccion(char* una_instruccion){
		free(una_instruccion);
	}
	list_destroy_and_destroy_elements(lista_instrucciones, (void*)_destruis_instruccion);
}

void eliminar_tabla_de_paginas(t_proceso* un_proceso){
	int cant_paginas = list_size(un_proceso->tabla_paginas);
	void _eliminar_pagina_y_liberar_marco(t_pagina* una_pagina){
		//Marcar como libre el marco correspondiente
		if(una_pagina->presente){
			t_marco* un_marco = obtener_marco_por_nro_marco(una_pagina->nro_marco);
			liberar_marco(un_marco);

		}
		free(una_pagina);
	}
	list_destroy_and_destroy_elements(un_proceso->tabla_paginas, (void*)_eliminar_pagina_y_liberar_marco);

	//Log obligatorio destruccion de tabla de paginas
	logg_destruir_tabla_de_paginas(un_proceso->pid, cant_paginas);
}

void crear_paginas_y_asignar_marcos_para_un_proceso_nuevo(t_proceso* un_proceso){
	int cantidad_paginas_necesarias = (int)ceil(un_proceso->size / TAM_PAGINA);
	for(int i=0; i<cantidad_paginas_necesarias; i++){
		t_pagina* una_pagina = malloc(sizeof(t_pagina));
		una_pagina->nro_pagina = i;
		una_pagina->presente = false;
		una_pagina->modificado = false;

		//Asignando marco a cada pagina
		int tipo_marco = 10;
		t_marco* un_marco = obtener_un_marco_de_la_lista_de_marcos(&tipo_marco);
		if(tipo_marco == MARCO_VICTIMA){
			if(un_marco->info_old != NULL){
				free(un_marco->info_old);
			}
			un_marco->info_old = un_marco->info_new;
		}
		un_marco->info_new = malloc(sizeof(frame_info));
		un_marco->info_new->proceso = un_proceso;
		un_marco->info_new->nro_pagina = una_pagina->nro_pagina;

		setear_config_del_marco_segun_algoritmo(un_marco);

		una_pagina->nro_marco = un_marco->nro_marco;

		list_add(un_proceso->tabla_paginas, una_pagina);

		//log obligatorio
		if(tipo_marco == MARCO_VICTIMA){
			logg_reemplazar_pagina(	un_marco->nro_marco,
								un_marco->info_old->proceso->pid,
								un_marco->info_old->nro_pagina,
								un_marco->info_new->proceso->pid,
								un_marco->info_new->nro_pagina);
		}

		//Control de ERROR
		if(tipo_marco >= 2){
			log_error(memoria_logger, "El tipo de marco no se setea Correctamente - Creacion de proceso");
			exit(EXIT_FAILURE);
		}
	}
}

void solo_crear_y_setear_tabla_de_paginas(t_proceso* un_proceso){
	int cantidad_paginas_necesarias = (int)ceil(un_proceso->size / TAM_PAGINA);
	for(int i=0; i<cantidad_paginas_necesarias; i++){
		t_pagina* una_pagina = malloc(sizeof(t_pagina));
		una_pagina->nro_pagina = i;
		una_pagina->presente = false;
		una_pagina->modificado = false;

		list_add(un_proceso->tabla_paginas, una_pagina);
	}



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

void agregar_proceso_a_listado(t_proceso* un_proceso ,t_list* una_lista) {
	list_add(una_lista, un_proceso);
}

void sacar_proceso_del_listado(t_proceso* un_proceso ,t_list* lst_procesos_recibido){
	if(list_remove_element(lst_procesos_recibido, un_proceso)){
		log_info(memoria_logger, "Proceso eliminado de la lista de procesos: <%d>", un_proceso->pid);
	}else{
		log_error(memoria_logger, "No se pudo eliminar de la lista el proceso: <%d>", un_proceso->pid);
		exit(EXIT_FAILURE);
	}
}


t_proceso* obtener_proceso_por_id(int pid){
	bool _buscar_el_pid(t_proceso* proceso){
		return proceso->pid == pid;
	}
	t_proceso* un_proceso = list_find(list_procss_recibidos, (void*)_buscar_el_pid);
	if(un_proceso == NULL){
		log_error(memoria_logger, "PID<%d> No encontrado en la lista de procesos", pid);
		exit(EXIT_FAILURE);
	}
	return un_proceso;
}


char* obtener_instruccion_por_indice(t_proceso* un_proceso, int indice_instruccion){
	char* instruccion_actual;
	if(indice_instruccion >= 0 && indice_instruccion < list_size(un_proceso->instrucciones)){
		instruccion_actual = list_get(un_proceso->instrucciones, indice_instruccion);
		return instruccion_actual;
	}
	else{
		log_error(memoria_logger, "PID<%d> - Nro de Instruccion <%d> NO VALIDA", un_proceso->pid, indice_instruccion);
		exit(EXIT_FAILURE);
		return NULL;
	}
}


/*La lista de pos_swap debe tener solo valores int*/
void cargar_bloques_asignados_en_proceso(t_proceso* un_proceso, t_list* lista_de_pos_swap){
	int cant_elementos = list_size(lista_de_pos_swap);
	for(int i=0; i<cant_elementos; i++){
		int* pos_swap = list_get(lista_de_pos_swap, i);
		t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, i);
		una_pagina->pos_en_swap = *pos_swap;
		//
//		log_warning(memoria_logger, "Seteado pos_swap: <PID:%d> <n_pag:%d> <pos_swap:%d>", un_proceso->pid, i, *pos_swap);
//		logg_acceso_a_tabla_de_paginas(un_proceso->pid, i, una_pagina->nro_marco);
	}
	log_warning(memoria_logger, "Se Cargaron pos_SWAP en proceso <PID:%d>", un_proceso->pid);
}


t_pagina* pag_obtener_pagina_completa(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	logg_acceso_a_tabla_de_paginas(un_proceso->pid, nro_pagina, una_pagina->nro_marco);
	return una_pagina;
}

int pag_obtener_nro_de_marco(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	logg_acceso_a_tabla_de_paginas(un_proceso->pid, nro_pagina, una_pagina->nro_marco);
	return una_pagina->nro_marco;
}

int pag_obtener_pos_en_swap(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	logg_acceso_a_tabla_de_paginas(un_proceso->pid, nro_pagina, una_pagina->nro_marco);
	return una_pagina->pos_en_swap;
}

bool pag_obtener_bit_de_PRESENCIA(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	logg_acceso_a_tabla_de_paginas(un_proceso->pid, nro_pagina, una_pagina->nro_marco);
	return una_pagina->presente;
}

bool pag_obtener_bit_de_MODIFICADO(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	logg_acceso_a_tabla_de_paginas(un_proceso->pid, nro_pagina, una_pagina->nro_marco);
	return una_pagina->modificado;
}












