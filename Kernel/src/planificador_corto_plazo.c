#include "../include/planificador_corto_plazo.h"


static void _enviar_pcb_a_CPU_por_dispatch(t_pcb* una_pcb){
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


static void _programar_interrupcion_por_quantum(t_pcb* una_pcb){
	int ticket_referencia = una_pcb->ticket;
	sleep(QUANTUM/1000);

	/*Esta comprobacion de ticket es en caso de que la PCB haya salido de CPU,
	 * Puesto en READY y por casulaidades de la vida haya vuelto a la CPU
	 * Y al despertar este hilo, primero verifique que la PCB objetivo, no haya
	 * salido de la CPU, y esto lo resolvemos con el ticket.
	 * Porque si salio la misma PCB y volvio a entrar, significa que el proceso tiene
	 * nuevo ticket*/
	if(ticket_referencia == var_ticket){

		if(!batisenal_exit){
			t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
			cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
			cargar_int_al_super_paquete(un_paquete, ticket_referencia);
			cargar_string_al_super_paquete(un_paquete, "ALGORITMO_QUANTUM");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
		}

	}

}


static void _atender_FIFO(){
	//Verificar que la lista de EXECUTE esté vacía
	pthread_mutex_lock(&mutex_lista_exec);
	if(list_is_empty(lista_execute)){
		t_pcb* una_pcb = NULL;
		pthread_mutex_lock(&mutex_lista_ready);
		//Verificar que haya elementos en la lista de READY
		if(!list_is_empty(lista_ready)){
			una_pcb = list_remove(lista_ready, 0);
		}
		pthread_mutex_unlock(&mutex_lista_ready);

		if(una_pcb != NULL){
			list_add(lista_execute, una_pcb);
			una_pcb->ticket = generar_ticket();
			cambiar_estado(una_pcb, EXEC);
			_enviar_pcb_a_CPU_por_dispatch(una_pcb);
		}else{
			log_warning(kernel_logger, "Lista de READY vacía");
		}
		//Si devuelve NULL, significa que la lista de READY esta vacia
		//Por lo que no se haria nada
	}

	pthread_mutex_unlock(&mutex_lista_exec);
}

static void _atender_RR(){
	//Verificar que la lista de EXECUTE esté vacía
	pthread_mutex_lock(&mutex_lista_exec);
	if(list_is_empty(lista_execute)){
		t_pcb* una_pcb = NULL;

		//Verificar que haya elementos en la lista de READY
		pthread_mutex_lock(&mutex_lista_ready);
		if(!list_is_empty(lista_ready)){
			una_pcb = list_remove(lista_ready, 0);
		}
		pthread_mutex_unlock(&mutex_lista_ready);

		if(una_pcb != NULL){
			list_add(lista_execute, una_pcb);
			una_pcb->ticket = generar_ticket();
			cambiar_estado(una_pcb, EXEC);
			_enviar_pcb_a_CPU_por_dispatch(una_pcb);
			ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, una_pcb);
		}else{
			log_warning(kernel_logger, "Lista de READY vacía");
		}
		//Si devuelve NULL, significa que la lista de READY esta vacia
		//Por lo que no se haria nada
	}

	pthread_mutex_unlock(&mutex_lista_exec);
}


static void _atender_PRIORIDADES(){
	t_pcb* pcb_execute;
	t_pcb* una_pcb = NULL;

	t_pcb* __maxima_prioridad(t_pcb* void_1, t_pcb* void_2){
		if(void_1->prioridad >= void_2->prioridad) return void_1;
		else return void_2;
	}

	//Verificar que haya elementos en la lista de READY
	pthread_mutex_lock(&mutex_lista_exec);
	pthread_mutex_lock(&mutex_lista_ready);
	if(!list_is_empty(lista_ready)){

		//Tomo el elemento de mayor prioridad
		if(list_size(lista_ready) == 1){
			una_pcb = list_get(lista_ready, 0);
		}else{
			una_pcb = list_get_maximum(lista_ready, (void*)__maxima_prioridad);
		}

		//Consulto si la prioridad le gana al que esta en EXECUTE
//		pthread_mutex_lock(&mutex_lista_exec); //<=========
		//Antes pregunta si hay alguna PCB ejecutando
		if(!list_is_empty(lista_execute)){
			/*Si hay algun elemento ejecutando en EXEC, lo comparo con el
			de mayor prioridad de la lista de READY*/
			pcb_execute = list_get(lista_execute, 0);
			if(una_pcb->prioridad > pcb_execute->prioridad){

				//Controlo que la PCB elegida sea siempre la de maxima prioridad.
				//Si es que viniera otra de mayor prioridad, haría el cambio
				if(hay_pcb_elegida){
					if(una_pcb->prioridad > pcb_prioritaria->prioridad){
						pcb_prioritaria = una_pcb;
					}
				}else{

					//Control para enviar interrupcion solo cuando la CPU estea en uso
					if(CPU_en_uso){
						/*Significa que, aun no habia algun proceso elegido para la interrupcion*/
						hay_pcb_elegida = true;
						pcb_prioritaria = una_pcb;

						//Enviar interrupcion por interrupt
						t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
						cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
						cargar_int_al_super_paquete(un_paquete, pcb_execute->ticket);
						cargar_string_al_super_paquete(un_paquete, "ALGORITMO_PRIORIDAD");
						enviar_paquete(un_paquete, fd_cpu_interrupt);
						eliminar_paquete(un_paquete);
					}

				}

			}
		}else {
			if(list_remove_element(lista_ready, una_pcb)){
				list_add(lista_execute, una_pcb);
				una_pcb->ticket = generar_ticket();
				cambiar_estado(una_pcb, EXEC);
				_enviar_pcb_a_CPU_por_dispatch(una_pcb);
				CPU_en_uso = true;
			}else{
				log_error(kernel_logger, "[PCP_PRIORIDAD] Algo salio muy mal en la logica de sacar de READY");
				exit(EXIT_FAILURE);
			}
		}
//		pthread_mutex_unlock(&mutex_lista_exec); //<=========

	}else{
		log_warning(kernel_logger, "Lista de READY vacía");
	}
	pthread_mutex_unlock(&mutex_lista_ready);
	pthread_mutex_unlock(&mutex_lista_exec);

}

void pcp_planificar_corto_plazo(){
	pausador();

	switch (ALGORITMO_PLANIFICACION) {
		case FIFO:
			_atender_FIFO();
			break;
		case ROUNDROBIN:
			_atender_RR();
			break;
		case PRIORIDADES:
			_atender_PRIORIDADES();
			break;
		default:
			log_error(kernel_logger, "ALGORITMO DE CORTO PLAZO DESCONOCIDO");
			exit(EXIT_FAILURE);
			break;
	}
}
