#include "../include/k_servicios_kernel.h"

//AGREGAR FUNCIONES FUNCIONALES


void pausador(){
	pthread_mutex_lock(&mutex_pausa);
	if(var_pausa == 1){
		log_warning(kernel_log_obligatorio, "PAUSA DE PLANIFICACIÓN"); // --> Tiene que ser log_info, por ahora lo dejamos asi para que se note
		sem_wait(&sem_pausa);
	}
	pthread_mutex_unlock(&mutex_pausa);
}

int generar_ticket(){
	int valor_ticket;
	pthread_mutex_lock(&mutex_ticket);
	var_ticket++;
	valor_ticket = var_ticket;
	pthread_mutex_unlock(&mutex_ticket);
	return valor_ticket;
}

char* algoritmo_to_string(t_algoritmo algoritmo){

	switch(algoritmo){
	case FIFO:
		return "FIFO";
		break;
	case ROUNDROBIN:
		return "RR";
		break;
	case PRIORIDADES:
		return "PRIORIDADES";
		break;
	default:
		return "ERROR";
	}
}
