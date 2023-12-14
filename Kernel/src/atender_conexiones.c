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
			int pid_process = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process);;

			log_info(kernel_logger, "Memoria soluciono el PAGE FAULT");
			// Este UNLOCK es del LOCK que esta en el hilo de atender_page_fault, para que se atienda de a 1.
			pthread_mutex_unlock(&mutex_manejo_page_fault);

			ejecutar_en_un_hilo_nuevo_detach((void*)pcp_planificar_corto_plazo, NULL);
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
			log_warning(kernel_logger, "Entre a atender instruccion");

			pcb = _recibir_proceso_desalojado(unBuffer);
			t_buffer* mochila = recibir_mochila_del_buffer(unBuffer);
			char* instruccion = recibir_string_del_buffer(mochila);
			pausador();

			pthread_mutex_lock(&mutex_flag_proceso_desalojado);
			flag_proceso_desalojado = true;
			pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

			//Esto sirve para darle prioridad al desalojo por consola
			pthread_mutex_lock(&mutex_flag_finalizar_proceso);
			if(flag_finalizar_proceso){
				pthread_mutex_lock(&mutex_flag_exit);
				flag_exit = true;
				pthread_mutex_unlock(&mutex_flag_exit);

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
				pthread_mutex_lock(&mutex_flag_exit);
				flag_exit = true;
				pthread_mutex_unlock(&mutex_flag_exit);

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
			}else{
				log_info(kernel_logger, "Entre al if vacio");
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
// ----- PARA RECIBIR INSTRUCIONES y PAGE FAULT DE CPU -----
t_buffer* recibir_mochila_del_buffer(t_buffer* buffer){
	t_buffer* mochila = malloc(sizeof(t_buffer));
	mochila->size = buffer->size;
	mochila->stream = recibir_choclo_del_buffer(buffer);
	return mochila;
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

		int pid_process;
		switch (cod_op) {
		case RESPUESTA_F_OPEN_FK:
			log_warning(kernel_logger, "Entre a OPEN");

			pthread_mutex_unlock(&mutex_peticion_fs);
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
			log_warning(kernel_logger, "Entre a TRUNCATE");
			// Hay que ver si recibo el archivo entero, o que, y ahi actualizo el tamaño
			pthread_mutex_unlock(&mutex_peticion_fs);

			int respuesta_truncate = recibir_int_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %d" , respuesta_truncate);

			pid_process = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process);

			break;
		case RESPUESTA_F_READ_FK:
			log_warning(kernel_logger, "Entre a READ");

			pthread_mutex_unlock(&mutex_peticion_fs);

			int respuesta_read = recibir_int_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %d" , respuesta_read);

			pid_process = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process);

			break;
		case RESPUESTA_F_WRITE_FK:
			log_warning(kernel_logger, "Entre a WRITE");

			pthread_mutex_unlock(&mutex_peticion_fs);

			int respuesta_write = recibir_int_del_buffer(unBuffer);
			log_info(kernel_logger, " La respuesta es: %d" , respuesta_write);

			pid_process = recibir_int_del_buffer(unBuffer);
			desbloquear_proceso_por_pid(pid_process);

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

		pthread_mutex_lock(&mutex_flag_exit);
		flag_exit = true;
		pthread_mutex_unlock(&mutex_flag_exit);

		_desalojar_proceso(un_pcb);

		printf("---------------<%d>\n", un_buffer->size);

	}else if(strcmp(motivo_desalojo, "ALGORITMO_QUANTUM") == 0 || strcmp(motivo_desalojo, "ALGORITMO_PRIORIDADES") == 0){

		pthread_mutex_lock(&mutex_flag_finalizar_proceso);
		if(flag_finalizar_proceso){
			//Esto sirve para darle prioridad al desalojo por consola
			pthread_mutex_lock(&mutex_flag_exit);
			flag_exit = true;
			pthread_mutex_unlock(&mutex_flag_exit);

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

	int flag_new_vacia = 0;
	pthread_mutex_lock(&mutex_lista_new);
	if(list_is_empty(lista_new))
		flag_new_vacia = 1;
	pthread_mutex_unlock(&mutex_lista_new);

	if(flag_new_vacia == 1){
		pcp_planificar_corto_plazo();
	}else
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

// ----- RECIBIR MENSAJES DE MEMORIA -----
void recibir_confirmacion_de_memoria(t_buffer* unBuffer){
	char* recibir_mensaje = (char*)recibir_choclo_del_buffer(unBuffer);
	printf("[%d]> %s\n", (int)strlen(recibir_mensaje), recibir_mensaje);
	free(recibir_mensaje);
}

// ----- LOG PROCESOS BLOQUEADOS -----
void log_blocked_proceso(int pid_process, char* motivo_block){
	log_info(kernel_log_obligatorio, "PID: %d - Bloqueado por: %s", pid_process, motivo_block);
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

// ----- Funciones de manejo de INSTRUCCIONES del CPU ------
// ----- SLEEP -----
void atender_sleep(t_pcb* pcb, int seconds_blocked, char* motivo_block){
	if(list_remove_element(lista_execute, pcb)){
		log_warning(kernel_logger,"ATIENDO SLEEP");

		t_sleep* pcb_sleep = malloc(sizeof(t_sleep));
		pcb_sleep->pcb_a_sleep = pcb;
		pcb_sleep->tiempo_en_block = seconds_blocked;
		pcb->motivo_block = OTRO;
		transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
		log_blocked_proceso(pcb->pid, motivo_block);
		ejecutar_en_un_hilo_nuevo_detach((void*) manejar_tiempo_sleep, pcb_sleep);
		pcp_planificar_corto_plazo();
	}else
		log_error(kernel_logger,"No se puedo remeover el proceso de EXECUTE al ejecutar la intrusccion SLEEP");
}

void manejar_tiempo_sleep(t_sleep* pcb_sleep){
	log_warning(kernel_logger,"BLOCKEO DURANTE: %d", pcb_sleep->tiempo_en_block);

	sleep(pcb_sleep->tiempo_en_block);
	pthread_mutex_lock(&mutex_lista_blocked);
	list_remove_element(lista_blocked, pcb_sleep->pcb_a_sleep);
	pthread_mutex_unlock(&mutex_lista_blocked);
	transferir_from_actual_to_siguiente(pcb_sleep->pcb_a_sleep, lista_ready, mutex_lista_ready, READY);
	pcp_planificar_corto_plazo();
	free(pcb_sleep);
}

// ----- WAIT  -----
void atender_wait(t_pcb* pcb,char* recurso_solicitado){
	log_info(kernel_logger, "Voy a buscar recurso: %s", recurso_solicitado);
	t_recurso* recurso_buscado = buscar_recurso(recurso_solicitado);
	// Verifico que exista el recurso, en caso de no hacerlo el proceso se envia a EXIT
	if(recurso_buscado != NULL){
		if(recurso_buscado->instancias > 0){
			recurso_buscado->instancias --;
			list_add(pcb->lista_recursos_pcb, recurso_buscado);
			recurso_buscado->pcb_asignado = pcb;
			log_info(kernel_log_obligatorio, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, recurso_buscado->recurso_name, recurso_buscado->instancias);
			_enviar_respuesta_instruccion_CPU_por_dispatch(1);
		}else{
			_enviar_respuesta_instruccion_CPU_por_dispatch(-1);
			pthread_mutex_lock(&mutex_lista_exec);
			list_remove(lista_execute,0);
			pthread_mutex_unlock(&mutex_lista_exec);

			list_add(pcb->lista_recursos_pcb, recurso_buscado);

			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			pcb->motivo_block = RECURSO;
			agregar_pcb_a_lista_recurso(pcb ,recurso_buscado->lista_bloqueados, recurso_buscado->mutex_bloqueados);
			log_blocked_proceso(pcb->pid, recurso_solicitado);
			sem_post(&sem_nuevo_en_block);

			pcp_planificar_corto_plazo();  // Capaz es mejor usar el hilodetach
		}
	}else{
		_enviar_respuesta_instruccion_CPU_por_dispatch(-1);
		pcb->motivo_exit = INVALID_RESOURCE;
//		log_info(kernel_log_obligatorio,"Finaliza el proceso %d - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));
//		plp_planificar_proceso_exit(pcb->pid);
		plp_exit(pcb);
	}
}

// ----- SIGNAL -----
void atender_signal(t_pcb* pcb,char* recurso_a_liberar){
	t_recurso* recurso_buscado = buscar_recurso(recurso_a_liberar);

	if(recurso_buscado != NULL  && list_remove_element(pcb->lista_recursos_pcb, recurso_buscado)){
		recurso_buscado->pcb_asignado = NULL;
		recurso_buscado->instancias ++;
		log_info(kernel_log_obligatorio, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, recurso_buscado->recurso_name, recurso_buscado->instancias);
		_enviar_respuesta_instruccion_CPU_por_dispatch(1);
		if(!list_is_empty(recurso_buscado->lista_bloqueados)){
			asignar_recurso_liberado_pcb(recurso_buscado);
		}
	}else{
		_enviar_respuesta_instruccion_CPU_por_dispatch(-1);
		pcb->motivo_exit = INVALID_RESOURCE;
//		log_info(kernel_log_obligatorio,"Finaliza el proceso %d - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));
//		plp_planificar_proceso_exit(pcb->pid);
		plp_exit(pcb);
	}
}

t_recurso* buscar_recurso(char* recurso_solicitado){
// Capaz es necesario utilziar mutex de la lista de recursos, veremos cuando se haga DEADLOCKS
	for(int i = 0; i<list_size(lista_recursos); i++){
		t_recurso* r_buscado = list_get(lista_recursos,i);
		if(strcmp(recurso_solicitado, r_buscado->recurso_name) == 0){
			return r_buscado;
		}
	}
	return NULL;
}

void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recurso, pthread_mutex_t mutex_recurso){
//	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb -> pid);
	pthread_mutex_lock(&mutex_recurso);
	list_add(lista_recurso, pcb);
	pthread_mutex_unlock(&mutex_recurso);
}

// ----- F_OPEN -----
void atender_F_open(t_pcb* pcb, char* nombre_archivo, char* tipo_apertura){
	t_archivo* archivo = obtener_archivo_global(nombre_archivo);

	if(archivo == NULL){
		send_atender_F_instruccion_fs(nombre_archivo, "ABRIR_ARCHIVO", 0, 0, MANEJAR_F_OPEN_KF);
		// Espero la respuesta del file system
		sem_wait(&sem_f_open_FS);
		archivo = obtener_archivo_global(nombre_archivo);
		pthread_mutex_lock(&mutex_existe_archivo);
		if(flag_existe_archivo == 0){
			archivo = crear_archivo(nombre_archivo, 0);
			log_info(kernel_logger, "Cree el archivo: %s", archivo->nombre_archivo);
			send_atender_F_instruccion_fs(nombre_archivo, "CREAR_ARCHIVO", 0, 0, MANEJAR_F_OPEN_KF);
			// Espero la respuesta del file system
			sem_wait(&sem_f_open_FS);
		}
		pthread_mutex_unlock(&mutex_existe_archivo);
	}

	asignar_archivo_pcb(pcb, archivo, tipo_apertura);

	if(archivo->lock_escritura->locked == 0){
		if(strcmp(tipo_apertura, "R") == 0){
			if(archivo->lock_lectura->locked == 1){
				list_add(archivo->lock_lectura->lista_participantes, pcb);

			}else{
				archivo->lock_lectura->locked = 1;
				list_add(archivo->lock_lectura->lista_participantes, pcb);
			}
		}else{
			archivo->lock_escritura->locked = 1;
			archivo->lock_escritura->pcb = pcb;
		}
		_enviar_respuesta_instruccion_CPU_por_dispatch(1);
	}else{
		_enviar_respuesta_instruccion_CPU_por_dispatch(-1);
		pthread_mutex_lock(&mutex_lista_exec);
		list_remove(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);
		pcb->motivo_block = RECURSO;
		transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
		log_blocked_proceso(pcb->pid, nombre_archivo);
		log_info(kernel_logger, "El PID: %d - Lo voy a bloquear en la cola fs", pcb->pid);
		bloquear_proceso_cola_fs(pcb, archivo);

		pcp_planificar_corto_plazo();
	}
}

void send_atender_f_open_FS(char* nombre_archivo, char* operacion){
	t_paquete* un_paquete = crear_super_paquete(MANEJAR_F_OPEN_KF);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	cargar_string_al_super_paquete(un_paquete, operacion);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

t_archivo* crear_archivo(char* nombre_archivo, int size_archivo){
	t_archivo* archivo = malloc(sizeof(t_archivo));
	archivo->nombre_archivo = strdup(nombre_archivo);
	archivo->cola_block_procesos = list_create();
	pthread_mutex_init(&archivo->mutex_cola_block, NULL);
	archivo->size = size_archivo;

	archivo->lock_escritura = malloc(sizeof(t_lock_escritura));
	archivo->lock_escritura->locked = 0;
	archivo->lock_escritura->pcb = NULL;

	archivo->lock_lectura = malloc(sizeof(t_lock_lectura));
	archivo->lock_lectura->locked = 0;
	archivo->lock_lectura->lista_participantes = list_create();
	pthread_mutex_init(&archivo->lock_lectura->mutex_lista_asiganada, NULL);

	return archivo;
}

t_archivo* obtener_archivo_global(char* nombre_archivo){
	t_archivo* archivo;
	if(!list_is_empty(lista_archivos_abiertos)){
		for(int i = 0; i < list_size(lista_archivos_abiertos); i++){
			archivo = list_get(lista_archivos_abiertos, i);
			if(strcmp(archivo->nombre_archivo, nombre_archivo) == 0){
					return archivo;
			}
		}
	}
	return NULL;
}

// ----- F_CLOSE -----
void atender_F_close(char* close_nombre_archivo, t_pcb* pcb){
	// Esto es para saber si ya solicito el F_OPEN del archivo
	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, close_nombre_archivo);

	if(archivo_pcb != NULL){
		t_archivo* archivo = obtener_archivo_global(close_nombre_archivo);
		// Aca determinamos si el pcb tiene el lock otorgado
		if(archivo_pcb->lock_otorgado == 1){
			if(strcmp(archivo_pcb->modo_apertura, "R") == 0){
				liberar_lock_lectura(archivo->lock_lectura, pcb);
			}else{
				liberar_lock_escritura(archivo->lock_escritura, pcb);
			}
			if(archivo->lock_lectura->locked == 0 && archivo->lock_escritura->locked == 0){
				pthread_mutex_lock(&archivo->mutex_cola_block);
				if(list_size(archivo->cola_block_procesos) == 0){
					list_remove_element(pcb->archivos_abiertos, archivo_pcb);
				}else{
					asignar_lock_pcb(archivo);
				}
				pthread_mutex_unlock(&archivo->mutex_cola_block);
			}
		}
	}else{
		log_error(kernel_logger,"El archivo no fue abierto previamente");
	}
	_enviar_respuesta_instruccion_CPU_por_dispatch(1);
}

// ----- F_SEEK -----
void atender_F_seek(char* nombre_archivo , int nuevo_puntero_archivo, t_pcb* pcb){
	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, nombre_archivo);

	if(archivo_pcb!= NULL){
		archivo_pcb->puntero = nuevo_puntero_archivo;
		log_info(kernel_log_obligatorio, "PID: %d - Actualizar puntero Archivo: %s - Puntero %d", pcb->pid, nombre_archivo, archivo_pcb->puntero);
	}else
		log_error(kernel_logger,"El archivo no fue abierto previamente");

	_enviar_respuesta_instruccion_CPU_por_dispatch(1);
}

// ----- F_TRUNCATE -----
void atender_F_truncate(char* nombre_archivo , int nuevo_size_archivo, t_pcb* pcb){

	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, nombre_archivo);

	if(archivo_pcb != NULL){

//		t_archivo* archivo = obtener_archivo_global(nombre_archivo);
		pthread_mutex_lock(&mutex_lista_exec);
		list_remove(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
		log_blocked_proceso(pcb->pid, nombre_archivo);

		// Llamo al planificador para que envie un nuevo preceso a CPU.
		pcp_planificar_corto_plazo();

		send_atender_F_instruccion_fs(nombre_archivo,"", nuevo_size_archivo, pcb->pid, MANEJAR_F_TRUNCATE_KF);
	}else
		log_error(kernel_logger,"El archivo no fue abierto previamente");
}

// ----- F_READ -----
void atender_F_read(char* nombre_archivo , int dir_fisica, t_pcb* pcb){
	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, nombre_archivo);
	t_archivo* archivo = obtener_archivo_global(nombre_archivo);

	log_info(kernel_log_obligatorio,"PID: %d - Leer Archivo: %s - Puntero: %d - Dirección Memoria %d - Tamaño %d", pcb->pid, nombre_archivo, archivo_pcb->puntero, dir_fisica, archivo->size);

	pthread_mutex_lock(&mutex_lista_exec);
	list_remove(lista_execute, 0);
	pthread_mutex_unlock(&mutex_lista_exec);

	transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
	log_blocked_proceso(pcb->pid, nombre_archivo);

	// Llamo al planificador para que envie un nuevo preceso a CPU.
	pcp_planificar_corto_plazo();

	// Le envio a File System  la solicitud de READ
	send_atender_F_read_write(nombre_archivo,archivo_pcb->puntero ,dir_fisica,pcb->pid ,MANEJAR_F_WRITE_KF);
}

// ----- F_WRITE -----
void atender_F_write(char* nombre_archivo , int dir_fisica, t_pcb* pcb){
	t_archivo_abierto_pcb* archivo_pcb = obtener_archivo_pcb(pcb, nombre_archivo);
	log_info(kernel_logger,"El lock es: %d y su modo de apertura %s", archivo_pcb->lock_otorgado , archivo_pcb->modo_apertura);

	if(archivo_pcb->lock_otorgado == 1 && strcmp(archivo_pcb->modo_apertura , "W") == 0){
		t_archivo* archivo = obtener_archivo_global(nombre_archivo);
		log_info(kernel_log_obligatorio,"PID: %d - Escribir Archivo: %s - Puntero: %d - Dirección Memoria %d - Tamaño %d", pcb->pid, nombre_archivo, archivo_pcb->puntero, dir_fisica, archivo->size);

		pthread_mutex_lock(&mutex_lista_exec);
		list_remove(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
		log_blocked_proceso(pcb->pid, nombre_archivo);

		// Llamo al planificador para que envie un nuevo preceso a CPU.
		pcp_planificar_corto_plazo();

		// Le envio a File System  la solicitud de WRITE
		send_atender_F_read_write(nombre_archivo,archivo_pcb->puntero ,dir_fisica, pcb->pid, MANEJAR_F_WRITE_KF);
	}else{
		pcb->motivo_exit = INVALID_WRITE;
//		plp_planificar_proceso_exit(pcb->pid);
		plp_exit(pcb);
		// Llamo al planificador para que envie un nuevo preceso a CPU.
		pcp_planificar_corto_plazo();
	}
}

// ----- Para enviar a FS, de instruccion: F_READ-F_WRITE
void send_atender_F_read_write(char* nombre_archivo, int puntero_pcb, int dir_fisica, int pid_process, op_code code){
	pthread_mutex_lock(&mutex_peticion_fs);
	t_paquete* un_paquete = crear_super_paquete(MANEJAR_F_READ_KF);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	cargar_int_al_super_paquete(un_paquete, pid_process);
	cargar_int_al_super_paquete(un_paquete, puntero_pcb);
	cargar_int_al_super_paquete(un_paquete, dir_fisica);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

// ----- Para enviar a FS, de instruccion: F_OPEN-F_TRUNCATE
void send_atender_F_instruccion_fs(char* nombre_archivo, char* operacion ,int operacion_entero, int pid_process, op_code cod){
//	pthread_mutex_lock(&mutex_peticion_fs);
	t_paquete* un_paquete = crear_super_paquete(cod);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	if(strcmp(operacion , "ABRIR_ARCHIVO") == 0 || strcmp(operacion , "CREAR_ARCHIVO") == 0){
		cargar_string_al_super_paquete(un_paquete, operacion);
	}else{
		cargar_int_al_super_paquete(un_paquete, operacion_entero);
		cargar_int_al_super_paquete(un_paquete, pid_process);
	}
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

// Validar el F_OPEN a partir de la respuesta de File System
void validar_respuesta_F_open(char* operacion, int confirmacion, t_buffer* unBuffer){
	if(strcmp(operacion , "ABRIR_ARCHIVO") == 0){
		if(confirmacion == 1){
			char* nombre_archivo = recibir_string_del_buffer(unBuffer);
			int size_archivo = recibir_int_del_buffer(unBuffer);

			t_archivo* archivo = crear_archivo(nombre_archivo, size_archivo);

			list_add(lista_archivos_abiertos, archivo);

			pthread_mutex_lock(&mutex_existe_archivo);
			flag_existe_archivo = 1;
			pthread_mutex_unlock(&mutex_existe_archivo);
		}else{
			pthread_mutex_lock(&mutex_existe_archivo);
			flag_existe_archivo = 0;
			pthread_mutex_unlock(&mutex_existe_archivo);
		}
	}else if(strcmp(operacion , "CREAR_ARCHIVO") == 0){
		log_info(kernel_logger,"Se creo el archivo en FS");
	}

}

void plp_exit(t_pcb* pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que ese PCB siga en la lista
	if(list_remove_element(lista_execute, pcb)){
		liberar_recursos_pcb(pcb);
		avisar_a_memoria_para_liberar_estructuras(pcb);
		sem_wait(&sem_estructura_liberada);
		transferir_from_actual_to_siguiente(pcb, lista_exit, mutex_lista_exit, EXIT);
		pcb->motivo_exit = SUCCESS;
		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));
	}else{
		log_error(kernel_logger, "PCB no encontrada en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);
	plp_planificar_proceso_nuevo(NULL);
}

