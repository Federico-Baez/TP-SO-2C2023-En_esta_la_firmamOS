#include "../include/fs_memoria.h"


// =========== ATENDER MEMORIA ===============

void atender_asignacion_de_bloques_por_creacion_de_proceso(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int cant_bloques = recibir_int_del_buffer(un_buffer);

	//Pedirle al modulo bloques.c
	t_list* una_lista_bloques = swap_obtener_n_cantidad_de_bloques(cant_bloques);

	enviar_lista_bloques_a_memoria(pid, una_lista_bloques);

	//Lierar lista de la memoria
	void _liberar_int(int* un_valor){
		free(un_valor);
	}
	list_destroy_and_destroy_elements(una_lista_bloques, (void*)_liberar_int);

}










// =========== ENVIAR A MEMORIA ===============

void  enviar_lista_bloques_a_memoria(int pid, t_list* lista_bloques){
	//Cargar lista en un paquete para enviar
	t_paquete* un_paquete = crear_super_paquete(100);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, list_size(lista_bloques));

	for(int i=0; i<list_size(lista_bloques); i++){
		int* nro_bloque = list_get(lista_bloques, i);
		cargar_int_al_super_paquete(un_paquete, *nro_bloque);
	}

	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

