#include "../include/planificador_largo_plazo.h"

//Esto es para que se liberen todos los recursos asignados
static void _liberar_todos_los_recursos_de_una_pcb(t_pcb* una_pcb){
	while(!list_is_empty(una_pcb->lista_recursos_pcb)){
		t_recurso* un_recurso = list_remove(una_pcb->lista_recursos_pcb, 0);

		//Aumento el valor del recurso en +1
		pthread_mutex_lock(&mutex_recurso);
		un_recurso->recurso_valor = un_recurso->recurso_valor + 1;
		pthread_mutex_unlock(&mutex_recurso);

		//[FALTA] LLamar al gestor de recursos para que explore y libere a alguna PCB en espera
	}
}

static void _avisar_a_memoria_para_liberar_estructuras(t_pcb* una_pcb){
	t_paquete* un_paquete = crear_super_paquete(LIBERAR_ESTRUCTURA_KM);
	cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
	log_info(kernel_logger, "Mensaje a MEMORIA: LIBERAR_ESTRUCTURA_KM [PID: %d]", una_pcb->pid);
}

static void _plp_planifica(){
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
			una_pcb->estado = READY;
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

			//[FALTA] Decidir si llamamos en un hilo al PCP para que comience a interactuar con CPU
		}

		pthread_mutex_unlock(&mutex_lista_new);
	}
	pthread_mutex_unlock(&mutex_core);
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
				una_pcb->estado = EXIT;
				_liberar_todos_los_recursos_de_una_pcb(una_pcb);
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
				una_pcb->estado = EXIT;
				_liberar_todos_los_recursos_de_una_pcb(una_pcb);
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
			_avisar_a_memoria_para_liberar_estructuras(una_pcb);
			_plp_planifica();
			//[FALTA] ENVIAR MENSAJE A MEMORIA PARA QUE LIBERE ESTRUCTURAS
			log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);
			break;
		case EXEC://[FALTA] PLP_EXIT_EXECUTE
			pthread_mutex_lock(&mutex_lista_exec);
			//1. Marcar la PCB como EXIT para que cuando vuelva de CPU Lo lleva a la lista EXIT
			una_pcb->estado = EXIT; // Al volver a pasar se verificara que esta en la lista EXIT, si no se la buscara en EXECUTE

			//2. Tendria que mandarse un interrup
			t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
			cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_logger, "Mensaje a CPU: FORZAR_DESALOJO_KC [PID: %d]", una_pcb->pid);

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
				/*Al buscar en la lista EXECUTE, estamos preguntando si
				 * la PCB fue desalojada por el usaurio*/
				pthread_mutex_lock(&mutex_lista_exec);
				if(esta_pcb_en_una_lista_especifica(lista_execute, una_pcb)){
					//Proceder a desalojar
					list_remove_element(lista_execute, una_pcb);
					list_add(lista_exit, una_pcb);
					_liberar_todos_los_recursos_de_una_pcb(una_pcb);
					//-----------------------
					pthread_mutex_lock(&mutex_core);
					procesos_en_core--;
					pthread_mutex_unlock(&mutex_core);
					//-----------------------

					//Aviar a memoria que libere estructuras
					_avisar_a_memoria_para_liberar_estructuras(una_pcb);
				}else{
					log_error(kernel_logger, "PCB no encontrado ni en EXIT, ni en EXECUTE. Algo debe estar muy mal");
				}
				pthread_mutex_unlock(&mutex_lista_exec);
			}
			pthread_mutex_unlock(&mutex_lista_exit);
			_plp_planifica();
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
