#include "../include/planificador_largo_plazo.h"


void plp_planifica(){
	t_pcb* un_pcb = NULL;

	pthread_mutex_lock(&mutex_core);
	pthread_mutex_lock(&mutex_lista_new);
	log_warning(kernel_logger, "ESTOY POR ENTRAR AL IF - CORE %d", procesos_en_core);
	if(GRADO_MULTIPROGRAMACION_INI > procesos_en_core && !list_is_empty(lista_new)){
		log_info(kernel_logger, "Entre al IF");

		//Remover PCB de NEW si existen elementos
		un_pcb = list_remove(lista_new, 0); //Sale por FIFO

		if(un_pcb != NULL){
			//Enviar Mensaje a memoria para que inicialice estructuras
			t_paquete* un_paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
			cargar_string_al_super_paquete(un_paquete, un_pcb->path);
			cargar_int_al_super_paquete(un_paquete, un_pcb->size);
			cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
			log_info(kernel_logger, "Se aviso a Memoria del nuevo proceso");
			enviar_paquete(un_paquete, fd_memoria);

			// 	Lo comento hasta que hagan la funcion.
			sem_wait(&sem_estructura_iniciada);


			//Agregando PCB a READY
//			transferir_from_actual_to_siguiente(un_pcb, lista_ready, mutex_lista_ready, READY);
			pthread_mutex_lock(&mutex_lista_ready);
			cambiar_estado(un_pcb, READY);
			list_add(lista_ready, un_pcb);
			log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", un_pcb->pid, estado_to_string(NEW), estado_to_string(un_pcb->estado));
			char* pids_en_ready = lista_pids_en_ready(lista_ready, mutex_lista_ready);
			log_info(kernel_log_obligatorio, "Cola Ready FIFO: %s", pids_en_ready);
			free(pids_en_ready);

			//Sumarle +1 a los procesos en Core
			procesos_en_core++;

			pthread_mutex_unlock(&mutex_lista_ready);

			eliminar_paquete(un_paquete);
		}

	}
	pthread_mutex_unlock(&mutex_lista_new);
	pthread_mutex_unlock(&mutex_core);

	ejecutar_en_un_hilo_nuevo_detach((void*)pcp_planificar_corto_plazo, NULL);
}

//===================================================0
// solo 2 funciones disponibles para compartir

/*Planificador a Largo Plazo para NEW -> READY*/
void plp_planificar_proceso_nuevo(t_pcb* un_pcb){
//	pausador();

	if(un_pcb != NULL){
		//Asignando PCB al estado NEW
		pthread_mutex_lock(&mutex_lista_new);
		list_add(lista_new, un_pcb);
		un_pcb->estado = NEW;
		pthread_mutex_unlock(&mutex_lista_new);
		log_info(kernel_log_obligatorio, "Se crea el proceso %d en NEW", un_pcb->pid);

	}
	pausador();
	//Verificar si puedo pasar a READY segun el GMMP
	ejecutar_en_un_hilo_nuevo_detach((void*)plp_planifica, NULL);
//	plp_planifica();
}

/*Planificador a Largo Plazo para [XX] -> EXIT*/

void plp_planificar_proceso_exit(int pid){
	// Solo remuevo los que estan en: NEW-READY-BLOCKED, si esta en: EXEC-EXIT
	// se procede de otra manera.
	t_pcb* un_pcb = buscar_y_remover_pcb_por_pid(pid);

	pthread_mutex_lock(&mutex_flag_proceso_desalojado);
	pthread_mutex_lock(&mutex_flag_finalizar_proceso);
	if(un_pcb->estado == 2 && flag_proceso_desalojado){
		flag_finalizar_proceso = true;
	}
	pthread_mutex_unlock(&mutex_flag_finalizar_proceso);
	pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

	pausador();
	if(un_pcb != NULL){
		switch(un_pcb->estado){
		case NEW:

			pthread_mutex_lock(&mutex_lista_new);
			// Quizas hay que agregar un semaforo que este a la espera de que se liberen las estructuras, para que no se genere condicion de carrera
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_new);

			break;
		case READY:

			pthread_mutex_lock(&mutex_lista_ready);
			liberar_recursos_pcb(un_pcb);
			//liberar_archivo_pcb(un_pcb);
			// Quizas hay que agregar un semaforo que este a la espera de que se liberen las estructuras, para que no se genere condicion de carrera
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_ready);

			//-----------------------
			pthread_mutex_lock(&mutex_core);
			procesos_en_core--;
			pthread_mutex_unlock(&mutex_core);
			//-----------------------

			plp_planificar_proceso_nuevo(NULL);
			break;
		case EXEC:

			pthread_mutex_lock(&mutex_flag_proceso_desalojado);
			pthread_mutex_lock(&mutex_flag_finalizar_proceso);
			if(flag_proceso_desalojado && flag_exit){
				flag_proceso_desalojado = false;
			}else{
	//			Enviar un interrupt
				flag_finalizar_proceso = true;
				log_info(kernel_logger, "Envio interrupcion");
				sem_post(&sem_enviar_interrupcion);
			}
			pthread_mutex_unlock(&mutex_flag_finalizar_proceso);
			pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

			break;
		case BLOCKED:

			pthread_mutex_lock(&mutex_lista_blocked);
			liberar_recursos_pcb(un_pcb);
			//liberar_archivo_pcb(un_pcb);
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_blocked);

			pthread_mutex_lock(&mutex_core);
			procesos_en_core--;
			pthread_mutex_unlock(&mutex_core);

			plp_planificar_proceso_nuevo(NULL);

			break;
		case EXIT:

			pthread_mutex_lock(&mutex_lista_exit);
			//Este control verifica que esa PCB siga en la lista
			if(esta_pcb_en_una_lista_especifica(lista_exit, un_pcb)){
				log_info(kernel_logger, "El PCB ya se encuentra en el estado EXIT");
			}else{
				log_error(kernel_logger, "El PCB_%d No se encuentra en la lista EXIT", un_pcb->pid);
				exit(EXIT_FAILURE);
			}
			pthread_mutex_unlock(&mutex_lista_exit);
			break;
		default:
			log_error(kernel_logger, "El PCB no tiene ESTADO");
			break;
		}
	}else
		log_error(kernel_logger, "CONSOLA - No se encontro el PID en ningun lado");
}
