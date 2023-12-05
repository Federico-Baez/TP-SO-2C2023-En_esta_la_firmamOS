#include "../include/atender_conexiones.h"

static void estructuras_inciadas_en_memoria_rpta(t_buffer* unBuffer){
	char* recibir_mensaje = (char*)recibir_choclo_del_buffer(unBuffer);
	printf("[%d]> %s\n", (int)strlen(recibir_mensaje), recibir_mensaje);
	free(recibir_mensaje);
}

static void log_blocked_proceso(int pid_process, char* motivo_block){
	log_info(kernel_log_obligatorio, "PID: %d - Bloqueado por: %s", pid_process, motivo_block);
}

static void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	if(!list_remove_element(lista_execute, un_pcb)){
		log_error(kernel_logger ,"PCB_%d No esta en EXECUTE", un_pcb->pid);
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "RR") == 0){
		log_info(kernel_log_obligatorio, "PID: %d - Desalojado por fin de Quantum", un_pcb->pid);
	}

	// Aca hay un tema, si llamo al planificador antes de cambiar de ESTADO al proceso que estaba en EXEC, primero
	// imprime el nuevo proceso en EXEC y despues el DESALOJADO(seria este), habria que ver cual es la manera correcta.
	// Despues, si la lista de ready esta vacia y llamo al planificador, el programa va a salir por el EXIT_FAILURE,
	// habria que ver si eso es lo correcto, si ese fuese el caso, el if comentado de abajo se borra, al igual que en _desalojar_proceso
	pcp_planificar_corto_plazo();

	transferir_from_actual_to_siguiente(un_pcb, lista_ready, mutex_lista_ready, READY);

	pcp_planificar_corto_plazo();

//	pthread_mutex_lock(&mutex_lista_exec);
//	if(!list_is_empty(lista_execute)){
//		pcp_planificar_corto_plazo();
//	}
//	pthread_mutex_unlock(&mutex_lista_exec);
//	plp_planificar_proceso_nuevo(NULL);
//	sem_post(&sem_nuevo_en_ready);
//	sem_post(&sem_check_desalojo);
}

static void _desalojar_proceso(t_pcb* un_pcb){

	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que esa PCB siga en la lista
	if(list_remove_element(lista_execute, un_pcb)){

		transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

		liberar_recursos_pcb(un_pcb);
		avisar_a_memoria_para_liberar_estructuras(un_pcb);

	}else{
		log_error(kernel_logger, "PCB no encontrada en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	//-----------------------
	pthread_mutex_lock(&mutex_core);
	procesos_en_core--;
	pthread_mutex_unlock(&mutex_core);
	//-----------------------

	log_info(kernel_logger, "plp_exit [PID: %d]", un_pcb->pid);

	log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: SUCCESS", un_pcb->pid);

	// Quizas no sea necesario, en caso de que no tenga que terminar el programa porque la lista de ready este vacia, se utiliza
	// porque cuando se busque otro proceso a mandar, iria por el EXIT_FAILURE, y romperia.
	pthread_mutex_lock(&mutex_lista_exec);
	if(!list_is_empty(lista_execute)){
		pcp_planificar_corto_plazo();
	}
	pthread_mutex_unlock(&mutex_lista_exec);
//	plp_planificar_proceso_nuevo(NULL);
//	pcp_planificar_corto_plazo();
}

static void _atender_motivo_desalojo(char* motivo_desalojo, t_buffer* un_buffer, t_pcb* un_pcb){
	if(strcmp(motivo_desalojo, "DESALOJO_POR_CONSOLA") == 0){
		_desalojar_proceso(un_pcb);

		pthread_mutex_lock(&mutex_flag_exit);
		flag_exit = false;
		pthread_mutex_unlock(&mutex_flag_exit);

		printf("---------------<%d>\n", un_buffer->size);

//		sem_post(&sem_pcb_execute);  // --> Le decimos al pcp que cpu esta free

	}else if(strcmp(motivo_desalojo, "ALGORITMO_QUANTUM") == 0 || strcmp(motivo_desalojo, "ALGORITMO_PRIORIDADES") == 0){
		pthread_mutex_lock(&mutex_flag_exit);
		if(flag_exit){
			//Esto sirve para darle prioridad al desalojo por consola
			_desalojar_proceso(un_pcb);
			flag_exit = false;
		}else{
			_reubicar_pcb_de_execute_a_ready(un_pcb);
		}
		pthread_mutex_unlock(&mutex_flag_exit);

		printf("---------------<%d>\n", un_buffer->size);

	}else if(strcmp(motivo_desalojo, "") == 0){
		/*Para el resto de los motivos, verificar los pcbs no tenga
		motivo de desalojo, porque puede pasar que el proceso se desaloje solo por alguna necesidad propia*/
		/*Propuesta, siempre verificar si la batiseñal de desalojo esta activada*/
		pthread_mutex_lock(&mutex_flag_exit);
		if(flag_exit){
			_desalojar_proceso(un_pcb);
			flag_exit = false;
		}else{
			//Si no hay batiseñal de desalojo, entonces procesder a atender la PCB segun su motivo normal
			//Hacer esta compŕobacion para todos los demas
		}
		pthread_mutex_unlock(&mutex_flag_exit);
	}
}

 static void _recibir_proceso_desalojado(t_buffer* un_buffer){
	pausador();

	int recibe_pid = recibir_int_del_buffer(un_buffer);
	t_pcb* un_pcb = buscar_pcb_por_pid(recibe_pid);
	if(un_pcb == NULL){
		log_error(kernel_logger, "PID recibido no coincide con PCB de ninguna lista");
		exit(EXIT_FAILURE);
	}

	//Controlando que la PCB encontrada corresponde a la lista execute
	if(!esta_pcb_en_una_lista_especifica(lista_execute, un_pcb)){
		log_error(kernel_logger, "PCB_%d (%d) - No se encuentró en la lista EXECUTE ", un_pcb->pid, un_pcb->estado);
		exit(EXIT_FAILURE);
	}

	un_pcb->program_counter = recibir_int_del_buffer(un_buffer);
	uint32_t* re_RX;
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	un_pcb->registros_CPU->AX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	un_pcb->registros_CPU->BX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	un_pcb->registros_CPU->CX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	un_pcb->registros_CPU->DX = *re_RX; free(re_RX);

	char* motivo_desalojo = recibir_string_del_buffer(un_buffer);
	log_warning(kernel_logger, "<PID:%d>[PC:%d][%u|%u|%u|%u][%s]",
						un_pcb->pid,
						un_pcb->program_counter,
						un_pcb->registros_CPU->AX,
						un_pcb->registros_CPU->BX,
						un_pcb->registros_CPU->CX,
						un_pcb->registros_CPU->DX,
								motivo_desalojo);

	_atender_motivo_desalojo(motivo_desalojo, un_buffer, un_pcb);
}

static void _gestionar_peticiones_de_memoria(){
	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de MEMORIA");

		switch (cod_op) {
		case INICIAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case ESTRUCTURA_INICIADA_KM_OK:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			estructuras_inciadas_en_memoria_rpta(unBuffer);
			sem_post(&sem_estructura_iniciada);
			free(unBuffer);
			break;
		case LIBERAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			// Cuando llega la estructura liberada, hago un sem_post(&sem_estructura_liberada);
			break;
		case PRUEBAS:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//atender_esta_prueba(myBuffer);
			break;
		case -1:
			log_error(kernel_logger, "[DESCONEXION]: MEMORIA");
			//control_key = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(kernel_logger, "Operacion desconocida");
			free(unBuffer);
			break;
		}
	}
}

static void _gestionar_peticiones_de_cpu_dispatch(){
	while(1){
		int cod_op = recibir_operacion(fd_cpu_dispatcher);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de CPU_Dispatch");

		switch (cod_op) {
		case ATENDER_INSTRUCCION_CPK:
			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
			t_pcb* pcb = recibir_contexto_de_ejecucion(unBuffer);
			char* instruccion = recibir_string_del_buffer(unBuffer);
			if(strcmp(instruccion, "SIGNAL") == 0){
				int seconds_blocked = recibir_int_del_buffer(unBuffer);
				atender_sleep(pcb, seconds_blocked, instruccion);

			}else if(strcmp(instruccion, "WAIT") == 0){
				char* recurso_solicitado = recibir_string_del_buffer(unBuffer);
				atender_wait(pcb,recurso_solicitado);

			}else if(strcmp(instruccion, "SIGNAL") == 0){
				char* recurso_a_liberar = recibir_string_del_buffer(unBuffer);
				atender_signal(pcb, recurso_a_liberar);

			}else if(strcmp(instruccion, "F_OPEN") == 0){


			}else if(strcmp(instruccion, "F_CLOSE") == 0){


			}else if(strcmp(instruccion, "F_SEEK") == 0){


			}else if(strcmp(instruccion, "F_READ") == 0){


			}else if(strcmp(instruccion, "F_WRITE") == 0){


			}else if(strcmp(instruccion, "F_TRUNCATE") == 0){


			}else if(strcmp(instruccion, "EXIT") == 0){

			}
			break;
		case DESALOJO_PROCESO_CPK:
			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
			_recibir_proceso_desalojado(unBuffer);
			break;
		case -1:
			log_error(kernel_logger, "[DESCONEXION]: FILESYSTEM");
			//control_key = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(kernel_logger, "Operacion desconocida");
			free(unBuffer);
			break;
		}
	}
}

static void _gestionar_interrupt(){
	while(1){
		sem_wait(&sem_enviar_interrupcion);

		log_warning(kernel_logger, "Pase el semaforo");

		pthread_mutex_lock(&mutex_lista_exec);
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->ticket);

		pthread_mutex_lock(&mutex_flag_finalizar_proceso);
		if(flag_finalizar_proceso == 1){
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_logger, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", pcb_execute->pid, pcb_execute->ticket);
			log_warning(kernel_logger, "ENVIE EL PAQUETE POR FINALIZACION_PROCESO");
			flag_finalizar_proceso = 0;
		}else{
			if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "PRIORIDADES") == 0){
				log_warning(kernel_logger, "ENTRE EN PRIORIDADES");

				cargar_string_al_super_paquete(un_paquete, "ALGORITMO_PRIORIDADES");
				enviar_paquete(un_paquete, fd_cpu_interrupt);
				eliminar_paquete(un_paquete);
				log_warning(kernel_logger, "ENVIE EL PAQUETE POR PRIORIDADES");

			}else if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "RR") == 0){

				cargar_string_al_super_paquete(un_paquete, "ALGORITMO_QUANTUM");
				enviar_paquete(un_paquete, fd_cpu_interrupt);
				eliminar_paquete(un_paquete);
				log_warning(kernel_logger, "ENVIE EL PAQUETE POR RR");

			}

		}
		pthread_mutex_unlock(&mutex_flag_finalizar_proceso);

	}
}

static void _gestionar_peticiones_de_filesystem(){
	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_filesystem);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de FILESYSTEM");

		switch (cod_op) {
		case SYSCALL_KF:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case PRUEBAS:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case -1:
			log_error(kernel_logger, "[DESCONEXION]: FILESYSTEM");
			//control_key = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(kernel_logger, "Operacion desconocida");
			free(unBuffer);
			break;
		}
	}
}

void iniciar_conexiones(){

	fd_cpu_dispatcher = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

}

void atender_memoria(){
	gestionar_handshake_como_cliente(fd_memoria, "MEMORIA", kernel_logger);
	identificarme_con_memoria(fd_memoria, KERNEL);
	log_info(kernel_logger, "HANDSHAKE CON MEMORIA [EXITOSO]");
	pthread_create(&hilo_memoria, NULL, (void*)_gestionar_peticiones_de_memoria, NULL);
	pthread_detach(hilo_memoria);
}

void atender_filesystem(){
	gestionar_handshake_como_cliente(fd_filesystem, "FILESYSTEM", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON FILESYSTEM [EXITOSO]");
	pthread_create(&hilo_filesystem, NULL, (void*)_gestionar_peticiones_de_filesystem, NULL);
	pthread_detach(hilo_filesystem);
}

void atender_cpu_dispatch(){
	gestionar_handshake_como_cliente(fd_cpu_dispatcher, "CPU_Dispatch", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON CPU_Dispatch [EXITOSO]");
	pthread_create(&hilo_cpu_dispatch, NULL, (void*)_gestionar_peticiones_de_cpu_dispatch, NULL);
	pthread_detach(hilo_cpu_dispatch);
}

void atender_cpu_interrupt(){
	gestionar_handshake_como_cliente(fd_cpu_interrupt, "CPU_Interrupt", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON CPU_Interrupt [EXITOSO]");
	pthread_create(&hilo_cpu_interrupt, NULL, (void*)_gestionar_interrupt, NULL);
	pthread_detach(hilo_cpu_interrupt);
}

// ----- Funciones de manejo de INSTRUCCIONES del CPU ------

void atender_sleep(t_pcb* pcb, int seconds_blocked, char* motivo_block){
	if(list_remove_element(lista_execute, pcb)){
		log_warning(kernel_logger,"ATIENDO SLEEP");

		t_sleep* pcb_sleep = malloc(sizeof(t_sleep));
		pcb_sleep->pcb_a_sleep = pcb;
		pcb_sleep->tiempo_en_block = seconds_blocked;
		transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
		log_blocked_proceso(pcb->pid, motivo_block);
		ejecutar_en_un_hilo_nuevo_detach((void*) manejar_tiempo_sleep, pcb_sleep);
	}else
		log_error(kernel_logger,"No se puedo remeover el proceso de EXECUTE al ejecutar la intrusccion SLEEP");
}

void manejar_tiempo_sleep(t_sleep* pcb_sleep){
	log_warning(kernel_logger,"BLOCKEO DURANTE: %d", pcb_sleep->tiempo_en_block);

	sleep(pcb_sleep->tiempo_en_block);
	transferir_from_actual_to_siguiente(pcb_sleep->pcb_a_sleep, lista_ready, mutex_lista_ready, READY);
	free(pcb_sleep);
}

void atender_wait(t_pcb* pcb,char* recurso_solicitado){
	t_recurso* recurso_buscado = buscar_recurso(recurso_solicitado);

	// Verifico que exista el recurso, en caso de no hacerlo el proceso se envia a EXIT
	if(recurso_buscado != NULL){
		if(recurso_buscado->instancias >= 0){
			recurso_buscado->instancias --;
			list_add(pcb->lista_recursos_pcb, recurso_buscado);
			list_add(recurso_buscado->lista_asignados, pcb);
		}else{
			pthread_mutex_lock(&mutex_lista_exec);
			list_remove(lista_execute,0);
			pthread_mutex_unlock(&mutex_lista_exec);

			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED); // --> Esto es necesario en caso de el FINALIZAR_PROCESO si o si necesite la lsita_blocked
																								   // 	    de lo contrario se busca el pcb directamente en lista_bloqueados	por cada recurso
			agregar_pcb_a_lista_recurso(pcb ,recurso_buscado->lista_bloqueados, recurso_buscado->mutex_bloqueados);
			log_blocked_proceso(pcb->pid, recurso_solicitado);
		}
	}else{
		log_warning(kernel_logger,"El recurso no existe, se procede a finalizar el PID: %d", pcb->pid);
		plp_planificar_proceso_exit(pcb);
	}
}

void atender_signal(t_pcb* pcb,char* recurso_a_liberar){
	t_recurso* recurso_buscado = buscar_recurso(recurso_a_liberar);
	if(recurso_buscado != NULL && list_remove_element(recurso_buscado->lista_asignados, pcb) && list_remove_element(pcb->lista_recursos_pcb, recurso_buscado)){
		recurso_buscado->instancias ++;
		if(!list_is_empty(recurso_buscado->lista_bloqueados)){
			asignar_recurso_liberado_pcb(recurso_buscado);
		}
	}else{
		log_warning(kernel_logger,"El recurso no existe, se procede a finalizar el PID: %d", pcb->pid);
		plp_planificar_proceso_exit(pcb);
	}
}

t_recurso* buscar_recurso(char* recurso_solicitado){
	t_recurso* recurso_encontrado = NULL;
// Capaz es necesario utilziar mutex de la lista de recursos, veremos cuando se haga DEADLOCKS
	for(int i = 0; i<list_size(lista_recursos); i++){
		t_recurso* r_buscado = list_get(lista_recursos,i);
		if(recurso_solicitado == r_buscado->recurso_name){
			recurso_encontrado = r_buscado;
			break;
		}
	}

	return recurso_encontrado;
}

void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recurso, pthread_mutex_t mutex_recurso){
//	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb -> pid);
	pthread_mutex_lock(&mutex_recurso);
	list_add(lista_recurso, pcb);
	pthread_mutex_unlock(&mutex_recurso);
}




