#include "../include/planificador_largo_plazo.h"


void plp_planifica(){
//	bool llamar_pcp = false;

	//Fijarse si la lista NEW tiene elementos, Si tiene elementos, sacar al 1ro
	pthread_mutex_lock(&mutex_core);
	t_pcb* un_pcb = NULL;

	//sacar un elemento y agregarlo a READY solo si GMMP lo permite
	if(GRADO_MULTIPROGRAMACION_INI > procesos_en_core){
		//Remover PCB de NEW si existen elementos
		pthread_mutex_lock(&mutex_lista_new);
		if(!list_is_empty(lista_new)){
			un_pcb = list_remove(lista_new, 0); //Sale por FIFO
		}
		pthread_mutex_unlock(&mutex_lista_new);

		if(un_pcb != NULL){
			//Enviar Mensaje a memoria para q' inicialice estructuras
			t_paquete* un_paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
			cargar_string_al_super_paquete(un_paquete, un_pcb->path);
			cargar_int_al_super_paquete(un_paquete, un_pcb->size);
			cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
			log_info(kernel_logger, "Se aviso a Memoria del nuevo proceso");
			enviar_paquete(un_paquete, fd_memoria);

			sem_wait(&sem_estructura_iniciada);

			//Agregando PCB a READY
			transferir_from_actual_to_siguiente(un_pcb, lista_ready, mutex_lista_ready, READY);

			//Sumarle +1 a los procesos en Core
			procesos_en_core++;

			eliminar_paquete(un_paquete);
		}

	}
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
	}
	log_info(kernel_log_obligatorio, "Se crea el proceso %d en NEW", un_pcb->pid);

	pausador();
	//Verificar si puedo pasar a READY segun el GMMP
	plp_planifica();
}

/*Planificador a Largo Plazo para [XX] -> EXIT */
void plp_planificar_proceso_exit(t_pcb* un_pcb){
//	pausador();
//	log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);

	//Considerar que al momento de sacar un estado a Exit
	//sin importar en la lista que se encuentre,
	//hay que proceder de formas distintas
	//Si esta en bloqueado habria que liberar recursos de la PCB
	//Si esta ne ready habria que llamar al plp_new...

	int flag_esta_en_exit = 0;
	int flag_finalizo_exec = 0;

	// Quizas es necesario agregar esta opcion dentro de NEW,READY,EXEC y BLOCKED, porque si no hay mas procesos que entren a ready o esten
	// en ready, va a romper
//	pthread_mutex_lock(&mutex_lista_exec);
//	if(!list_is_empty(lista_execute)){
//		plp_planifica();
//	}
//	pthread_mutex_unlock(&mutex_lista_exec);


	switch (un_pcb->estado) {
		case NEW:
			pausador();
			pthread_mutex_lock(&mutex_lista_new);
			//Este control verifica que ese PCB siga en la lista
			if(list_remove_element(lista_new, un_pcb)){
				transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
			}else{
				log_error(kernel_logger, "PCB no encontrado en NEW");
			}
			pthread_mutex_unlock(&mutex_lista_new);
			break;
		case READY:
			pausador();
			pthread_mutex_lock(&mutex_lista_ready);
			//Este control verifica que ese PCB siga en la lista
			if(list_remove_element(lista_ready, un_pcb)){
				transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
				liberar_recursos_pcb(un_pcb);
			}else{
				log_error(kernel_logger, "PCB no encontrado en READY");
			}
			pthread_mutex_unlock(&mutex_lista_ready);
			//-----------------------
			pthread_mutex_lock(&mutex_core);
			procesos_en_core--;
			pthread_mutex_unlock(&mutex_core);
			//-----------------------
			avisar_a_memoria_para_liberar_estructuras(un_pcb);

			pthread_mutex_lock(&mutex_lista_new);
			if(!list_is_empty(lista_new)){
				plp_planifica();
			}
			pthread_mutex_unlock(&mutex_lista_new);

			//[FALTA] ENVIAR MENSAJE A MEMORIA PARA QUE LIBERE ESTRUCTURAS
			break;
		case EXEC://[FALTA] PLP_EXIT_EXECUTE

			pthread_mutex_lock(&mutex_flag_exit);
			flag_exit = true;
			pthread_mutex_unlock(&mutex_flag_exit);


//			Enviar un interrupt
			sem_post(&sem_enviar_interrupcion);

			flag_finalizo_exec = 1;

			break;
		case BLOCKED:
			pthread_mutex_lock(&mutex_lista_blocked);
			if(list_remove_element(lista_blocked,un_pcb)){
				transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
				liberar_recursos_pcb(un_pcb);
			}else
				log_warning(kernel_logger,"El PCB no se encuentra en BLOCEKD");
			pthread_mutex_unlock(&mutex_lista_blocked);


			pthread_mutex_lock(&mutex_core);
			procesos_en_core--;
			pthread_mutex_unlock(&mutex_core);

			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			pthread_mutex_lock(&mutex_lista_new);
			if(!list_is_empty(lista_new)){
				plp_planifica();
			}
			pthread_mutex_unlock(&mutex_lista_new);

			break;
		case EXIT:
			pausador();
			pthread_mutex_lock(&mutex_lista_exit);
			//Esto control verifica que esa PCB siga en la lista
			if(esta_pcb_en_una_lista_especifica(lista_exit, un_pcb)){
				flag_esta_en_exit = 1;
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

	if(flag_esta_en_exit == 0 && flag_finalizo_exec == 0)
		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: SUCCESS", un_pcb->pid);
}
