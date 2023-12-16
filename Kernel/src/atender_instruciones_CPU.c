#include "../include/atender_instrucciones_CPU.h"



// ----- LOG PROCESOS BLOQUEADOS -----
void log_blocked_proceso(int pid_process, char* motivo_block){
	log_info(kernel_log_obligatorio, "PID: %d - Bloqueado por: %s", pid_process, motivo_block);
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
			log_info(kernel_log_obligatorio, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, recurso_buscado->recurso_name, recurso_buscado->instancias-1);
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

// ----- F_OPEN -----
void atender_F_open(t_pcb* pcb, char* nombre_archivo, char* tipo_apertura){
	t_archivo* archivo = obtener_archivo_global(nombre_archivo);

	if(archivo == NULL){
		send_atender_f_open(nombre_archivo, "ABRIR_ARCHIVO");
		// Espero la respuesta del file system
		sem_wait(&sem_f_open_FS);
		archivo = obtener_archivo_global(nombre_archivo);
		pthread_mutex_lock(&mutex_existe_archivo);
		if(flag_existe_archivo == 0){
			archivo = crear_archivo(nombre_archivo, 0);
			list_add(lista_archivos_abiertos, archivo);
			log_info(kernel_logger, "Cree el archivo: %s", archivo->nombre_archivo);
			send_atender_f_open(nombre_archivo, "CREAR_ARCHIVO");
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
		t_archivo* archivo = obtener_archivo_global(nombre_archivo);
		archivo->size = nuevo_size_archivo;
		// Llamo al planificador para que envie un nuevo preceso a CPU.
		pcp_planificar_corto_plazo();

		send_atender_f_truncate(nombre_archivo, nuevo_size_archivo, pcb->pid);
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
		log_info(kernel_logger, "VOy a entrar a obtener archivo");

		t_archivo* archivo = obtener_archivo_global(nombre_archivo);
		log_info(kernel_logger, "Obtuve el archivo : %s ", archivo->nombre_archivo);
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

// ----- EXIT -----
void plp_exit(t_pcb* pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que ese PCB siga en la lista
	if(list_remove_element(lista_execute, pcb)){
		liberar_recursos_pcb(pcb);
		//liberar_archivo_pcb(pcb);
		avisar_a_memoria_para_liberar_estructuras(pcb);
		sem_wait(&sem_estructura_liberada);
		transferir_from_actual_to_siguiente(pcb, lista_exit, mutex_lista_exit, EXIT);
		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));
		pthread_mutex_lock(&mutex_core);
		procesos_en_core--;
		pthread_mutex_unlock(&mutex_core);
	}else{
		log_error(kernel_logger, "PCB no encontrada en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);
	plp_planificar_proceso_nuevo(NULL);
}

// ----- FUNCIONALIDADES -----
t_archivo* crear_archivo(char* nombre_archivo, int size_archivo){
	t_archivo* archivo = malloc(sizeof(t_archivo));
	archivo->nombre_archivo = strdup(nombre_archivo);
	archivo->cola_block_procesos = list_create();
	pthread_mutex_init(&(archivo->mutex_cola_block), NULL);
	archivo->size = size_archivo;

	archivo->lock_escritura = malloc(sizeof(t_lock_escritura));
	archivo->lock_escritura->locked = 0;
	archivo->lock_escritura->pcb = NULL;

	archivo->lock_lectura = malloc(sizeof(t_lock_lectura));
	archivo->lock_lectura->locked = 0;
	archivo->lock_lectura->lista_participantes = list_create();
	pthread_mutex_init(&(archivo->lock_lectura->mutex_lista_asiganada), NULL);

	return archivo;
}

t_archivo* obtener_archivo_global(char* nombre_archivo){
	t_archivo* archivo;

	for(int i = 0; i < list_size(lista_archivos_abiertos); i++){
		archivo = list_get(lista_archivos_abiertos, i);
		if(strcmp(archivo->nombre_archivo, nombre_archivo) == 0){
				return archivo;
		}
	}
	return NULL;
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

// ----- ENVIOS A FILE SYSTEM -----

// ----- Para enviar a FS F_OPEN
void send_atender_f_open(char* nombre_archivo, char* operacion){
	t_paquete* un_paquete = crear_super_paquete(MANEJAR_F_OPEN_KF);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	cargar_string_al_super_paquete(un_paquete, operacion);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

// ----- Para enviar a FS F_TRUNCATE
void send_atender_f_truncate(char* nombre_archivo, int nuevo_tamanio, int pid_process){
	t_paquete* un_paquete = crear_super_paquete(MANEJAR_F_TRUNCATE_KF);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	cargar_int_al_super_paquete(un_paquete, nuevo_tamanio);
	cargar_int_al_super_paquete(un_paquete, pid_process);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

// ----- Para enviar a FS, de instruccion: F_READ-F_WRITE
void send_atender_F_read_write(char* nombre_archivo, int puntero_pcb, int dir_fisica, int pid_process, op_code code){
	t_paquete* un_paquete = crear_super_paquete(code);
	cargar_string_al_super_paquete(un_paquete, nombre_archivo);
	cargar_int_al_super_paquete(un_paquete, pid_process);
	cargar_int_al_super_paquete(un_paquete, puntero_pcb);
	cargar_int_al_super_paquete(un_paquete, dir_fisica);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}







