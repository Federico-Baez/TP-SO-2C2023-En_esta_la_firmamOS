#include "../include/atender_kernel.h"

//============RECIBIDOS DE KERNEL=======================

void iniciar_estructura_para_un_proceso_nuevo(t_buffer* un_Buffer){
	char* path = recibir_string_del_buffer(un_Buffer);
	int size = recibir_int_del_buffer(un_Buffer);
	int pid = recibir_int_del_buffer(un_Buffer);

	//Crear un proceso
	t_proceso* un_proceso = crear_proceso(pid, size, path);

	//Agregar a la lista de procesos
	agregar_proceso_a_listado(un_proceso, list_procss_recibidos);

	//Enviar a FS la cant_marcos para swap
	enviar_a_fs_peticion_de_asignacion_de_bloques(un_proceso->pid, list_size(un_proceso->tabla_paginas));
}

void eliminar_proceso_y_liberar_estructuras(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	if(list_remove_element(list_procss_recibidos, un_proceso)){
		eliminar_proceso(un_proceso);
		log_warning(memoria_logger, ":::::> DELETE <PID:%d>", pid);
	}else{
		log_error(memoria_logger, "Proceso no encontrado en la lista de procesos para ser eliminados");
		exit(EXIT_FAILURE);
	}
}

/*Se asume que la pagina por la que se pregunta es poque CPU ya confirmo Page Fault*/
void atender_pagefault_kernel(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int nro_pagina = recibir_int_del_buffer(un_buffer);

	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	t_pagina* una_pagina = pag_obtener_pagina_completa(un_proceso, nro_pagina);

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

	//Enviar a FileSystem peticion de lectura de pagina SWAP
	//M -> FS: [PID][pos_swap]
	pedir_lectura_de_pag_swap_a_fs(pid, nro_pagina, una_pagina->pos_en_swap);
	//Log de lectura obligatorio
	logg_lectura_pagina_swap(pid, un_marco->nro_marco, nro_pagina);

	//Log obligatorio
	if(tipo_marco == MARCO_VICTIMA){
		logg_reemplazar_pagina(	un_marco->nro_marco,
							un_marco->info_old->proceso->pid,
							un_marco->info_old->nro_pagina,
							un_marco->info_new->proceso->pid,
							un_marco->info_new->nro_pagina);
	}

	//Control de ERROR
	if(tipo_marco >= 2){
		log_error(memoria_logger, "El tipo de marco no se setea Correctamente - Kernel_pagefault");
		exit(EXIT_FAILURE);
	}

}

//============ENVIOS A KERNEL=======================

void enviar_a_kernel_rpta_del_pedido_de_carga_de_pagina(int pid){
	//M -> K : [int pid]
	t_paquete* un_paquete = crear_super_paquete(RESPUESTA_PAGE_FAULT_MK);
	cargar_int_al_super_paquete(un_paquete, pid);
	enviar_paquete(un_paquete, fd_kernel);
	eliminar_paquete(un_paquete);
}























