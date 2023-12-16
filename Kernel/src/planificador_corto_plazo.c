#include "../include/planificador_corto_plazo.h"

static void _programar_interrupcion_por_quantum(t_pcb* un_pcb){
	int ticket_referencia = un_pcb->ticket;
	usleep(QUANTUM*1000);

	/*Esta comprobacion de ticket es en caso de que la PCB haya salido de CPU,
	 * Puesto en READY y por casulaidades de la vida haya vuelto a la CPU
	 * Y al despertar este hilo, primero verifique que la PCB objetivo, no haya
	 * salido de la CPU, y esto lo resolvemos con el ticket.
	 * Porque si salio la misma PCB y volvio a entrar, significa que el proceso tiene
	 * nuevo ticket*/
	log_info(kernel_logger, "Me desperte");
	pthread_mutex_lock(&mutex_ticket);
	if(ticket_referencia == var_ticket){
		pthread_mutex_lock(&mutex_flag_exit);
		if(!flag_exit){

			sem_post(&sem_enviar_interrupcion);
		}
//		flag_exit = false;
		pthread_mutex_unlock(&mutex_flag_exit);
	}
	pthread_mutex_unlock(&mutex_ticket);
}

static void _atender_RR_FIFO(){
	//Verificar que la lista de EXECUTE esté vacía

	pthread_mutex_lock(&mutex_lista_exec);
	if(list_is_empty(lista_execute)){
		t_pcb* un_pcb = NULL;

		//Verificar que haya elementos en la lista de READY
		pthread_mutex_lock(&mutex_lista_ready);
		if(!list_is_empty(lista_ready)){
			un_pcb = list_remove(lista_ready, 0);
		}
		pthread_mutex_unlock(&mutex_lista_ready);

		if(un_pcb != NULL){
			list_add(lista_execute, un_pcb);
			cambiar_estado(un_pcb, EXEC);
			log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: READY - Estado Actual: EXEC", un_pcb -> pid);
			un_pcb->ticket = generar_ticket();

			_enviar_pcb_a_CPU_por_dispatch(un_pcb);

			if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "RR") == 0){
				pthread_mutex_lock(&mutex_flag_exit);
				flag_exit = false;
				pthread_mutex_unlock(&mutex_flag_exit);
				ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, un_pcb);
			}
		}else{
			log_warning(kernel_logger, "Lista de READY vacía");
		}
	}
	pthread_mutex_unlock(&mutex_lista_exec);
}

static void _atender_PRIORIDADES(){
	t_pcb* un_pcb = NULL;

	//Tomo el elemento de mayor prioridad
	pthread_mutex_lock(&mutex_lista_ready);
	if(list_size(lista_ready) == 1){
		un_pcb = list_get(lista_ready, 0);
	}else{
		un_pcb = list_get_maximum(lista_ready, (void*)__maxima_prioridad);
	}

	pthread_mutex_lock(&mutex_lista_exec);
	if(!list_is_empty(lista_execute)){
		/*Si hay algun elemento ejecutando en EXEC, lo comparo con el
		de mayor prioridad de la lista de READY*/
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		if(un_pcb->prioridad < pcb_execute->prioridad){
			log_warning(kernel_logger, "ENVIO INTERRUPCION: %d", un_pcb->pid);

			sem_post(&sem_enviar_interrupcion);
		}
	}else if(!list_is_empty(lista_ready)){
			if(list_remove_element(lista_ready, un_pcb)){

				list_add(lista_execute, un_pcb);
				un_pcb->ticket = generar_ticket();
				cambiar_estado(un_pcb, EXEC);
				log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: READY - Estado Actual: EXEC", un_pcb -> pid);

				_enviar_pcb_a_CPU_por_dispatch(un_pcb);

			}else{
				log_error(kernel_logger, "No se encontro el PCB con mayor PRIORIDAD");
				exit(EXIT_FAILURE);
			}
	}
	pthread_mutex_unlock(&mutex_lista_exec);
	pthread_mutex_unlock(&mutex_lista_ready);
}

t_pcb* __maxima_prioridad(t_pcb* void_1, t_pcb* void_2){
		if(void_1->prioridad <= void_2->prioridad) return void_1;
		else return void_2;
}

void pcp_planificar_corto_plazo(){
	pausador();
	int flag_lista_ready_vacia = 0;

	pthread_mutex_lock(&mutex_lista_ready);
	if(list_is_empty(lista_ready)){
		flag_lista_ready_vacia = 1;
	}
	pthread_mutex_unlock(&mutex_lista_ready);

	if(flag_lista_ready_vacia == 0){

		switch (ALGORITMO_PLANIFICACION) {
			case FIFO:
				_atender_RR_FIFO();
				break;
			case ROUNDROBIN:
				_atender_RR_FIFO();
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
}
