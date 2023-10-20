#include "../include/Kernel.h"

int main(int argc, char** argv) {
	kernel_logger = log_create("kernel.log", "[Kernel]", 1, LOG_LEVEL_INFO);
	kernel_log_obligatorio = log_create("kernel_log_obligatorio.log", "[Kernel - Log obligatorio]", 1, LOG_LEVEL_INFO);

	kernel_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//kernel_config = config_create("kernel.config");

	if(kernel_config == NULL){
		log_error(kernel_logger, "No se encontro el path del config\n");
		config_destroy(kernel_config);
		log_destroy(kernel_logger);
		log_destroy(kernel_log_obligatorio);
		exit(2);
	}

	leer_config(kernel_config);

	pthread_t hilo_cpu_dispatch, hilo_cpu_interrupt;
	pthread_t hilo_memoria;
	pthread_t hilo_consola;
	pthread_t hilo_filesystem;
	//pthread_t hilo_experimentos_xd;

	// ---------------- LE DAMOS CORRIENTE A LOS SEMAFOROS ----------------
	iniciar_semaforos();
	iniciar_pthread();
	iniciar_listas();

	//Probando conexiones
	fd_cpu_dispatcher = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);


	pthread_create(&hilo_filesystem, NULL, (void*)atender_filesystem, NULL);
	pthread_detach(hilo_filesystem);

	pthread_create(&hilo_cpu_dispatch, NULL, (void*)atender_cpu_dispatch, NULL);
	pthread_detach(hilo_cpu_dispatch);
	pthread_create(&hilo_cpu_interrupt, NULL, (void*)atender_cpu_interrupt, NULL);
	pthread_detach(hilo_cpu_interrupt);

	pthread_create(&hilo_memoria, NULL, (void*)atender_memoria, NULL);
	pthread_detach(hilo_memoria);

//	pthread_create(&hilo_experimentos_xd, NULL, (void*)atender_experimentos_xd, NULL);
//	pthread_detach(hilo_experimentos_xd);

	pthread_create(&hilo_consola, NULL, (void*)leer_consola, NULL);
	pthread_join(hilo_consola, NULL);


	finalizar_kernel();

	return EXIT_SUCCESS;
}


void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	IP_FILESYSTEM = config_get_string_value(config,"IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(config,"PUERTO_FILESYSTEM");
	IP_CPU = config_get_string_value(config,"IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
	char* algoritmo_planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	asignar_planificador_cp(algoritmo_planificacion);
	QUANTUM = config_get_int_value(config,"QUANTUM");
	RECURSOS = config_get_array_value(config, "RECURSOS");
	INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS"); //TODO: tratar de convertirlo en un array de ints
	GRADO_MULTIPROGRAMACION_INI = config_get_int_value(config,"GRADO_MULTIPROGRAMACION_INI");

}


void leer_consola(){
	char* leido;
	leido = readline("> ");
	char** ingreso = string_split(leido, " ");
	t_pcb* pcb;

	// Despues hay que actualizar la consola para que no tire error
	while(strcmp(leido,"\0") != 0){
		//log_info(kernel_logger, "%s [%d]",leido, (int)strlen(leido));
		if(string_equals_ignore_case(ingreso[0], "INICIAR_PROCESO")){
			log_info(kernel_logger, "Selecionaste INICIAR_PROCESO");
			pcb = iniciar_pcb(atoi(ingreso[3]));
			log_info(kernel_log_obligatorio, "Se crea el proceso %d en NEW", pcb -> pid);

			agregar_pcb_lista(pcb, list_new, mutex_list_new);
			if(!list_is_empty(list_new)){
				inicializar_estructura(fd_memoria, ingreso[1], atoi(ingreso[2]), pcb);
			}
		}else if(string_equals_ignore_case(ingreso[0], "FINALIZAR_PROCESO")){
			//
		}else if(string_equals_ignore_case(ingreso[0], "DETENER_PLANIFICACION")){
			//
		}else if(string_equals_ignore_case(ingreso[0], "INICIAR_PLANIFICACION")){
			//
		}else if(string_equals_ignore_case(ingreso[0], "MULTIPROGRAMACION")){
			//
		}else if(string_equals_ignore_case(ingreso[0], "PROCESO_ESTADO")){
			//
		}else if(string_equals_ignore_case(ingreso[0], "SALIR")){
			free(leido);
			free(ingreso);
			break;
		}
		free(leido);
		free(ingreso);
		leido = readline("> ");
		ingreso = string_split(leido, " ");
	}

	free(leido);
}

void finalizar_kernel(){
	log_destroy(kernel_logger);
	log_destroy(kernel_log_obligatorio);
	config_destroy(kernel_config);
	liberar_conexion(fd_cpu_dispatcher);
	liberar_conexion(fd_cpu_interrupt);
	liberar_conexion(fd_filesystem);
	liberar_conexion(fd_memoria);
	free(INSTANCIAS_RECURSOS);
	free(RECURSOS);
}

void asignar_planificador_cp(char* algoritmo_planificacion){
	if (strcmp(algoritmo_planificacion, "FIFO") == 0) {
			ALGORITMO_PLANIFICACION = FIFO;
		} else if (strcmp(algoritmo_planificacion, "RR") == 0) {
			ALGORITMO_PLANIFICACION = ROUNDROBIN;
		} else if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
			ALGORITMO_PLANIFICACION = PRIORIDADES;
		} else {
			log_error(kernel_logger, "No se encontro el algoritmo de planificacion de corto plazo");
		}
}

void atender_experimentos_xd(){
//	t_paquete* paquete = crear_super_paquete(MENSAJES_POR_CONSOLA);
	int i;
	char* cadena = string_new();
	sleep(2);
	for(i=1; i<=50; i++){
		t_paquete* paquete = crear_super_paquete(MENSAJES_POR_CONSOLA);
		cadena = string_repeat('A', i);
		cargar_int_al_super_paquete(paquete, strlen(cadena)+1);
		cargar_string_al_super_paquete(paquete, cadena);
		enviar_paquete(paquete, fd_cpu_dispatcher);
		eliminar_paquete(paquete);
		sleep(1);
	}
}

void atender_esta_prueba(t_buffer* myBuffer){
	//
}

void atender_memoria(){
	gestionar_handshake_como_cliente(fd_memoria, "MEMORIA", kernel_logger);
	identificarme_con_memoria(fd_memoria, KERNEL);
	log_info(kernel_logger, "HANDSHAKE CON MEMORIA [EXITOSO]");

	//int control_key = 1;
	while(1){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de MEMORIA");

		switch (cod_op) {
		case ESTRUCTURA_INICIADA_KM_OK:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
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
void atender_filesystem(){
	gestionar_handshake_como_cliente(fd_filesystem, "FILESYSTEM", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON FILESYSTEM [EXITOSO]");

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
void atender_cpu_dispatch(){
	gestionar_handshake_como_cliente(fd_cpu_dispatcher, "CPU_Dispatch", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON CPU_Dispatch [EXITOSO]");
	while(1){
		int cod_op = recibir_operacion(fd_cpu_dispatcher);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de FILESYSTEM");

		switch (cod_op) {
		case EJECUTAR_PROCESO_KC:
			unBuffer = recibiendo_super_paquete(fd_cpu_dispatcher);
			//
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
void atender_cpu_interrupt(){
	gestionar_handshake_como_cliente(fd_cpu_interrupt, "CPU_Interrupt", kernel_logger);
	log_info(kernel_logger, "HANDSHAKE CON CPU_Interrupt [EXITOSO]");
	while(1){
		int cod_op = recibir_operacion(fd_cpu_interrupt);
		t_buffer* unBuffer;
		log_info(kernel_logger, "Se recibio algo de FILESYSTEM");

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

void iniciar_semaforos(){
	sem_init(&sem_init_pcb, 0, 1);
	sem_init(&sem_grado_multiprogramacion, 0, 1);
}

void iniciar_pthread(){
	pthread_mutex_init(&mutex_list_new, NULL);
	pthread_mutex_init(&mutex_list_ready, NULL);
}

void iniciar_listas(){
	list_new = list_create();
	list_ready = list_create();
	list_execute = list_create();
	list_blocked = list_create();
}

// ------ Inicializar proceso ------

t_pcb* iniciar_pcb(int prioridad){
	sem_wait(&sem_init_pcb);

	t_pcb* new_pcb = crear_pcb(process_id, prioridad);
	process_id ++;

	sem_post(&sem_init_pcb);

	return new_pcb;
}

void agregar_pcb_lista(t_pcb* pcb, t_list* list_estado, pthread_mutex_t mutex_list_new){
	pthread_mutex_lock(&mutex_list_new);
	list_add(list_estado, pcb);
	pthread_mutex_unlock(&mutex_list_new);
}

void transferir_from_new_to_ready(){
//	sem_wait(&sem_grado_multiprogramacion);   // Queda comentado hasta que agreguemos el sem_post, porque todavia no esta hecho el finalizar proceso
	t_pcb* pcb;

	pthread_mutex_lock(&mutex_list_new);
	pcb = list_remove(list_new, 0);
	pthread_mutex_unlock(&mutex_list_new);

	cambiar_estado_pcb(pcb, READY);
	agregar_pcb_lista(pcb, list_ready, mutex_list_ready);

	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: NEW - Estado Actual: READY", pcb -> pid);
}

void inicializar_estructura(int fd_memoria, char* path, int size, t_pcb* pcb){
	transferir_from_new_to_ready();

	t_paquete* paquete = crear_super_paquete(INICIAR_ESTRUCTURA_KM);
	cargar_string_al_super_paquete(paquete, path);
	cargar_int_al_super_paquete(paquete, size);
	cargar_int_al_super_paquete(paquete, pcb -> pid);

	enviar_paquete(paquete, fd_memoria);
	eliminar_paquete(paquete);
}
