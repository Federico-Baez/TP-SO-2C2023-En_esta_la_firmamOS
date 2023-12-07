#include "../include/swap.h"

void solicitar_asignacion_bloques_SWAP_a_FS(int pid, int numero_paginas){
	//
	t_paquete* un_paquete = crear_super_paquete(PETICION_ASIGNACION_BLOQUE_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, numero_paginas);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void asignar_posicions_de_SWAP_a_tabla_de_paginas(void* pid_cant_lista){
	//
	int pid = recibir_int_del_buffer(pid_cant_lista);
	int cant_bloques = recibir_int_del_buffer(pid_cant_lista);
	t_list* lista_de_bloques_asignados = list_create();

	for(int i=0; i<cant_bloques; i++){
		int* n_bloque = malloc(sizeof(int));
		list_add(lista_de_bloques_asignados, n_bloque);
	}

	cargar_bloques_asignados_en_proceso(pid, lista_de_bloques_asignados);

	void _elminar_nodo_temporal(int* nodo_bloque){
		free(nodo_bloque);
	}

	list_clean_and_destroy_elements(lista_de_bloques_asignados, (void*)_elminar_nodo_temporal);
	list_destroy(lista_de_bloques_asignados);

}
