#include "../include/marcos.h"

t_marco* crear_marco(int base, bool libre, int index){
	t_marco *nuevo_marco = malloc(sizeof(t_marco));
	nuevo_marco->nro_marco = index;
	nuevo_marco->base = base;
	nuevo_marco->libre = libre;
	nuevo_marco->info_new = NULL;
	nuevo_marco->info_old = NULL;

	nuevo_marco->orden_carga = 0;
	nuevo_marco->ultimo_uso = NULL;

	return nuevo_marco;
}

void liberar_marco(t_marco* un_marco){
	un_marco->libre = true;
	un_marco->orden_carga = 0;
	if(un_marco->info_new != NULL){
		free(un_marco->info_new);
		un_marco->info_new = NULL;
	}
	if(un_marco->info_old != NULL){
		free(un_marco->info_old);
		un_marco->info_old = NULL;
	}
	if(un_marco->ultimo_uso != NULL){
		temporal_destroy(un_marco->ultimo_uso);
		un_marco->ultimo_uso = NULL;
	}
}

t_marco* obtener_marco_por_nro_marco(int nro_marco){
	t_marco* un_marco;
//	pthread_mutex_lock(&mutex_lst_marco);
	un_marco = list_get(lst_marco, nro_marco);
//	pthread_mutex_unlock(&mutex_lst_marco);

	return un_marco;
}

void destruir_list_marcos_y_todos_los_marcos(){
	void _destruir_un_marco(t_marco* un_marco){
		if(un_marco->info_new != NULL){
			free(un_marco->info_new);
		}
		if(un_marco->info_old != NULL){
			free(un_marco->info_old);
		}
		if(un_marco->ultimo_uso != NULL){
			temporal_destroy(un_marco->ultimo_uso);
		}
		free(un_marco);
	}
	list_destroy_and_destroy_elements(lst_marco, (void*)_destruir_un_marco);
}


/*Devuelve:
 * false: marco limpio sin asignar de la lista
 * true: marco victima*/
t_marco* obtener_un_marco_de_la_lista_de_marcos(int* tipo_de_marco){
	t_marco* un_marco = NULL;

	bool _marco_libre(t_marco* un_marco){
		return un_marco->libre;
	}
//	pthread_mutex_lock(&mutex_lst_marco);
	un_marco = list_find(lst_marco, (void*)_marco_libre);

	//Asumiendo que retorne NULL por no encontrar marcos vacios disponibles
	if(un_marco == NULL){
		un_marco = elegir_victima_segun_algoritmo();
		*tipo_de_marco = MARCO_VICTIMA;

		//Si es pagina victima, preguntar si la pagina correspondiente tiene bit de modificado y actuar en conescueencia
		evaluar_si_esta_o_no_el_bit_modificado_del_marco(un_marco);

	}
	else{
		*tipo_de_marco = MARCO_LIMPIO;
	}
	un_marco->libre = false;
//	pthread_mutex_unlock(&mutex_lst_marco);

	return un_marco;
}

/*Si llamaas esta funcion es que todos los marcos estan ocupados (asignados a alguna pagina)*/
t_marco* elegir_victima_segun_algoritmo(){
	t_marco* un_marco;

	t_marco* _comparar_orden_carga(t_marco* marco1, t_marco* marco2) {
		if (marco1->orden_carga < marco2->orden_carga){
			if(marco1->orden_carga == 0 || marco2->orden_carga == 0){
				log_error(memoria_logger, "SE SUPONE QUE TODAS LOS MARCOS ESTAN OCUPADOS Y NINGUNO DEBERIA ESTAR SETEADO CON ORDEN DE CARGA = 0");
				exit(EXIT_FAILURE);
			}
			return marco1;
		}
		else return marco2;
	}

	t_marco* _comparar_acceso_LRU(t_marco* marco1, t_marco* marco2) {
		if (temporal_gettime(marco1->ultimo_uso) > temporal_gettime(marco2->ultimo_uso)){
			return marco1;
		}
		else return marco2;
	}

	if(strcmp(ALGORITMO_REEMPLAZO, "FIFO")){
		un_marco = list_get_minimum(lst_marco, (void*)_comparar_orden_carga);
	}else if(strcmp(ALGORITMO_REEMPLAZO, "LRU")){
		un_marco = list_get_maximum(lst_marco, (void*)_comparar_acceso_LRU);
	}else{
		log_error(memoria_logger, "Algoritmo de Reemplazo NO VALIDO");
		exit(EXIT_FAILURE);
	}

	return un_marco;
}


void evaluar_si_esta_o_no_el_bit_modificado_del_marco(t_marco* un_marco){
	t_pagina* una_pagina = pag_obtener_pagina_completa(un_marco->info_new->proceso, un_marco->info_new->nro_pagina);
	if(una_pagina->modificado){
		//Guardar pagina en FS
		user_escribir_pagina_en_swap(un_marco);
//		user_escribir_pagina_en_swap(un_marco->base, una_pagina->pos_en_swap);

		//Log obligatorio por escritura de pagina en swap
		logg_escritura_pagina_en_swap(un_marco->info_new->proceso->pid, un_marco->nro_marco, un_marco->info_new->nro_pagina);

		//Setear pagina con bit de modificado 0
		una_pagina->modificado = false;
	}

	//Setear pagina con bit de presencia 0
	una_pagina->presente = false;
}

void user_escribir_pagina_en_swap(t_marco* un_marco){
	//Se copia el marco correspondiente
	void* pagina_copy = copiar_marco_desde_una_dir_fisica(un_marco->info_new->proceso->pid, un_marco->base);

	int pos_swap = pag_obtener_pos_en_swap(un_marco->info_new->proceso, un_marco->info_new->nro_pagina);
	//Se envia a FS
	evniar_pagina_a_fs_area_swap(pos_swap, pagina_copy);
	free(pagina_copy);
}


// ========================================



