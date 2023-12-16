#include "../include/atender_conexiones.h"

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

void _gestionar_peticiones_de_memoria(){
	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de MEMORIA");

		switch (cod_op) {
		case ESTRUCTURA_INICIADA_MK:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			recibir_confirmacion_de_memoria(unBuffer);
			sem_post(&sem_estructura_iniciada);
			free(unBuffer);
			break;
		case ESTRUCTURA_LIBERADA_MK:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			recibir_confirmacion_de_memoria(unBuffer);
			sem_post(&sem_estructura_liberada);
			free(unBuffer);
			break;
		case RESPUESTA_PAGE_FAULT_MK:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			int* pid_process = malloc(sizeof(int));
			*pid_process = recibir_int_del_buffer(unBuffer);
			ejecutar_en_un_hilo_nuevo_detach((void*)desbloquear_proceso_por_pid, pid_process);

			log_info(kernel_logger, "Memoria soluciono el PAGE FAULT");
			// Este UNLOCK es del LOCK que esta en el hilo de atender_page_fault, para que se atienda de a 1.
			pthread_mutex_unlock(&mutex_manejo_page_fault);

			free(unBuffer);
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

void _gestionar_peticiones_de_cpu_dispatch(){
	while(1){
		int cod_op = recibir_operacion(fd_cpu_dispatcher);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de CPU_Dispatch");
		unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
		t_pcb* pcb;

		switch (cod_op) {
		case ATENDER_INSTRUCCION_CPK:

			pcb = _recibir_proceso_desalojado(unBuffer);
			t_buffer* mochila = recibir_mochila_del_buffer(unBuffer);
			char* instruccion = recibir_string_del_buffer(mochila);
			pausador();

			pthread_mutex_lock(&mutex_flag_proceso_desalojado);
			flag_proceso_desalojado = true;
			pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

			pthread_mutex_lock(&mutex_flag_exit);
			flag_exit = true;
			pthread_mutex_unlock(&mutex_flag_exit);

			//Esto sirve para darle prioridad al desalojo por consola
			pthread_mutex_lock(&mutex_flag_finalizar_proceso);
			if(flag_finalizar_proceso){

				ejecutar_en_un_hilo_nuevo_detach((void*)_desalojar_proceso, pcb);
	//			_desalojar_proceso(un_pcb);

			}else if(strcmp(instruccion, "SLEEP") == 0){
				int seconds_blocked = recibir_int_del_buffer(mochila);
				atender_sleep(pcb, seconds_blocked, instruccion);

			}else if(strcmp(instruccion, "WAIT") == 0){
				char* recurso_solicitado = recibir_string_del_buffer(mochila);
				log_info(kernel_logger, " If del WAIT %s", recurso_solicitado);
				atender_wait(pcb,recurso_solicitado);
				free(recurso_solicitado);

			}else if(strcmp(instruccion, "SIGNAL") == 0){
				log_info(kernel_logger, "Entre a SIGNAL: %s", instruccion);
				char* recurso_a_liberar = recibir_string_del_buffer(mochila);
				atender_signal(pcb, recurso_a_liberar);
				free(recurso_a_liberar);

			}else if(strcmp(instruccion, "F_OPEN") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				log_info(kernel_log_obligatorio, "PID: %d - Abrir Archivo: %s", pcb->pid, nombre_archivo);
				char* tipo_apertura = recibir_string_del_buffer(mochila); // Puede ser 'r' para lectura, o 'w' para escritura
				atender_F_open(pcb, nombre_archivo, tipo_apertura);

				free(nombre_archivo);
				free(tipo_apertura);
			}else if(strcmp(instruccion, "F_CLOSE") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				log_info(kernel_log_obligatorio, "PID: %d - Cerrar Archivo: %s", pcb->pid, nombre_archivo);
				atender_F_close(nombre_archivo , pcb);

				free(nombre_archivo);
			}else if(strcmp(instruccion, "F_SEEK") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				int nuevo_puntero_archivo = recibir_int_del_buffer(mochila);
				atender_F_seek(nombre_archivo , nuevo_puntero_archivo, pcb);

				free(nombre_archivo);
			}else if(strcmp(instruccion, "F_TRUNCATE") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				int nuevo_size_archivo = recibir_int_del_buffer(mochila);
				atender_F_truncate(nombre_archivo , nuevo_size_archivo, pcb);
				log_info(kernel_log_obligatorio,"PID: %d - Archivo: %s - Tamaño: %d", pcb->pid, nombre_archivo, nuevo_size_archivo);

				free(nombre_archivo);
			}else if(strcmp(instruccion, "F_READ") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				int dir_fisica = recibir_int_del_buffer(mochila);
				atender_F_read(nombre_archivo , dir_fisica, pcb);

				free(nombre_archivo);
			}else if(strcmp(instruccion, "F_WRITE") == 0){
				char* nombre_archivo = recibir_string_del_buffer(mochila);
				int dir_fisica = recibir_int_del_buffer(mochila);
				atender_F_write(nombre_archivo , dir_fisica, pcb);

				free(nombre_archivo);
			}else if(strcmp(instruccion, "EXIT") == 0){

				pcb->motivo_exit = SUCCESS;
				plp_exit(pcb);

			}else if(strcmp(instruccion, "PAGE_FAULT") == 0){
				int pagina = recibir_int_del_buffer(mochila);
				pthread_mutex_lock(&mutex_lista_exec);
				if(list_remove_element(lista_execute,pcb)){
					t_page_fault* un_page_fault = malloc(sizeof(t_page_fault));
					un_page_fault->pid_process = pcb->pid;
					un_page_fault->numero_pagina = pagina;

					log_info(kernel_log_obligatorio, "Page Fault PID: %d - Pagina: %d", pcb->pid, pagina);
					transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);

					ejecutar_en_un_hilo_nuevo_detach((void*)_atender_page_fault, un_page_fault);
				}
				pthread_mutex_unlock(&mutex_lista_exec);
			}

			pthread_mutex_unlock(&mutex_flag_finalizar_proceso);

			free(instruccion);
			free(mochila);
			break;
		case DESALOJO_PROCESO_CPK:

			pcb = _recibir_proceso_desalojado(unBuffer);

			pthread_mutex_lock(&mutex_flag_proceso_desalojado);
			flag_proceso_desalojado = true;
			pthread_mutex_unlock(&mutex_flag_proceso_desalojado);
			pthread_mutex_lock(&mutex_flag_exit);
			flag_exit = true;
			pthread_mutex_unlock(&mutex_flag_exit);

			char* motivo_desalojo = recibir_string_del_buffer(unBuffer);
			log_warning(kernel_logger, "<PID:%d>[PC:%d][%u|%u|%u|%u][%s]",
										pcb->pid,
										pcb->program_counter,
										pcb->registros_CPU->AX,
										pcb->registros_CPU->BX,
										pcb->registros_CPU->CX,
										pcb->registros_CPU->DX,
										motivo_desalojo);
			_atender_motivo_desalojo(motivo_desalojo, unBuffer, pcb);
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

void _gestionar_interrupt(){
	while(1){
		sem_wait(&sem_enviar_interrupcion);

		pthread_mutex_lock(&mutex_lista_exec);
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_KC);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->ticket);

		pthread_mutex_lock(&mutex_flag_finalizar_proceso);
		if(flag_finalizar_proceso){
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_logger, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", pcb_execute->pid, pcb_execute->ticket);
//			flag_finalizar_proceso = false;

		}else{
			if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "PRIORIDADES") == 0){

				cargar_string_al_super_paquete(un_paquete, "ALGORITMO_PRIORIDADES");
				enviar_paquete(un_paquete, fd_cpu_interrupt);
				eliminar_paquete(un_paquete);
				log_info(kernel_logger, "Envio interrupcion por Prioridades");

			}else if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "RR") == 0){

				cargar_string_al_super_paquete(un_paquete, "ALGORITMO_QUANTUM");
				enviar_paquete(un_paquete, fd_cpu_interrupt);
				eliminar_paquete(un_paquete);
				log_info(kernel_logger, "Envio interrupcion por RR");
			}
		}
		pthread_mutex_unlock(&mutex_flag_finalizar_proceso);
	}
}

void _gestionar_peticiones_de_filesystem(){
	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_filesystem);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de FILESYSTEM");
		unBuffer = recibiendo_super_paquete(fd_filesystem);

		switch (cod_op) {
		case RESPUESTA_F_OPEN_FK:
			char* operacion = recibir_string_del_buffer(unBuffer);
			int confirmacion = recibir_int_del_buffer(unBuffer);

			log_info(kernel_logger, "LA oepracion es: %d -- %s", confirmacion, operacion);

//			Recibo la operacion, para actualizar el valor del flag y contiruan en atender_F_open
			validar_respuesta_F_open(operacion, confirmacion, unBuffer);
			sem_post(&sem_f_open_FS);

			free(operacion);
			break;
		case RESPUESTA_F_CLOSE_FK:
			//
			break;
		case RESPUESTA_F_SEEK_FK:
			//
			break;
		case RESPUESTA_F_TRUNCATE_FK:
			char* respuesta_truncate = recibir_string_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %s" , respuesta_truncate);

			int* pid_process_truncate = malloc(sizeof(int));
			*pid_process_truncate = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process_truncate);

			break;
		case RESPUESTA_F_READ_FK:
			char* respuesta_read = recibir_string_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %s" , respuesta_read);

			int* pid_process_read = malloc(sizeof(int));
			*pid_process_read = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process_read);
			free(respuesta_read);
			break;
		case RESPUESTA_F_WRITE_FK:
			char* respuesta_write = recibir_string_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %s" , respuesta_write);

			int* pid_process_write = malloc(sizeof(int));
			*pid_process_write = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process_write);
			free(respuesta_write);
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
		free(unBuffer);
	}
}

// ----- PAGE FAULT -----
void _atender_page_fault(t_page_fault* un_page_fault){
	pthread_mutex_lock(&mutex_manejo_page_fault);

	// Envio PCB: [PID][NUM_PAG]
	t_paquete* un_paquete = crear_super_paquete(PETICION_PAGE_FAULT_KM);
	cargar_int_al_super_paquete(un_paquete, un_page_fault->pid_process);
	cargar_int_al_super_paquete(un_paquete, un_page_fault->numero_pagina);
	enviar_paquete(un_paquete, fd_memoria);

	log_info(kernel_logger,"Se solicito a Memoria resolver Page FAULT");
	free(un_page_fault);
	eliminar_paquete(un_paquete);
}

// ----- PARA RECIBIR INSTRUCIONES y PAGE FAULT DE CPU -----
t_buffer* recibir_mochila_del_buffer(t_buffer* buffer){
	t_buffer* mochila = malloc(sizeof(t_buffer));
	mochila->size = buffer->size;
	mochila->stream = recibir_choclo_del_buffer(buffer);
	return mochila;
}

// ----- RECIBIR MENSAJES DE MEMORIA -----
void recibir_confirmacion_de_memoria(t_buffer* unBuffer){
	char* recibir_mensaje = (char*)recibir_choclo_del_buffer(unBuffer);
	printf("[%d]> %s\n", (int)strlen(recibir_mensaje), recibir_mensaje);
	free(recibir_mensaje);
}

// ----- FUNCIONES PARA LA GESTION DE DESALOJO DE PROCESO -----
t_pcb* _recibir_proceso_desalojado(t_buffer* un_buffer){

	int recibe_pid = recibir_int_del_buffer(un_buffer);

	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb* un_pcb = buscar_pcb_por_pid_en(recibe_pid, lista_execute, mutex_lista_exec);
	pthread_mutex_unlock(&mutex_lista_exec);
	if(un_pcb == NULL){
		log_error(kernel_logger, "PID recibido no coincide con PCB de ninguna lista");
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

	return un_pcb;
}

void _atender_motivo_desalojo(char* motivo_desalojo, t_buffer* un_buffer, t_pcb* un_pcb){
	pausador();

	if(strcmp(motivo_desalojo, "DESALOJO_POR_CONSOLA") == 0){

//		pthread_mutex_lock(&mutex_flag_exit);
//		flag_exit = true;
//		pthread_mutex_unlock(&mutex_flag_exit);

		_desalojar_proceso(un_pcb);

		printf("---------------<%d>\n", un_buffer->size);

	}else if(strcmp(motivo_desalojo, "ALGORITMO_QUANTUM") == 0 || strcmp(motivo_desalojo, "ALGORITMO_PRIORIDADES") == 0){

		pthread_mutex_lock(&mutex_flag_finalizar_proceso);
		if(flag_finalizar_proceso){
			//Esto sirve para darle prioridad al desalojo por consola
//			pthread_mutex_lock(&mutex_flag_exit);
//			flag_exit = true;
//			pthread_mutex_unlock(&mutex_flag_exit);

			ejecutar_en_un_hilo_nuevo_detach((void*)_desalojar_proceso, un_pcb);
//			_desalojar_proceso(un_pcb);
			log_info(kernel_logger,"FALG_EXIT %d", flag_finalizar_proceso);
		}else{
			//Cambiamos el estado de desalojado para futuros FINALIZAR_PROCESO, ya que aca es atendido.

			_reubicar_pcb_de_execute_a_ready(un_pcb);
		}
		pthread_mutex_unlock(&mutex_flag_finalizar_proceso);

		printf("---------------<%d>\n", un_buffer->size);
	}
}

void _desalojar_proceso(t_pcb* un_pcb){

	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que esa PCB siga en la lista
	if(list_remove_element(lista_execute, un_pcb)){
		liberar_recursos_pcb(un_pcb);
		//liberar_archivo_pcb(un_pcb);
		avisar_a_memoria_para_liberar_estructuras(un_pcb);
		sem_wait(&sem_estructura_liberada);
		transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: SUCCESS", un_pcb->pid);
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

	pthread_mutex_lock(&mutex_flag_proceso_desalojado);
	flag_proceso_desalojado = false;
	pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

	pthread_mutex_lock(&mutex_flag_finalizar_proceso);
	flag_finalizar_proceso = false;
	pthread_mutex_unlock(&mutex_flag_finalizar_proceso);

	plp_planificar_proceso_nuevo(NULL);
}

void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	pthread_mutex_lock(&mutex_lista_ready);

	if(!list_remove_element(lista_execute, un_pcb)){
		log_error(kernel_logger ,"PCB_%d No esta en EXECUTE", un_pcb->pid);
		exit(EXIT_FAILURE);
	}

	cambiar_estado(un_pcb, READY);
	list_add(lista_ready, un_pcb);

	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", un_pcb -> pid, estado_to_string(EXEC), estado_to_string(un_pcb->estado));

	char* pids_en_ready = lista_pids_en_ready(lista_ready, mutex_lista_ready);
	log_info(kernel_log_obligatorio, "Cola Ready %s: %s", algoritmo_to_string(ALGORITMO_PLANIFICACION), pids_en_ready);
	free(pids_en_ready);

	if(strcmp(algoritmo_to_string(ALGORITMO_PLANIFICACION), "RR") == 0){
		log_info(kernel_log_obligatorio, "PID: %d - Desalojado por fin de Quantum", un_pcb->pid);
	}

	pthread_mutex_unlock(&mutex_lista_ready);
	pthread_mutex_unlock(&mutex_lista_exec);

	pthread_mutex_lock(&mutex_flag_proceso_desalojado);
	flag_proceso_desalojado = false;
	pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

	pcp_planificar_corto_plazo();
}
