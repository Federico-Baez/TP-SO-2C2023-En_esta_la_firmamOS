#include "../include/planificador_largo_plazo.h"


static void _plp_planifica(){
//	bool llamar_pcp = false;

	//Fijarse si la lista NEW tiene elementos, Si tiene elementos, sacar al 1ro
	pthread_mutex_lock(&mutex_core);
	t_pcb* una_pcb = NULL;

	//sacar un elemento y agregarlo a READY solo si GMMP lo permite
	if(GRADO_MULTIPROGRAMACION_INI > procesos_en_core){
		//Remover PCB de NEW si existen elementos
		pthread_mutex_lock(&mutex_lista_new);
		if(!list_is_empty(lista_new)){
			una_pcb = list_remove(lista_new, 0); //Sale por FIFO
		}
//		pthread_mutex_unlock(&mutex_lista_new);

		if(una_pcb != NULL){
			//Agregando PCB a READY
			pthread_mutex_lock(&mutex_lista_ready);
			list_add(lista_ready, una_pcb);
//			una_pcb->estado = READY;
			cambiar_estado(una_pcb, READY);
			pthread_mutex_unlock(&mutex_lista_ready);

			//Sumarle +1 a los procesos en Core
			procesos_en_core++;

			//Enviar Mensaje a memoria para q' inicialice estructuras
			t_paquete* un_paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
			cargar_string_al_super_paquete(un_paquete, una_pcb->path);
			cargar_int_al_super_paquete(un_paquete, una_pcb->size);
			cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
			enviar_paquete(un_paquete, fd_memoria);
			log_info(kernel_logger, "Se aviso a Memoria del nuevo proceso");
			eliminar_paquete(un_paquete);

//			llamar_pcp = true;
		}

		pthread_mutex_unlock(&mutex_lista_new);
	}
	pthread_mutex_unlock(&mutex_core);

	ejecutar_en_un_hilo_nuevo_detach((void*)pcp_planificar_corto_plazo, NULL);
}


//===================================================0
// solo 2 funciones disponibles para compartir

/*Planificador a Largo Plazo para NEW -> READY*/
void plp_planificar_proceso_nuevo(t_pcb* una_pcb){
//	pausador();

	if(una_pcb != NULL){
		//Asignando PCB al estado NEW
		pthread_mutex_lock(&mutex_lista_new);
		list_add(lista_new, una_pcb);
		una_pcb->estado = NEW;
		pthread_mutex_unlock(&mutex_lista_new);
	}
	pausador();
	//Verificar si puedo pasar a READY segun el GMMP
	_plp_planifica();


}

/*Planificador a Largo Plazo para [XX] -> EXIT */
void plp_planificar_proceso_exit(t_pcb* una_pcb){
	pausador();
//	log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);

	//Considerar que al momento de sacar un estado a Exit
	//sin importar en la lista que se encuentre,
	//hay que proceder de formas distintas
	//Si esta en bloqueado habria que liberar recursos de la PCB
	//Si esta ne ready habria que llamar al plp_new...

//	t_pcb* tempora_pcb;
	switch (una_pcb->estado) {
		case NEW:
			pthread_mutex_lock(&mutex_lista_new);
			//Esto control verifica que esa PCB siga en la lista
			if(list_remove_element(lista_new, una_pcb)){
				pthread_mutex_lock(&mutex_lista_exit);
				list_add(lista_exit, una_pcb);
//				una_pcb->estado = EXIT;
				cambiar_estado(una_pcb, EXIT);
				liberar_todos_los_recursos_de_una_pcb(una_pcb);
				pthread_mutex_unlock(&mutex_lista_exit);
			}else{
				log_error(kernel_logger, "PCB no encontradad en NEW");
			}
			pthread_mutex_unlock(&mutex_lista_new);
			log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);
			break;
		case READY:
			pthread_mutex_lock(&mutex_lista_ready);
			//Esto control verifica que esa PCB siga en la lista
			if(list_remove_element(lista_ready, una_pcb)){
				pthread_mutex_lock(&mutex_lista_exit);
				list_add(lista_exit, una_pcb);
//				una_pcb->estado = EXIT;
				cambiar_estado(una_pcb, EXIT);
				liberar_todos_los_recursos_de_una_pcb(una_pcb);
				pthread_mutex_unlock(&mutex_lista_exit);
			}else{
				log_error(kernel_logger, "PCB no encontradad en READY");
			}
			pthread_mutex_unlock(&mutex_lista_ready);
			//-----------------------
			pthread_mutex_lock(&mutex_core);
			procesos_en_core--;
			pthread_mutex_unlock(&mutex_core);
			//-----------------------
			avisar_a_memoria_para_liberar_estructuras(una_pcb);
			_plp_planifica();
			//[FALTA] ENVIAR MENSAJE A MEMORIA PARA QUE LIBERE ESTRUCTURAS
			log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);
			break;
		case EXEC://[FALTA] PLP_EXIT_EXECUTE
			pthread_mutex_lock(&mutex_lista_exec);
			batisenal_exit = true;
			//Enviar un interrupt
			t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
			cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
			cargar_int_al_super_paquete(un_paquete, una_pcb->ticket);//<<<<<<<<<<<<
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_logger, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", una_pcb->pid, una_pcb->ticket);

			pthread_mutex_unlock(&mutex_lista_exec);
			break;
		case BLOCKED://[FALTA] PLP_EXIT_BLOCKED
			pthread_mutex_lock(&mutex_lista_blocked);
			//[FALTA] Sacar de la LISTA_RECURSOS, LA_PCB de su respectiva lista de bloqueados
			pthread_mutex_unlock(&mutex_lista_blocked);
//			//-----------------------
//			pthread_mutex_lock(&mutex_core);
//			procesos_en_core--;
//			pthread_mutex_unlock(&mutex_core);
//			//-----------------------
			//Avisar a memoria para liberar estructuras
			//_plp_planifica();
			break;
		case EXIT:
			pthread_mutex_lock(&mutex_lista_exit);
			//Esto control verifica que esa PCB siga en la lista
			if(esta_pcb_en_una_lista_especifica(lista_exit, una_pcb)){
				log_info(kernel_logger, "La PCB ya se encuentra en el estado EXIT");
			}else{
				log_error(kernel_logger, "La PCB_%d NO se encuentra en la lista EXIT - Esto es RARO", una_pcb->pid);
				exit(EXIT_FAILURE);
			}
			pthread_mutex_unlock(&mutex_lista_exit);
			break;
		default:
			log_error(kernel_logger, "JAMAS DE LOS JAMASES DEBERIAS LLEGAR AQUI");
			break;
	}

//	log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);

	//Pasar PCB al estado EXIT

	//Libear recursos

	//Dar aviso a MEMORIA para que libere sus estructuras

}
