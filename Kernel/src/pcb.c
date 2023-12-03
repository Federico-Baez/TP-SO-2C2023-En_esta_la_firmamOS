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

	return nuevo_PCB;
}


void destruir_pcb(t_pcb* un_pcb){
	free(un_pcb->path);
	free(un_pcb->registros_CPU);
	list_destroy(un_pcb->lista_recursos_pcb);
	pthread_mutex_destroy(&un_pcb->mutex_lista_recursos);
	free(un_pcb);
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

// Para atender las INSTRUCCIONES DEL CPU
t_pcb* recibir_contexto_de_ejecucion(t_buffer* un_buffer){
	int recibe_pid = recibir_int_del_buffer(un_buffer);
	t_pcb* pcb = buscar_pcb_por_pid(recibe_pid);
	if(pcb == NULL){
		log_error(kernel_logger, "PID recibido no coincide con PCB de ninguna lista");
		exit(EXIT_FAILURE);
	}

	//Controlando que la PCB encontrada corresponde a la lista execute
	if(!esta_pcb_en_una_lista_especifica(lista_execute, pcb)){
		log_error(kernel_logger, "PCB_%d (%d) - No se encuentró en la lista EXECUTE ", pcb->pid, pcb->estado);
		exit(EXIT_FAILURE);
	}

	pcb->program_counter = recibir_int_del_buffer(un_buffer);
	uint32_t* re_RX;
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	pcb->registros_CPU->AX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	pcb->registros_CPU->BX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	pcb->registros_CPU->CX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	pcb->registros_CPU->DX = *re_RX; free(re_RX);

	return pcb;
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


void transferir_from_actual_to_siguiente(t_pcb* pcb, t_list* lista_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente){
	char*  estado_anterior = estado_to_string(pcb->estado);
	char* siguente_estado = estado_to_string(estado_siguiente);

	cambiar_estado(pcb, estado_siguiente);

	agregar_pcb_lista(pcb, lista_siguiente, mutex_siguiente);

	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb -> pid, estado_anterior, siguente_estado);


	if(strcmp(siguente_estado, "READY") == 0){
		pthread_mutex_lock(&mutex_siguiente);
		if(strcmp(estado_anterior, "NEW") == 0){
			char* pids_en_ready = lista_pids_en_estado(lista_ready, mutex_lista_ready);
			log_info(kernel_log_obligatorio, "Cola Ready FIFO: %s", pids_en_ready);
			free(pids_en_ready);
		}else{
			char* pids_en_ready = lista_pids_en_estado(lista_ready, mutex_lista_ready);
			log_info(kernel_log_obligatorio, "Cola Ready %s: %s", algoritmo_to_string(ALGORITMO_PLANIFICACION), pids_en_ready);
			free(pids_en_ready);
		}
		pthread_mutex_unlock(&mutex_siguiente);
	}
	free(estado_anterior);
	free(siguente_estado);
}

void agregar_pcb_lista(t_pcb* pcb, t_list* lista_estado, pthread_mutex_t mutex_lista){
	pthread_mutex_lock(&mutex_lista);
	list_add(lista_estado, pcb);
	pthread_mutex_unlock(&mutex_lista);
}

char* lista_pids_en_estado(t_list* lista_estado, pthread_mutex_t mutex_lista){
	int id_process;
	char* pids_en_string = string_new();
	string_append(&pids_en_string, "[");

	pthread_mutex_lock(&mutex_lista);
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
	pthread_mutex_unlock(&mutex_lista);

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

			}
			pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
			un_recurso->instancias++;
			asignar_recurso_liberado_pcb(un_recurso);
		}
		pthread_mutex_unlock(&pcb->mutex_lista_recursos);
	}
	list_destroy(recursos_del_pcb);
}

void asignar_recurso_liberado_pcb(t_recurso* un_recurso){
	if(!list_is_empty(un_recurso->lista_bloqueados)){
		t_pcb* pcb_liberado = list_remove(un_recurso->lista_bloqueados,0);

		pthread_mutex_lock(&un_recurso->mutex_asignados);
		list_add(un_recurso->lista_asignados, pcb_liberado);
		pthread_mutex_unlock(&un_recurso->mutex_asignados);

		transferir_from_actual_to_siguiente(pcb_liberado, lista_ready, mutex_lista_ready, READY);

		un_recurso->instancias--;
	}else
		log_warning(kernel_logger, "La lista de BLOQUEADOS del RECURSO esta vacia");
}










