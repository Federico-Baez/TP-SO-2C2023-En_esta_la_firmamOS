#include "../include/finalizar_kernel.h"

static void _finalizar_logger(){
	log_destroy(kernel_logger);
	log_destroy(kernel_log_obligatorio);
}

static void _finalizar_config(){
	string_array_destroy(RECURSOS);
	string_array_destroy(INSTANCIAS_RECURSOS);
	config_destroy(kernel_config);

}

static void _finalizar_listas(){
	list_destroy(lista_new);
	list_destroy(lista_ready);
	list_destroy(lista_execute);
	list_destroy(lista_blocked);
	list_destroy(lista_exit);
	list_destroy(lista_general);
	list_destroy(cola_blocked_fs);
}


static void _finalizar_semaforos(){
	sem_destroy(&sem_pausa);
	sem_destroy(&sem_estructura_iniciada);
	sem_destroy(&sem_enviar_interrupcion);
	sem_destroy(&sem_estructura_iniciada);
	sem_destroy(&sem_estructura_liberada);
	sem_destroy(&sem_nuevo_en_block);
}

static void _finalizar_pthread(){
	pthread_mutex_destroy(&mutex_lista_new);
	pthread_mutex_destroy(&mutex_lista_ready);
	pthread_mutex_destroy(&mutex_lista_exec);
	pthread_mutex_destroy(&mutex_lista_blocked);
	pthread_mutex_destroy(&mutex_lista_exit);
	pthread_mutex_destroy(&mutex_lista_general);
	pthread_mutex_destroy(&mutex_cola_blocked_fs);

	pthread_mutex_destroy(&mutex_process_id);
	pthread_mutex_destroy(&mutex_core);
	pthread_mutex_destroy(&mutex_pausa);
	pthread_mutex_destroy(&mutex_recurso);
	pthread_mutex_destroy(&mutex_ticket);
	pthread_mutex_destroy(&mutex_enviar_interrupcion);

	pthread_mutex_destroy(&mutex_flag_finalizar_proceso);
	pthread_mutex_destroy(&mutex_manejo_page_fault);
	pthread_mutex_destroy(&mutex_flag_proceso_desalojado);
	pthread_mutex_destroy(&mutex_peticion_fs);
	pthread_mutex_destroy(&mutex_existe_archivo);

}

static void _finalizar_recursos(){
	void __eliminar_nodo_recurso(t_recurso* un_recurso){
		list_destroy(un_recurso->lista_bloqueados);
		pthread_mutex_destroy(&un_recurso->mutex_bloqueados);
		un_recurso->pcb_asignado = NULL;
		free(un_recurso);
	}

	list_destroy_and_destroy_elements(lista_recursos, (void*)__eliminar_nodo_recurso);
}

static void _finalizar_archivos(){

	void __eliminar_nodo_archivo(t_archivo* archivo){
		list_destroy(archivo->cola_block_procesos);
		pthread_mutex_destroy(&archivo->mutex_cola_block);
		free(archivo->nombre_archivo);
		archivo->lock_escritura->pcb = NULL;
		list_destroy(archivo->lock_lectura->lista_participantes);
		pthread_mutex_destroy(&archivo->lock_lectura->mutex_lista_asiganada);
		free(archivo);
	}

	list_destroy_and_destroy_elements(lista_archivos_abiertos, (void*)__eliminar_nodo_archivo);
}

static void _destruir_conexiones(){
	liberar_conexion(fd_cpu_dispatcher);
	liberar_conexion(fd_cpu_interrupt);
	liberar_conexion(fd_filesystem);
	liberar_conexion(fd_memoria);
}

static void _eliminar_pcbs(){
	while(!list_is_empty(lista_new)){
		list_clean_and_destroy_elements(lista_new, (void*)destruir_pcb);
	}
	while(!list_is_empty(lista_ready)){
		list_clean_and_destroy_elements(lista_ready, (void*)destruir_pcb);
	}
	while(!list_is_empty(lista_execute)){
		list_clean_and_destroy_elements(lista_execute, (void*)destruir_pcb);
	}
	while(!list_is_empty(lista_blocked)){
		list_clean_and_destroy_elements(lista_blocked, (void*)destruir_pcb);
	}
	while(!list_is_empty(lista_exit)){
		list_clean_and_destroy_elements(lista_exit, (void*)destruir_pcb);
	}
}

/*==========================================*/

void finalizar_kernel(){
	_finalizar_logger();
	_finalizar_config();
	_eliminar_pcbs();
	_finalizar_recursos();
	_finalizar_archivos();

	_finalizar_listas();
	_finalizar_semaforos();
	_finalizar_pthread();

	_destruir_conexiones();
}
