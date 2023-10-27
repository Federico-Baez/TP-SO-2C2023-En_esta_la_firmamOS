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



