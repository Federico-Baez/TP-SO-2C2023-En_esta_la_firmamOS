/*
 * pagina.c
 *
 *  Created on: Oct 28, 2023
 *      Author: utnso
 */
#include "../include/pagina.h"

tabla_paginas* crear_tabla_paginas(int pid) {
    tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
    log_debug(memoria_logger, "[PAG]: Creo tabla de paginas PID %d", pid);

    nueva_tabla->pid = pid;
    nueva_tabla->paginas = list_create();
    pthread_mutex_init(&(nueva_tabla->mutex),NULL);
    char* spid = string_itoa(pid);
    free(spid);
    bloquear_lista_tablas();
    dictionary_put(tablas, spid, nueva_tabla);
    desbloquear_lista_tablas();


    return nueva_tabla;
}

int liberar_pagina(Pagina* una_pagina, int offset, int faltante){
	if(offset + faltante <= TAM_PAGINA){
		una_pagina->tamanio_ocupado -= faltante;
		return faltante;
	}
	una_pagina->tamanio_ocupado -= TAM_PAGINA- offset;
	return TAM_PAGINA- offset;
}

void liberar_paginas(tabla_paginas* una_tabla, int  dirLogica, int tamanio, int pid){
	/*****************PAGINACION SIMPLE**************************/
	int faltante = tamanio;
	int num_pagina = dirLogica/TAM_PAGINA;
	int offset = dirLogica%TAM_PAGINA;
	Pagina* una_pagina;

	t_list* pagina_a_eliminar;
	pagina_a_eliminar = list_create();

	if(offset>0){
		/*****desplazamiento es mayor a 0*****/
//		una_pagina = get_pagina(una_tabla->paginas,pid,num_pagina,false);
		faltante -=liberar_pagina(una_pagina,offset,faltante);

		if(una_pagina->tamanio_ocupado<=0){
			list_add(pagina_a_eliminar,(void*)num_pagina);
		}else{
			desbloquear_pagina(una_pagina);
		}
		num_pagina++;
	}
	void* un_num;
	eliminar_pagina(una_tabla,un_num);
	list_iterate(pagina_a_eliminar,eliminar_pagina);
	list_destroy(pagina_a_eliminar);
}

marco* crear_marco(int base, bool presente){
	marco *nuevo_marco = malloc(sizeof(marco));
	nuevo_marco->base = base;
	nuevo_marco->libre = presente;
	pthread_mutex_init(&(nuevo_marco->mutex), NULL);
	return nuevo_marco;
}

void eliminar_pagina(tabla_paginas* tabla, void* un_num){
	int num = (int) un_num;
	Pagina* una_pag = list_get(tabla->paginas, num);
	if(una_pag->presente){
		una_pag->ptr_marco->libre = true;
		una_pag->ptr_marco->nro_pagina = 0;
		una_pag->ptr_marco->pid = 0;
		una_pag->presente = false;
		una_pag->ptr_marco = NULL;
	}
	log_info(memoria_logger,"ELimino pagina");
	desbloquear_pagina(una_pag);
}



Pagina* get_pagina(t_list* pagina, int pid, int num_pagina){

}



/************************SEMAFOROS*******************************/
void bloquear_pagina(Pagina* pagina){
	pthread_mutex_lock(&(pagina->mutex));
	log_info(memoria_logger, "Bloqueo la pagina de tamanio ocupado%d\n",pagina->tamanio_ocupado);
}
void desbloquear_pagina(Pagina* pagina){
	log_info(memoria_logger, "Desbloqueo la pagina de tamanio ocupado%d\n",pagina->tamanio_ocupado);
	pthread_mutex_unlock(&(pagina->mutex));
}
