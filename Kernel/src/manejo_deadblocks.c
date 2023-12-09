#include "../include/manejo_deadblocks.h"

#include <stdbool.h>
#include <stdlib.h>

void deteccion_deadlock(){
	while(1){
//		sem_wait(&sem_nuevo_en_block);
		pthread_mutex_lock(&mutex_lista_blocked);
		t_list* lista_posibles_deadlocks_recurso = obtener_lista_pcbs_block_recursos();
		t_list* lista_pcbs_en_deadlock = list_create();

		bool hay_ciclo = false;

		t_pcb* pcb = list_get(lista_posibles_deadlocks_recurso, 0);
		while(1){
			t_recurso* recurso = list_get(pcb->lista_recursos_pcb, list_size(pcb->lista_recursos_pcb)-1);
			// Ver si es recurso asignado o bloqueado
			if(esta_pcb_en_una_lista_especifica(lista_posibles_deadlocks_recurso, recurso->pcb_asignado)){
				list_add(lista_pcbs_en_deadlock, pcb);
				pcb = recurso->pcb_asignado;
				if(pcb == list_get(lista_posibles_deadlocks_recurso, 0)){
					hay_ciclo = true;
					break;
				}
			}else{
				int j = 0;
				list_remove_element(lista_posibles_deadlocks_recurso, pcb);
				while(!list_is_empty(lista_pcbs_en_deadlock)){
					t_pcb* un_pcb = list_remove(lista_pcbs_en_deadlock, j);
					list_remove_element(lista_posibles_deadlocks_recurso, un_pcb);
					j++;
				}
				break;
			}
		}
		if(hay_ciclo){
			char* log_proceso = string_new();
			for(int i = 0; i < list_size(lista_pcbs_en_deadlock); i++){
				t_pcb* pcb = list_get(lista_pcbs_en_deadlock, i);
				logear_proceso_en_deadlock(log_proceso, pcb);
				free(log_proceso);
			}
		}else
			log_info(kernel_logger, "No hay procesos en DEADLOCK");

		pthread_mutex_unlock(&mutex_lista_blocked);
	}
}


t_list* obtener_lista_pcbs_block_recursos(){
	t_list* lista_posibles_deadlocks_recurso = list_create();
	pthread_mutex_lock(&mutex_lista_blocked);

	for(int i = 0; i < list_size(lista_blocked); i++){
		t_pcb* pcb = list_get(lista_blocked, i);
		if(pcb->motivo_block == 1){
			list_add(lista_posibles_deadlocks_recurso, pcb);
		}
	}
	pthread_mutex_unlock(&mutex_lista_blocked);

	return lista_posibles_deadlocks_recurso;
}

t_list* obtener_lista_pcbs_block_archivos(){
	t_list* lista_posibles_deadlocks_archivo = list_create();


	for(int i = 0; i < list_size(lista_blocked); i++){
		t_pcb* pcb = list_get(lista_blocked, i);
		if(pcb->motivo_block == 2){
			list_add(lista_posibles_deadlocks_archivo, pcb);
		}
	}
	return lista_posibles_deadlocks_archivo;
}

void logear_proceso_en_deadlock(char* log_proceso, t_pcb* pcb){

	string_append(&log_proceso,string_itoa(pcb->pid));
	string_append(&log_proceso," -  Recursos en posesión ");
	for(int j = 0; j < list_size(pcb->lista_recursos_pcb)-1; j++){
		t_recurso* recurso = list_get(pcb->lista_recursos_pcb, j);
		string_append(&log_proceso,recurso->recurso_name);
		if(j == list_size(pcb->lista_recursos_pcb)-2){

		}else{
			string_append(&log_proceso,", ");
		}
	}
	t_recurso* recurso = list_get(pcb->lista_recursos_pcb, list_size(pcb->lista_recursos_pcb)-1);
	string_append(&log_proceso," - Recurso requqerido: ");
	string_append(&log_proceso,recurso->recurso_name);

	log_info(kernel_log_obligatorio, "Deadlock Detectado: %s", log_proceso);
}

