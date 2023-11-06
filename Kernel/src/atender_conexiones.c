#include "../include/atender_conexiones.h"

static void estructuras_inciadas_en_memoria_rpta(t_buffer* unBuffer){
	char* recibir_mensaje = (char*)recibir_choclo_del_buffer(unBuffer);
	printf("[%d]> %s\n", (int)strlen(recibir_mensaje), recibir_mensaje);
	free(recibir_mensaje);
}

static void _desalojar_proceso(t_pcb* una_pcb){

	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que esa PCB siga en la lista
	if(list_remove_element(lista_execute, una_pcb)){
		pthread_mutex_lock(&mutex_lista_exit);
		list_add(lista_exit, una_pcb);
//		una_pcb->estado = EXIT;
		cambiar_estado(una_pcb, EXIT);
		liberar_todos_los_recursos_de_una_pcb(una_pcb);
		avisar_a_memoria_para_liberar_estructuras(una_pcb);
		pthread_mutex_unlock(&mutex_lista_exit);

//		hay_pcb_elegida = false; // <---------------
	}else{
		log_error(kernel_logger, "PCB no encontradad en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	//-----------------------
	pthread_mutex_lock(&mutex_core);
	procesos_en_core--;
	pthread_mutex_unlock(&mutex_core);
	//-----------------------

	log_info(kernel_logger, "plp_exit [PID: %d]", una_pcb->pid);

//	hay_pcb_elegida = false; // <---------1111111

	pcp_planificar_corto_plazo();
	plp_planificar_proceso_nuevo(NULL);
	pcp_planificar_corto_plazo();
}

static void _reubicar_pcb_de_execute_a_ready(t_pcb* una_pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	if(!list_remove_element(lista_execute, una_pcb)){
		log_error(kernel_logger ,"PCB_%d No esta en EXECUTE - RArisimo", una_pcb->pid);
		exit(EXIT_FAILURE);
	}
//	hay_pcb_elegida = false; // <-------------11111
	pthread_mutex_unlock(&mutex_lista_exec);

//	pcp_planificar_corto_plazo();

	pthread_mutex_lock(&mutex_lista_ready);
	list_add(lista_ready, una_pcb);
	cambiar_estado(una_pcb, READY);
	pthread_mutex_unlock(&mutex_lista_ready);
//	hay_pcb_elegida = false;

//	plp_planificar_proceso_nuevo(NULL);
	pcp_planificar_corto_plazo();
}

static void _atender_motivo_desalojo(char* motivo_desalojo, t_buffer* un_buffer, t_pcb* una_pcb){
	if(strcmp(motivo_desalojo, "DESALOJO_POR_CONSOLA") == 0){
		_desalojar_proceso(una_pcb);
		batisenal_exit = false;
		printf("---------------<%d>\n", un_buffer->size);

	}else if(strcmp(motivo_desalojo, "ALGORITMO_QUANTUM") == 0 ||
			strcmp(motivo_desalojo, "ALGORITMO_PRIORIDAD") == 0){
		if(batisenal_exit){
			//Esto sirve para darle prioridad al desalojo por consola
			_desalojar_proceso(una_pcb);
			batisenal_exit = false;
		}else{
			_reubicar_pcb_de_execute_a_ready(una_pcb);
		}
		printf("---------------<%d>\n", un_buffer->size);

	}else if(strcmp(motivo_desalojo, "") == 0){
		/*Para el resto de los motivos, verificar los pcbs no tenga
		motivo de desalojo, porque puede pasar que el proceso se desaloje solo por alguna necesidad propia*/
		/*Propuesta, siempre verificar si la batiseñal de desalojo esta activada*/
		if(batisenal_exit){
			_desalojar_proceso(una_pcb);
			batisenal_exit = false;
		}else{
			//Si no hay batiseñal de desalojo, entonces procesder a atender la PCB segun su motivo normal
			//Hacer esta compŕobacion para todos los demas
		}

	}else if(strcmp(motivo_desalojo, "") == 0){

	}else if(strcmp(motivo_desalojo, "") == 0){

	}else if(strcmp(motivo_desalojo, "") == 0){

	}else if(strcmp(motivo_desalojo, "") == 0){

	}else if(strcmp(motivo_desalojo, "") == 0){

	}
}

static void _recibir_proceso_desalojado(t_buffer* un_buffer){
	pausador();
	int recibe_pid = recibir_int_del_buffer(un_buffer);
	t_pcb* una_pcb = buscar_pcb_por_pid(recibe_pid);
	if(una_pcb == NULL){
		log_error(kernel_logger, "PID recibido no coincide con PCB de ninguna lista");
		exit(EXIT_FAILURE);
	}

	//Controlando que la PCB encontrada corresponde a la lista execute
	if(!esta_pcb_en_una_lista_especifica(lista_execute, una_pcb)){
		log_error(kernel_logger, "PCB_%d (%d) - No se encuentró en la lista EXECUTE - Esto es RARO", una_pcb->pid, una_pcb->estado);
		exit(EXIT_FAILURE);
	}

	una_pcb->program_counter = recibir_int_del_buffer(un_buffer);
	uint32_t* re_RX;
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	una_pcb->registros_CPU->AX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	una_pcb->registros_CPU->BX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	una_pcb->registros_CPU->CX = *re_RX; free(re_RX);
	re_RX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	una_pcb->registros_CPU->DX = *re_RX; free(re_RX);

	char* motivo_desalojo = recibir_string_del_buffer(un_buffer);
	log_warning(kernel_logger, "<PID:%d>[PC:%d][%u|%u|%u|%u][%s]",
								una_pcb->pid,
								una_pcb->program_counter,
								una_pcb->registros_CPU->AX,
								una_pcb->registros_CPU->BX,
								una_pcb->registros_CPU->CX,
								una_pcb->registros_CPU->DX,
								motivo_desalojo);
	_atender_motivo_desalojo(motivo_desalojo, un_buffer, una_pcb);
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
			free(unBuffer);
			break;
		case LIBERAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
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
		case EJECUTAR_PROCESO_KC:
			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
//			CPU_en_uso = false; // <1111111111111111111
			_recibir_proceso_desalojado(unBuffer);
			free(unBuffer);
			break;
		case MANEJO_RECURSOS_CPK: //recibe: [char* motivo]("SIGNAL/WAIT"),[char* recurso]("RECURSO_N")
//			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
//			CPU_en_uso = false; // <1111111111111111111
//			_recibir_proceso_desalojado(unBuffer);
//			free(unBuffer);
			break;
		case PRUEBAS:
			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
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

static void _gestionar_peticiones_de_cpu_interrupt(){
	while(1){
		int cod_op = recibir_operacion(fd_cpu_interrupt);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de CPU_Interrupt");

		switch (cod_op) {
		case FORZAR_DESALOJO_KC:
			unBuffer = recibiendo_super_paquete(fd_cpu_interrupt);
			//
			break;
		case PRUEBAS:
			unBuffer = recibiendo_super_paquete(fd_cpu_interrupt);
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
	pthread_create(&hilo_cpu_interrupt, NULL, (void*)_gestionar_peticiones_de_cpu_interrupt, NULL);
	pthread_detach(hilo_cpu_interrupt);
}

