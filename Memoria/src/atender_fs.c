#include "../include/atender_fs.h"

//============RECIBIDOS DE FAILESYSTEM=======================

void asignar_posicions_de_SWAP_a_tabla_de_paginas_de_un_proceso(t_buffer* un_buffer){
	//
	int pid = recibir_int_del_buffer(un_buffer);
	int cant_bloques = recibir_int_del_buffer(un_buffer);

	//Extrayendo pos_en_swap del buffer en una lista auxiliar
	t_list* lista_de_pos_swap = list_create();
	for(int i=0; i<cant_bloques; i++){
		int* pos_en_swap = malloc(sizeof(int));
		*pos_en_swap = recibir_int_del_buffer(un_buffer);
		list_add(lista_de_pos_swap, pos_en_swap);
	}

	//Verifico que la cantidad de elementos extraidos conincida con la cantidad de paginas a asignar
	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	int cantidad_de_paginas = list_size(un_proceso->tabla_paginas);
	log_info(memoria_logger, "FILESYSTEM => <PID:%d> <cant_pag:%d> - <cant_bloques:%d>", un_proceso->pid, cantidad_de_paginas, cant_bloques);
	if(cant_bloques != cantidad_de_paginas){
		log_error(memoria_logger, "<PID:%d> NO COINCIDE la cant_paginas del proceso <%d> y la cant_bloques <%d> recibidos de FS", un_proceso->pid, cantidad_de_paginas, cant_bloques);
		exit(EXIT_FAILURE);
	}

	//Si todo coincide, proceder a asignar pos_swap en cada pagina
	cargar_bloques_asignados_en_proceso(un_proceso, lista_de_pos_swap);

	//Eliminar la lista temporal de info_pos_swap
	void _elminar_nodo_temporal(int* nodo_bloque){
		free(nodo_bloque);
	}
	list_clean_and_destroy_elements(lista_de_pos_swap, (void*)_elminar_nodo_temporal);
	list_destroy(lista_de_pos_swap);

}

void atender_lectura_de_pagina_de_swap_a_memoria(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int nro_pagina = recibir_int_del_buffer(un_buffer);
	void* pagina = recibir_choclo_del_buffer(un_buffer);

	//Ubicar proceso correspondiente
	t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//ubicar pagina correspondiente
	t_pagina* una_pagina = pag_obtener_pagina_completa(un_proceso, nro_pagina);

	//Ubicar el marco correspondiente
	t_marco* un_marco = obtener_marco_por_nro_marco(una_pagina->nro_marco);

	//Guardar en el espacio_usuario el choclo
	escribir_pagina_en_una_dir_fisica_especifica(pid, un_marco->base, pagina);

	//Avisar a KERNEL el exito de la operacion
	enviar_a_kernel_rpta_del_pedido_de_carga_de_pagina(pid);
}

void atender_bloque_de_memoria_y_llevarlos_a_fylesystem(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);
	int nro_bloque = recibir_int_del_buffer(un_buffer);
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);

	//Extraer el marco completo en un void*
	void* un_marco = copiar_marco_desde_una_dir_fisica(pid, dir_fisica);

	//Enviar marco a FS
	enviar_marco_a_fs(pid, nro_bloque, un_marco, nombre_archivo);

	free(un_marco);

}

void atender_bloque_de_fs_a_memoria(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);
	void* choclito_bloque = recibir_choclo_del_buffer(un_buffer);

	//Guardar bloque en memoria
	escribir_pagina_en_una_dir_fisica_especifica(pid, dir_fisica, choclito_bloque);

	//Responder a FS el exito de la operacion
	enviar_rpta_por_pedido_de_escritura_en_memoria(pid);
}


//============ENVIOS A FILEsYSTEM=======================

void enviar_a_fs_peticion_de_asignacion_de_bloques(int pid, int cantidad_de_paginas){
	//[int pid][int cant_bloq_swap]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(PETICION_ASIGNACION_BLOQUE_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, cantidad_de_paginas);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void enviar_a_fs_orden_de_liberacion_de_posiciones_swap(t_proceso* un_proceso){
	//[int cant_bloq_swap][int][int]...[int]
	retardo_respuesta_cpu_fs();
	int cant_elementos = list_size(un_proceso->tabla_paginas);
	t_paquete* un_paquete = crear_super_paquete(LIBERAR_PAGINAS_FM);
	cargar_int_al_super_paquete(un_paquete, cant_elementos);
	for(int i=0; i<cant_elementos; i++){
		t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, i);
		cargar_int_al_super_paquete(un_paquete, una_pagina->pos_en_swap);
	}

	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void evniar_pagina_a_fs_area_swap(int pos_swap, void* coso_marco){
	//[int pos_swap][void* choclo]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(GUARDAR_MARCO_EN_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, pos_swap);
	cargar_choclo_al_super_paquete(un_paquete, coso_marco, TAM_PAGINA);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}


void pedir_lectura_de_pag_swap_a_fs(int pid, int nro_pagina, int pos_en_swap){
	//[int pid][int nro_pag][int pos_swap]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(LECTURA_MARCO_DE_SWAP_MF);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, nro_pagina);
	cargar_int_al_super_paquete(un_paquete, pos_en_swap);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void enviar_marco_a_fs(int pid, int nro_bloque, void* un_marco, char* nombre_archivo){
	//[int pid][int nro_bloque][void* marco_bloque][char* nombre_archivo]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(BLOQUE_DE_MEMORIA_A_FILESYSTEM_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_int_al_super_paquete(un_paquete, nro_bloque);
	cargar_choclo_al_super_paquete(un_paquete, un_marco, TAM_PAGINA);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void enviar_rpta_por_pedido_de_escritura_en_memoria(int pid){
	//[int pid][char* rpta]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(RPTA_BLOQUE_DE_FILESYSTEM_A_MEMORIA_FM);
	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_string_al_super_paquete(un_paquete, "OK");
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}



