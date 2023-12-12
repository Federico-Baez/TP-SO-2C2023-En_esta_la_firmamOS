#include "../include/inicializar_estructuras.h"

static void iniciar_logger(){
	kernel_logger = log_create("kernel.log", "[Kernel]", 1, LOG_LEVEL_INFO);
	kernel_log_obligatorio = log_create("kernel_log_obligatorio.log", "[Kernel - Log obligatorio]", 1, LOG_LEVEL_INFO);

}

static void iniciar_config(char* config_path){
	kernel_config = config_create(config_path); //Esto quiza lo descomentemos para las pruebas
	//kernel_config = config_create("kernel.config");

	if(kernel_config == NULL){
		log_error(kernel_logger, "No se encontro el path del config\n");
		config_destroy(kernel_config);
		log_destroy(kernel_logger);
		log_destroy(kernel_log_obligatorio);
		exit(2);
	}

	IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernel_config,"PUERTO_MEMORIA");
	IP_FILESYSTEM = config_get_string_value(kernel_config,"IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(kernel_config,"PUERTO_FILESYSTEM");
	IP_CPU = config_get_string_value(kernel_config,"IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config,"PUERTO_CPU_INTERRUPT");
	char* algoritmo_planificacion = config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION");
	QUANTUM = config_get_int_value(kernel_config,"QUANTUM");
	RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
	INSTANCIAS_RECURSOS = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS"); //TODO: tratar de convertirlo en un array de ints
	GRADO_MULTIPROGRAMACION_INI = config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION_INI");

	if(strcmp(algoritmo_planificacion, "FIFO") == 0) {
			ALGORITMO_PLANIFICACION = FIFO;
	} else if (strcmp(algoritmo_planificacion, "RR") == 0) {
		ALGORITMO_PLANIFICACION = ROUNDROBIN;
	} else if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
		ALGORITMO_PLANIFICACION = PRIORIDADES;
	} else {
		log_error(kernel_logger, "No se encontro el algoritmo de planificacion de corto plazo");
	}

}

static void iniciar_listas(){
	lista_new = list_create();
	lista_ready = list_create();
	lista_execute = list_create();
	lista_blocked = list_create();
	lista_exit = list_create();

	lista_general = list_create();
	lista_recursos = list_create();
	lista_archivos_abiertos = list_create();
}

static void iniciar_semaforos(){
	sem_init(&sem_pausa, 0, 0);
	sem_init(&sem_enviar_interrupcion, 0, 0);
	sem_init(&sem_estructura_iniciada, 0, 0);
	sem_init(&sem_estructura_liberada, 0, 0);
	sem_init(&sem_nuevo_en_block, 0, 0);
}

static void iniciar_pthread(){
	pthread_mutex_init(&mutex_lista_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_lista_exit, NULL);
	pthread_mutex_init(&mutex_lista_general, NULL);

	pthread_mutex_init(&mutex_process_id, NULL);
	pthread_mutex_init(&mutex_core, NULL);
	pthread_mutex_init(&mutex_pausa, NULL);
	pthread_mutex_init(&mutex_recurso, NULL);
	pthread_mutex_init(&mutex_ticket, NULL);
	pthread_mutex_init(&mutex_enviar_interrupcion, NULL);

	pthread_mutex_init(&mutex_flag_finalizar_proceso, NULL);
	pthread_mutex_init(&mutex_manejo_page_fault, NULL);
	pthread_mutex_init(&mutex_existe_archivo, NULL);
	pthread_mutex_init(&mutex_flag_proceso_desalojado, NULL);
	pthread_mutex_init(&mutex_peticion_fs, NULL);
}

static void _iniciar_recursos(){
	int i = 0;
	while(RECURSOS[i] != NULL){
		t_recurso* recurso = malloc(sizeof(t_recurso));
		recurso->recurso_name = RECURSOS[i];
		recurso->instancias = atoi(INSTANCIAS_RECURSOS[i]);
		recurso->pcb_asignado = NULL;
		recurso->lista_bloqueados = list_create();
		list_add(lista_recursos, recurso);
		i++;

		pthread_mutex_init(&recurso->mutex_bloqueados, NULL);

		log_info(kernel_logger, "[%s | %d]", recurso->recurso_name, recurso->instancias);
	}
}

/*==========================================*/

void inicializar_kernel(char* config_path){
	iniciar_logger();
	iniciar_config(config_path);

	iniciar_listas();
	iniciar_semaforos();
	iniciar_pthread();

	_iniciar_recursos();

}


