#include "../include/fs_memoria.h"


// =========== ATENDER MEMORIA ===============

void atender_asignacion_de_bloques_por_creacion_de_proceso(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int cant_bloques = recibir_int_del_buffer(un_buffer);

	//Pedirle al modulo tabla_fat.c
	t_list* una_lista_bloques = swap_obtener_n_cantidad_de_bloques(cant_bloques);

	enviar_lista_bloques_a_memoria(pid, una_lista_bloques);

	//Lierar lista de la memoria
	void _liberar_int(int* un_valor){
		free(un_valor);
	}
	list_destroy_and_destroy_elements(una_lista_bloques, (void*)_liberar_int);

}

void atender_peticion_de_libarar_bloques_swap(t_buffer* un_buffer){
	int cant_bloques = recibir_int_del_buffer(un_buffer);
	log_warning(filesystem_logger, "CANT_BLOQUES_A_LIBERAR: %d", cant_bloques);

	int nro_bloque_swap;
	for(int i=0; i<=cant_bloques; i++){
		nro_bloque_swap = recibir_int_del_buffer(un_buffer);
		setear_bloque_de_swap_como_libre(nro_bloque_swap);
	}
}

void atender_peticion_de_guardar_marco_en_swap(t_buffer* un_buffer){
	int pos_swap = recibir_int_del_buffer(un_buffer);
	void* un_marco = recibir_choclo_del_buffer(un_buffer);

	swap_actualizar_pagina_bloque_swap(pos_swap, un_marco);
}

void atender_pedido_de_lectura_de_pag_swap(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int nro_pagina = recibir_int_del_buffer(un_buffer);
	int pos_swap = recibir_int_del_buffer(un_buffer);

	log_warning(filesystem_logger, "SWAP_PAGEFAULT: <PID:%d> <Nro_PAG:%d> <POS_SWAP>", pid, nro_pagina, pos_swap);
	void* un_bloque = swap_obtener_bloque_pagina_de_pos_swap(pos_swap);

	//Enviar bloque_pagina a Memoria
	enviar_bloque_pagina_a_memoria(pid, nro_pagina, un_bloque);
}

void atender_recepcion_de_marco_bloque_de_memoria_por_f_write_de_kernel(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int nro_bloque = recibir_int_del_buffer(un_buffer);
	void* marco_bloque = recibir_choclo_del_buffer(un_buffer);
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);

	//Guardar en un nro de marco especifico
	guardar_info_pagina_en_un_bloque_especifico(nombre_archivo, nro_bloque, marco_bloque);

	//Enviar rpta a kernel
	//RESPUESTA_F_WRITE_FK
	enviar_rpta_a_kernel_del_f_write(pid);
}

void atender_rpta_de_memoria_a_fs_por_lectura_de_marco_por_dir_fisica(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	char* rpta_de_memoria = recibir_string_del_buffer(un_buffer);

	if(strcmp(rpta_de_memoria, "OK") == 0){
		enviar_confirmacion_de_lectura_a_kernel(pid);
	}else{
		log_error(filesystem_logger, "RPTA de Memoria a FileSystem por el fread");
	}
}



// =========== ENVIAR A MEMORIA ============================

void  enviar_lista_bloques_a_memoria(int pid, t_list* lista_bloques){
	//Cargar lista en un paquete para enviar
	t_paquete* un_paquete = crear_super_paquete(PETICION_ASIGNACION_BLOQUE_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, list_size(lista_bloques));

	for(int i=0; i<list_size(lista_bloques); i++){
		int* nro_bloque = list_get(lista_bloques, i);
		cargar_int_al_super_paquete(un_paquete, *nro_bloque);
	}

	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

void enviar_bloque_pagina_a_memoria(int pid, int nro_pagina, void* un_bloque){
	t_paquete* un_paquete = crear_super_paquete(RPTA_LECTURA_MARCO_DE_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, nro_pagina);
	cargar_choclo_al_super_paquete(un_paquete, un_bloque, TAM_BLOQUE);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}



// =========== ENVIAR A KERNEL ============================

void enviar_rpta_a_kernel_del_f_write(int pid){
	t_paquete* un_paquete = crear_super_paquete(RESPUESTA_F_WRITE_FK);
	cargar_string_al_super_paquete(un_paquete, "OK");
	cargar_int_al_super_paquete(un_paquete, pid);
	enviar_paquete(un_paquete, fd_kernel);
	eliminar_paquete(un_paquete);
}

void enviar_confirmacion_de_lectura_a_kernel(pid){
	t_paquete* un_paquete = crear_super_paquete(RESPUESTA_F_READ_FK);
	cargar_string_al_super_paquete(un_paquete, "OK");
	cargar_int_al_super_paquete(un_paquete, pid);
	enviar_paquete(un_paquete, fd_kernel);
	eliminar_paquete(un_paquete);
}





