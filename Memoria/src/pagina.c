/*
 * pagina.c
 *
 *  Created on: Oct 28, 2023
 *      Author: utnso
 */
#include "../include/pagina.h"

int contador_accesos = 0;    // Para LRU
//int ordenCargaGlobal = 0;    // Para FIFO - Fue puesto como GLOBAL en m_gestor.h
tabla_paginas* crear_tabla_paginas(int pid) {
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));

	    if(!nueva_tabla) {
	    	log_error(memoria_logger, "Error al crear la tabla de pagina\n");
	        // Error al asignar memoria, puedes agregar un mensaje de error si lo prefieres.
	        return NULL;
	    }

	    nueva_tabla->pid = pid;
	    nueva_tabla->paginas = list_create();

	    if(!nueva_tabla->paginas) {
	    	log_error(memoria_logger, "Error al crear la pagina\n");
	        // Error al crear la lista, puedes agregar un mensaje de error si lo prefieres.
	        free(nueva_tabla);
	        return NULL;
	    }

	    pthread_mutex_init(&(nueva_tabla->mutex), NULL);

	    return nueva_tabla;
}
void destruir_tabla_paginas(tabla_paginas* tabla) {
    list_destroy(tabla->paginas);
    pthread_mutex_destroy(&(tabla->mutex));
    free(tabla);
}

int liberar_pagina(Pagina* una_pagina, int offset, int faltante){
	if (!una_pagina || offset < 0 || faltante <= 0) {
	        return -1;  // Parámetros no válidos
	}
	if(offset + faltante <= TAM_PAGINA){
		una_pagina->tamanio_ocupado -= faltante;
		return faltante;
	}
	una_pagina->tamanio_ocupado -= TAM_PAGINA- offset;
	return TAM_PAGINA- offset;
}

void liberar_paginas(tabla_paginas* una_tabla, int  dirLogica, int tamanio, int pid){
//	/*****************PAGINACION SIMPLE**************************/

	t_list* pagina_a_eliminar = list_create();
	int faltante = tamanio;
	int num_pagina = dirLogica / TAM_PAGINA;
	int offset = dirLogica % TAM_PAGINA;

	while (faltante > 0) {
		Pagina* una_pagina = list_get(una_tabla->paginas, num_pagina);

		if (!una_pagina) break;

		int liberado = liberar_pagina(una_pagina, offset, faltante);
		faltante -= liberado;

		if (una_pagina->tamanio_ocupado == 0) {
			list_add(pagina_a_eliminar, una_pagina);
		}

		offset = 0;
		num_pagina++;
	}

	for (int i = 0; i < list_size(pagina_a_eliminar); i++) {
		Pagina* pagina_para_borrar = list_get(pagina_a_eliminar, i);
		list_remove_and_destroy_element(una_tabla->paginas, num_pagina, (void*)destruir_pagina);
		free(pagina_para_borrar);
	}

	list_destroy(pagina_a_eliminar);
}

void destruir_pagina(Pagina* pagina) {
    free(pagina);
}
marco* crear_marco(int base, bool presente, int index){
	marco *nuevo_marco = malloc(sizeof(marco));
	nuevo_marco->base = base;
	nuevo_marco->libre = presente;
	nuevo_marco->pid = index;
	pthread_mutex_init(&(nuevo_marco->mutex), NULL);
	return nuevo_marco;
}

void eliminar_pagina(tabla_paginas* tabla, int num) {
    // Asegúrate de que el índice 'num' es válido antes de obtener un elemento de la lista.
    if (num >= list_size(tabla->paginas)) {
        log_error(memoria_logger, "Error al eliminar: Índice de página %d es inválido.", num);
        return;
    }

    Pagina* una_pag = list_get(tabla->paginas, num);

    // Bloquea la página y el marco para modificarlos
    pthread_mutex_lock(&(una_pag->mutex));
    if (una_pag->ptr_marco) pthread_mutex_lock(&(una_pag->ptr_marco->mutex));

    if (una_pag->presente && una_pag->ptr_marco) {
        una_pag->ptr_marco->libre = true;
        una_pag->ptr_marco->nro_pagina = 0;
        una_pag->ptr_marco->pid = 0;
        una_pag->presente = false;
        una_pag->ptr_marco = NULL;
    }

    // Desbloquea después de modificar
    if (una_pag->ptr_marco) pthread_mutex_unlock(&(una_pag->ptr_marco->mutex));
    pthread_mutex_unlock(&(una_pag->mutex));

    log_info(memoria_logger, "Eliminada página %d.", num);
}


/*
 *
 * En caso de que la memoria principal se encuentre llena,
 * se deberá seleccionar una página víctima utilizando
 * el algoritmo de reemplazo. Si la víctima se encuentra modificada,
 * se deberá previamente escribir en SWAP.
 *
 */
void acceder_pagina(tabla_paginas* tabla, int numero_pagina) {
//    contador_accesos++;

    Pagina* pagina = list_get(tabla->paginas, numero_pagina);
    pthread_mutex_lock(&(pagina->mutex));

    if (strcmp(ALGORITMO_REEMPLAZO, "LRU") == 0) {
    	temporal_destroy(pagina->ultimo_uso);
        pagina->ultimo_uso = temporal_create();
    }

    pthread_mutex_unlock(&(pagina->mutex));

    // Aquí iría el código para leer o escribir en la página
}

void actualizar_tiempo(Pagina* pagina,t_temporal* tiempo){
	pthread_mutex_lock(&(pagina->mutex));
	if (strcmp(ALGORITMO_REEMPLAZO, "LRU") == 0) {
	        temporal_destroy(tiempo);  // Liberar el tiempo antiguo
	        pagina->ultimo_uso = temporal_create();  // Crear un nuevo temporizador
	    }
	 pthread_mutex_unlock(&(pagina->mutex));

}
/*
 * Pagina* pagina = list_get(tabla->paginas, numero_pagina);
 * actualizar_tiempo(pagina, pagina->ultimo_uso);
 *
 */
void cargar_pagina_en_memoria(tabla_paginas* tabla, Pagina* pagina) {
    // Aquí iría el código para cargar la página en memoria

    // Si es FIFO, actualizamos el orden de carga
    if (strcmp(ALGORITMO_REEMPLAZO, "FIFO") == 0) {
        pagina->orden_carga = ordenCargaGlobal++;
    }
}
Pagina* comparar_acceso_LRU(Pagina* pagina1, Pagina* pagina2) {
    if (temporal_gettime(pagina1->ultimo_uso)>temporal_gettime(pagina2->ultimo_uso)){
    	return pagina1;
    }
    else return pagina2;
}

// Comparador para FIFO
Pagina* comparar_orden_carga(Pagina* pagina1, Pagina* pagina2) {
    if (pagina1->orden_carga < pagina2->orden_carga){
    	return pagina1;
    }
    else return pagina2;
}
Pagina* victima_pagina_LRU(tabla_paginas* tabla) {
    Pagina* pagina_a_reemplazar = list_get_maximum(tabla->paginas, (void*) comparar_acceso_LRU);
    return pagina_a_reemplazar;
}
Pagina* victima_pagina_FIFO(tabla_paginas* tabla) {
    Pagina* pagina_a_reemplazar = list_get_minimum(tabla->paginas, (void*) comparar_orden_carga);
    return pagina_a_reemplazar;
}


// Comparador para LRU


Pagina* obtener_pagina_por_marco(marco* un_marco){
	if(un_marco->libre){
		return NULL;
	}
	int pid = un_marco->pid;
	int nro_pagina = un_marco->nro_pagina;
	if(pid == 0){
		printf("No encuentro en pid del marco");
		return NULL;
	}
	if(nro_pagina == 0){
		printf("No encuentro el nro de pagina del marco");
		return NULL;
	}
	tabla_paginas* una_tabla = buscar_tabla(pid);
	if(una_tabla == NULL){
		printf("No encuentro la tabla buscando por el pid");
		return NULL;
	}
	return list_get(una_tabla->paginas, nro_pagina-1);
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
