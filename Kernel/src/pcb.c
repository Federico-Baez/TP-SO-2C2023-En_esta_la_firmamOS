#include "../include/pcb.h"

/*Convierte el valor del estado a texto
 * (Retorna NULL si no detecta algun estado conocido)*/
char* estado_to_string(int un_valor){
	char* nombre_del_estado;
	switch (un_valor) {
		case NEW:
			nombre_del_estado = "NEW";
			break;
		case READY:
			nombre_del_estado = "READY";
			break;
		case EXEC:
			nombre_del_estado = "EXEC";
			break;
		case BLOCKED:
			nombre_del_estado = "BLOCKED";
			break;
		case EXIT:
			nombre_del_estado = "EXIT";
			break;
		default:
			nombre_del_estado = NULL;
			log_error(kernel_logger, "No se reconocio el nombre del estado");
			break;
	}
	if(nombre_del_estado != NULL){
		return string_duplicate(nombre_del_estado);
	}else{
		return nombre_del_estado;
	}
}

char* motivo_to_string(t_motivo_exit motivo_exit){

	switch(motivo_exit){
	case SUCCESS:
		return "SUCCESS";
		break;
	case INVALID_RESOURCE:
		return "INVALID_RESOURCE";
		break;
	case INVALID_WRITE:
		return "INVALID_WRITE";
		break;
	default:
		return "ERROR";
	}
}

t_pcb* crear_pcb(char* path, char* size, char* prioridad){
	t_pcb* nuevo_PCB = malloc(sizeof(t_pcb));
	pthread_mutex_lock(&mutex_process_id);
	process_id++;
	nuevo_PCB->pid = process_id;
	pthread_mutex_unlock(&mutex_process_id);
	nuevo_PCB->program_counter = 0;
	nuevo_PCB->ticket = 0;

	nuevo_PCB->size = atoi(size);
	nuevo_PCB->prioridad = atoi(prioridad);
	nuevo_PCB->path = string_duplicate(path);

	nuevo_PCB->lista_recursos_pcb = list_create();
	pthread_mutex_init(&nuevo_PCB->mutex_lista_recursos, NULL);

	nuevo_PCB->registros_CPU = malloc(sizeof(t_registros_CPU));
	nuevo_PCB->registros_CPU->AX = 0;
	nuevo_PCB->registros_CPU->BX = 0;
	nuevo_PCB->registros_CPU->CX = 0;
	nuevo_PCB->registros_CPU->DX = 0;

	nuevo_PCB->archivos_abiertos = list_create();

	return nuevo_PCB;
}


void destruir_pcb(t_pcb* un_pcb){
	free(un_pcb->path);
	free(un_pcb->registros_CPU);
	list_destroy(un_pcb->lista_recursos_pcb);
	pthread_mutex_destroy(&un_pcb->mutex_lista_recursos);
	destruir_t_archivos_abiertos(un_pcb->archivos_abiertos);
	free(un_pcb);
}

void destruir_t_archivos_abiertos (t_list* lista_archivos_abiertos_pcb){
	void __eliminar_nodo_archivo_pcb(t_archivo_abierto_pcb* archivo_pcb){
		free(archivo_pcb->modo_apertura);
		free(archivo_pcb);
	}

	list_clean_and_destroy_elements(lista_archivos_abiertos_pcb, (void*)__eliminar_nodo_archivo_pcb);
}

void imprimir_pcb(t_pcb* un_pcb){
	log_info(kernel_logger, "<PCB_%d> [%s][%d][%d]",
							un_pcb->pid,
							un_pcb->path,
							un_pcb->size,
							un_pcb->prioridad);

}

void imprimir_pcb_v2(t_pcb* un_pcb){
	char* string_estado = estado_to_string(un_pcb->estado);
	log_info(kernel_logger, "<PCB>[PID:%d][%s][%d]",
							un_pcb->pid,
							string_estado,
							un_pcb->prioridad);
	free(string_estado);
}

void cambiar_estado(t_pcb* un_pcb, est_pcb nex_state){
	un_pcb->estado = nex_state;
}

void _enviar_pcb_a_CPU_por_dispatch(t_pcb* una_pcb){
	t_paquete* un_paquete = crear_super_paquete(EJECUTAR_PROCESO_KC);
	cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
	cargar_int_al_super_paquete(un_paquete, una_pcb->ticket);
	cargar_int_al_super_paquete(un_paquete, una_pcb->program_counter);
	cargar_choclo_al_super_paquete(un_paquete, &(una_pcb->registros_CPU->AX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(un_paquete, &(una_pcb->registros_CPU->BX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(un_paquete, &(una_pcb->registros_CPU->CX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(un_paquete, &(una_pcb->registros_CPU->DX), sizeof(uint32_t));

	enviar_paquete(un_paquete, fd_cpu_dispatcher);
	eliminar_paquete(un_paquete);
}

// Envia a CPU un valor entero >0 en caso de que se haya podido realizar la instruccion, para que siga ejecutando
// o un -1 en caso de desalojo.
void _enviar_respuesta_instruccion_CPU_por_dispatch(int respuesta){
	t_paquete* un_paquete = crear_super_paquete(RESPUESTA_INSTRUCCION_KC);
//	cargar_string_al_super_paquete(un_paquete, instruccion);
	cargar_int_al_super_paquete(un_paquete, respuesta);

	enviar_paquete(un_paquete, fd_cpu_dispatcher);
	eliminar_paquete(un_paquete);
}

/*Busca la PCB por el PID pasado por parametro
 * (Devuleve NULL si no encusntra el PID en ninguna lista)*/
t_pcb* buscar_pcb_por_pid(int un_pid){
	t_pcb* un_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_new);
		if(list_any_satisfy(lista_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_new, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_new);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_ready);
		if(list_any_satisfy(lista_ready, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_ready, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_ready);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exec);
		if(list_any_satisfy(lista_execute, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_execute, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exec);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exit);
		if(list_any_satisfy(lista_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exit);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_blocked);
		if(list_any_satisfy(lista_blocked, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_blocked, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_blocked);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		un_pcb = NULL;
		log_error(kernel_logger, "PID no encontrada en ninguna lista");
	}

	return un_pcb;
}

// Lo tengo que proteger con su mutex pertinente
t_pcb* buscar_pcb_por_pid_en(int un_pid, t_list* lista_estado, pthread_mutex_t mutex_lista){
	t_pcb* un_pcb;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

//	pthread_mutex_lock(&mutex_lista);
	if(list_any_satisfy(lista_estado, (void*)__buscar_pcb)){
		un_pcb = list_find(lista_estado, (void*)__buscar_pcb);
	}
	else{
		un_pcb = NULL;
	}
//	pthread_mutex_unlock(&mutex_lista);
	return un_pcb;
}

/*Para llamar a esta  funcion necesitas protegerla con MUTEX segun la lista*/
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* un_pcb){
	int var_aux = 0;
	void __buscar_pcb_exacta(t_pcb* void_pcb){
		if(void_pcb == un_pcb){
			var_aux++;
		}
	}

	list_iterate(una_lista, (void*)__buscar_pcb_exacta);
	if(var_aux != 0){
		return true;
	}else{
		return false;
	}
}

//Esto es para que se liberen todos los recursos asignados
void liberar_todos_los_recursos_de_un_pcb(t_pcb* un_pcb){
	while(!list_is_empty(un_pcb->lista_recursos_pcb)){
		t_recurso* un_recurso = list_remove(un_pcb->lista_recursos_pcb, 0);

		//Aumento el valor del recurso en +1
		pthread_mutex_lock(&mutex_recurso);
		un_recurso->instancias = un_recurso->instancias + 1;
		pthread_mutex_unlock(&mutex_recurso);

		//[FALTA] LLamar al gestor de recursos para que explore y libere a alguna PCB en espera
	}
}

void avisar_a_memoria_para_liberar_estructuras(t_pcb* un_pcb){
	t_paquete* un_paquete = crear_super_paquete(LIBERAR_ESTRUCTURA_KM);
	cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
	log_info(kernel_logger, "Mensaje a MEMORIA: LIBERAR_ESTRUCTURA_KM [PID: %d]", un_pcb->pid);
}

t_pcb* recibir_pcb_memoria(t_buffer* un_buffer){
//	int recibe_pid = recibir_int_del_buffer(un_buffer);
//	t_pcb* pcb = buscar_pcb_por_pid(recibe_pid);
//	if(pcb == NULL){
//		log_error(kernel_logger, "PID recibido no coincide con PCB de ninguna lista");
//		exit(EXIT_FAILURE);
//	}
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->pid = recibir_int_del_buffer(un_buffer);
	pcb->size = recibir_int_del_buffer(un_buffer);
	pcb->path = recibir_string_del_buffer(un_buffer);

	return pcb;
}

void transferir_from_actual_to_siguiente(t_pcb* pcb, t_list* lista_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente){
	char*  estado_anterior = estado_to_string(pcb->estado);
	char* siguente_estado = estado_to_string(estado_siguiente);

	cambiar_estado(pcb, estado_siguiente);

	agregar_pcb_lista(pcb, lista_siguiente, mutex_siguiente);
//	list_add(lista_siguiente, pcb);

	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb -> pid, estado_anterior, siguente_estado);


	if(strcmp(siguente_estado, "READY") == 0){
//		pthread_mutex_lock(&mutex_siguiente);
		if(strcmp(estado_anterior, "NEW") == 0){
			char* pids_en_ready = lista_pids_en_ready(lista_ready, mutex_lista_ready);
			log_info(kernel_log_obligatorio, "Cola Ready FIFO: %s", pids_en_ready);
			free(pids_en_ready);
		}else{
			char* pids_en_ready = lista_pids_en_ready(lista_ready, mutex_lista_ready);
			log_info(kernel_log_obligatorio, "Cola Ready %s: %s", algoritmo_to_string(ALGORITMO_PLANIFICACION), pids_en_ready);
			free(pids_en_ready);
		}
//		pthread_mutex_unlock(&mutex_siguiente);
	}
	free(estado_anterior);
	free(siguente_estado);
}

void agregar_pcb_lista(t_pcb* pcb, t_list* lista_estado, pthread_mutex_t mutex_lista){
	pthread_mutex_lock(&mutex_lista);
	list_add(lista_estado, pcb);
	pthread_mutex_unlock(&mutex_lista);
}

char* lista_pids_en_ready(t_list* lista_estado, pthread_mutex_t mutex_lista){
	int id_process;
	char* pids_en_string = string_new();
	string_append(&pids_en_string, "[");

//	pthread_mutex_lock(&mutex_lista);
	for(int i = 0; i < list_size(lista_estado); i++){
		if(i == 0){
			t_pcb* pcb = list_get(lista_estado, i);
			id_process = pcb -> pid;
			string_append(&pids_en_string, string_itoa(id_process));
		}else{
			string_append(&pids_en_string, ", ");
			t_pcb* pcb = list_get(lista_estado, i);
			id_process = pcb -> pid;
			string_append(&pids_en_string, string_itoa(id_process));
		}

	}
//	pthread_mutex_unlock(&mutex_lista);

	string_append(&pids_en_string, "]");

	return pids_en_string;
}

void liberar_recursos_pcb(t_pcb* pcb){
	t_list* recursos_del_pcb = list_duplicate(pcb->lista_recursos_pcb);
	while(!list_is_empty(recursos_del_pcb)){
		t_recurso* un_recurso = NULL;
		pthread_mutex_lock(&pcb->mutex_lista_recursos);
		un_recurso = list_remove(recursos_del_pcb,0);
		if(un_recurso != NULL){
			pthread_mutex_lock(&un_recurso->mutex_bloqueados);
			if(list_remove_element(un_recurso->lista_bloqueados, pcb)){
				if(list_remove_element(pcb->lista_recursos_pcb, un_recurso)){

				}
			}else{
				un_recurso->instancias++;
				// Se manda como un hilo, para que cuando se detiene la planificacion, se pause la asignacion de recursos para los demas PCBs y no entren a READY.
				// Tambien agiliza eliminar +1 proceso a la vez.
				ejecutar_en_un_hilo_nuevo_detach((void*)asignar_recurso_liberado_pcb, un_recurso);
			}
			pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
		}
		pthread_mutex_unlock(&pcb->mutex_lista_recursos);
	}
	list_destroy(recursos_del_pcb);
}

void asignar_recurso_liberado_pcb(t_recurso* un_recurso){
	pausador();

	pthread_mutex_lock(&un_recurso->mutex_bloqueados);

	if(!list_is_empty(un_recurso->lista_bloqueados)){
		t_pcb* pcb_liberado = list_remove(un_recurso->lista_bloqueados,0);

		un_recurso->pcb_asignado = pcb_liberado;
		un_recurso->instancias--;
		int* pid_process = malloc(sizeof(int));
		*pid_process = pcb_liberado->pid;
		desbloquear_proceso_por_pid(pid_process);
	}else
		log_warning(kernel_logger, "La lista de BLOQUEADOS del RECURSO esta vacia");

	pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
}

void desbloquear_proceso_por_pid(int* pid_process){
//	Busco el pcb en la lista de bloqueados y lo elimino
	pausador();
	pthread_mutex_lock(&mutex_lista_blocked);
	t_pcb* pcb = buscar_pcb_por_pid_en(*pid_process, lista_blocked, mutex_lista_blocked);
	if(pcb != NULL){
		list_remove_element(lista_blocked, pcb);
	//	El pcb pasa a la lista de READY
		transferir_from_actual_to_siguiente(pcb, lista_ready, mutex_lista_ready, READY);
	}
	pthread_mutex_unlock(&mutex_lista_blocked);
	free(pid_process);
	pcp_planificar_corto_plazo();
}

void bloquear_proceso_cola_fs(t_pcb* pcb, t_archivo* archivo){
	pthread_mutex_lock(&archivo->mutex_cola_block);
	list_add(archivo->cola_block_procesos, pcb);
	pthread_mutex_unlock(&archivo->mutex_cola_block);
}

void liberar_lock_lectura(t_lock_lectura* el_lock, t_pcb* pcb){
	pthread_mutex_lock(&el_lock->mutex_lista_asiganada);
	list_remove_element(el_lock->lista_participantes, pcb);
	el_lock->cantidad_participantes--;

	if(el_lock->cantidad_participantes <= 0){
		el_lock->locked = 0;
	}
	pthread_mutex_unlock(&el_lock->mutex_lista_asiganada);
}

void liberar_lock_escritura(t_lock_escritura* el_lock, t_pcb* pcb){
	el_lock->locked = 0;
	el_lock->pcb = NULL;
}

void asignar_lock_pcb(t_archivo* archivo){
	// Saco el primer pcb en la cola de blocks
	t_pcb* pcb = list_remove(archivo->cola_block_procesos,0);
	// Busco el strcut del archivo dentro del pcb
	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, archivo->nombre_archivo);
	// Consulto si su solicitud de apertura es "r" o "w"
	if(strcmp(archivo_pcb->modo_apertura , "R") == 0){
		archivo->lock_lectura->locked = 1;
		list_add(archivo->lock_lectura->lista_participantes, pcb);
		archivo->lock_lectura->cantidad_participantes++;
		archivo_pcb->lock_otorgado = 1;
		int* pid = malloc(sizeof(int));
		*pid = pcb->pid;
		desbloquear_proceso_por_pid(pid);
		if(!list_is_empty(archivo->cola_block_procesos)){
			pcb = list_get(archivo->cola_block_procesos,0);
			archivo_pcb = obtener_archivo_pcb(pcb, archivo->nombre_archivo);
			while(strcmp(archivo_pcb->modo_apertura , "R") == 0 && pcb != NULL){
				archivo->lock_lectura->locked = 1;
				list_add(archivo->lock_lectura->lista_participantes, pcb);
				archivo->lock_lectura->cantidad_participantes++;
				archivo_pcb->lock_otorgado = 1;
				list_remove_element(archivo->cola_block_procesos, pcb);
				*pid = pcb->pid;
				desbloquear_proceso_por_pid(pid);
				if(!list_is_empty(archivo->cola_block_procesos)){
					pcb = list_get(archivo->cola_block_procesos,0);
					archivo_pcb = obtener_archivo_pcb(pcb, archivo->nombre_archivo);
				}
			}
		}
	}else{
		archivo->lock_escritura->locked = 1;
		archivo->lock_escritura->pcb = pcb;
		archivo_pcb->lock_otorgado = 1;
		int* pid = malloc(sizeof(int));
		*pid = pcb->pid;
		desbloquear_proceso_por_pid(pid);
	}
}


void asignar_archivo_pcb(t_pcb* pcb, t_archivo* archivo, char* tipo_apertura){
	t_archivo_abierto_pcb* archivo_pcb = malloc(sizeof(t_archivo_abierto_pcb));
//	archivo_pcb->archivo_abierto = archivo;
	archivo_pcb->nombre_archivo = archivo->nombre_archivo;
	archivo_pcb->modo_apertura = strdup(tipo_apertura);
	archivo_pcb->lock_otorgado = 0;
	archivo_pcb->puntero = 0;
	list_add(pcb->archivos_abiertos, archivo_pcb);
}

t_archivo_abierto_pcb* obtener_archivo_pcb(t_pcb* pcb, char* nombre_archivo_pcb){
	t_archivo_abierto_pcb* archivo_pcb;

	if(!list_is_empty(pcb->archivos_abiertos)){
		for(int i = 0; i < list_size(pcb->archivos_abiertos); i++){
			archivo_pcb = list_get(pcb->archivos_abiertos, i);
			if(strcmp(nombre_archivo_pcb , archivo_pcb->nombre_archivo) == 0){
				 return archivo_pcb;
			}
		}
	}
	return NULL;
}

t_pcb* buscar_y_remover_pcb_por_pid(int un_pid){
	t_pcb* un_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_new);
		if(list_any_satisfy(lista_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_new, (void*)__buscar_pcb);
			list_remove_element(lista_new, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_new);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_ready);
		if(list_any_satisfy(lista_ready, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_ready, (void*)__buscar_pcb);
			list_remove_element(lista_ready, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_ready);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exec);
		if(list_any_satisfy(lista_execute, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_execute, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exec);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exit);
		if(list_any_satisfy(lista_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exit);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_blocked);
		if(list_any_satisfy(lista_blocked, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_blocked, (void*)__buscar_pcb);
			list_remove_element(lista_blocked, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_blocked);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		un_pcb = NULL;
		log_error(kernel_logger, "PID no encontrada en ninguna lista");
	}
	return un_pcb;
}



