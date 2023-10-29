#include "../include/k_servicios_kernel.h"

//AGREGAR FUNCIONES FUNCIONALES


void pausador(){
	pthread_mutex_lock(&mutex_pausa);
	if(var_pausa == 1){
		log_warning(kernel_logger, "Planificadores Pausados");
		sem_wait(&sem_pausa);
	}
	pthread_mutex_unlock(&mutex_pausa);
}

void cambiar_estado(t_pcb* una_pcb, est_pcb nex_state){
	una_pcb->estado = nex_state;
}

int generar_ticket(){
	int valor_ticket;
	pthread_mutex_lock(&mutex_ticket);
	var_ticket++;
	valor_ticket = var_ticket;
	pthread_mutex_unlock(&mutex_ticket);
	return valor_ticket;
}

