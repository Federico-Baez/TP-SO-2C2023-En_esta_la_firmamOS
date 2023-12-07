/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "../include/Memoria.h"


int main(int argc, char** argv) {
	memoria_logger = log_create("Memoria.log", "[Memoria]", 1, LOG_LEVEL_INFO);
	memoria_log_obligatorio = log_create("Memoria_log_obligatorio.log", "[Memoria- Log obligatorio]", 1, LOG_LEVEL_INFO);

	memoria_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//char* config_path = "../memoria.config";
	//memoria_config = config_create(config_path);

	if(memoria_config == NULL){
		log_error(memoria_logger, "No se encontro el path \n.");
		config_destroy(memoria_config);
		log_destroy(memoria_logger);
		exit(1);
	}

	leer_config(memoria_config);

//	leer_log();

	list_procss_recibidos = list_create();
	//TODO: verificar como inicializar memoria
	inicializar_memoria();
	server_fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);
	while(server_escucha())

	finalizar_memoria();

	return EXIT_SUCCESS;
}
void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(config, "IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(config, "PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
	PATH_INSTRUCCIONES = config_get_string_value(config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(config, "RETARDO_RESPUESTA");
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
}

void leer_log(){
	log_info(memoria_logger, "IP_MEMORIA: %s", IP_MEMORIA);
	log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
	log_info(memoria_logger, "IP_FILESYSTEM: %s", IP_FILESYSTEM);
	log_info(memoria_logger, "PUERTO_FILESYSTEM: %s", PUERTO_FILESYSTEM);
	log_info(memoria_logger, "TAM_MEMORIA: %d",TAM_MEMORIA);
	log_info(memoria_logger, "TAM_PAGINA: %d",TAM_PAGINA);
	log_info(memoria_logger, "PATH_INSTRUCCIONES: %s",PATH_INSTRUCCIONES);
	log_info(memoria_logger, "RETARDO_RESPUESTA: %d",RETARDO_RESPUESTA);
	log_info(memoria_logger, "ALGORITMO DE ASIGNACION: %s \n",ALGORITMO_REEMPLAZO);

}
void inicializar_memoria(){

	espacio_usuario = malloc(TAM_MEMORIA);
//	memset(espacio_usuario,',',TAM_MEMORIA);
	if(espacio_usuario == NULL){
			log_error(memoria_logger, "Fallo Malloc");
	    	exit(1);
	    }
	tablas = dictionary_create();
	log_info(memoria_logger, "Se inicia memoria con Paginacion.\n");
	log_info(memoria_logger, "Algoritmo de reemplazo a usar: %s\n", ALGORITMO_REEMPLAZO);

	lst_marco = list_create();
	int cant_marcos = TAM_MEMORIA/TAM_PAGINA;

	for(int i=0;i< cant_marcos;i++){
		marco* nuevo_marco  = crear_marco(TAM_PAGINA*i, true, i);

		list_add(lst_marco,nuevo_marco);

	}

	pthread_mutex_init(&mutex_lst_marco, NULL);

}

void* buscar_tabla(int pid){
	char* spid = string_itoa(pid);
	void* tabla = dictionary_get(tablas, spid);
	free(spid);
	return tabla;

}


void finalizar_memoria(){
//	list_destroy_and_destroy_elements(lst_marco, (void*)destruir_marco);
//	free(espacio_usuario);
	log_destroy(memoria_logger);
	log_destroy(memoria_log_obligatorio);
	config_destroy(memoria_config);
//    log_info(memoria_logger, "Memoria finalizada correctamente");


//	liberar_conexion(fd_cpu);
//	liberar_conexion(fd_filesystem);
//	liberar_conexion(fd_kernel);
}

void atender_mensajes_kernel(t_buffer* buffer){
	int tamanio = recibir_int_del_buffer(buffer);
	char* mensaje = recibir_string_del_buffer(buffer);
	log_info(memoria_logger, "[KERNEL]> [%d]%s", tamanio, mensaje);
	free(mensaje);
	free(buffer);
}

/*----------------TODO COMUNICACION SOCKETS --------*/

void identificar_modulo(t_buffer* unBuffer, int conexion){
	int modulo_id = recibir_int_del_buffer(unBuffer);
	switch (modulo_id) {
		case KERNEL:
			fd_kernel = conexion;
			log_info(memoria_logger, "!!!!! KERNEL CONECTADO !!!!!");
			atender_kernel(fd_kernel);

			break;
		case CPU:
			fd_cpu = conexion;
			log_info(memoria_logger, "!!!!! CPU CONECTADO !!!!!");
			atender_cpu(fd_cpu);

			break;
		case FILESYSTEM:
			fd_filesystem = conexion;
			log_info(memoria_logger, "!!!!! FILESYSTEM CONECTADO !!!!!");
			atender_filesystem(fd_filesystem);

			break;
		default:
			log_error(memoria_logger, "[%d]Error al identificar modulo",modulo_id);
			exit(EXIT_FAILURE);
			break;
	}
}



static void procesar_conexion(void *void_args){
	int* args = (int*) void_args;
	int cliente_socket = *args;

		int cod_op = recibir_operacion(cliente_socket);
		t_buffer* unBuffer;
		switch(cod_op){
		case IDENTIFICACION:
			unBuffer = recibiendo_super_paquete(cliente_socket);
			identificar_modulo(unBuffer, cliente_socket);

			break;
		case -1:
			log_error(memoria_logger, "CLIENTE DESCONCETADO");
			close(cliente_socket);
			break;
		default:
			log_error(memoria_logger, "Operacion desconocida. No quieras meter la pata en [MEMORIA]");
			break;
		}
		free(unBuffer);


}

void atender_kernel(int cliente_socket) {
    int control_key = 1;
    while(control_key){
    t_buffer* unBuffer;
    int cod_op = recibir_operacion(cliente_socket);
		switch(cod_op) {
				case INICIAR_ESTRUCTURA_KM:
					printf("Se un proceso nuevo\n");
					unBuffer = recibiendo_super_paquete(fd_kernel);
					agregar_proceso_a_listado(unBuffer, list_procss_recibidos);

//					printf("Se libera el buffer\n");
					break;
				case LIBERAR_ESTRUCTURA_KM:
					unBuffer = recibiendo_super_paquete(fd_kernel);
					int pid = recibir_int_del_buffer(unBuffer);
					proceso_recibido* proceso_a_liberar = obtener_proceso_por_id(pid, list_procss_recibidos);
					liberar_proceso(proceso_a_liberar);
					log_warning(memoria_logger, "Se liberaron las estructuras del proceso: PID_%d", pid);
//					free(unBuffer);
					//
					break;
				case MENSAJES_POR_CONSOLA:
					unBuffer = recibiendo_super_paquete(fd_kernel);
					atender_mensajes_kernel(unBuffer);
//					free(unBuffer);
					break;
			case -1:
				log_error(memoria_logger, "[DESCONEXION]: KERNEL");
				close(cliente_socket);
				control_key = 0;
	//            exit(EXIT_FAILURE);
				// Otras acciones que quieras tomar en caso de desconexión
				return;  // Salimos de la función

			default:
				log_error(memoria_logger, "Operacion desconocida KERNEL");
				break;
			}
		free(unBuffer);
    }

}
void atender_cpu(int cliente_socket) {
    int control_key = 1;
    //[FALTA]Enviar tamaño de pagina a CPU
	while(control_key){
		t_buffer* unBuffer;
		int cod_op = recibir_operacion(cliente_socket);
		switch(cod_op) {
				case PETICION_INFO_RELEVANTE_CM:
					unBuffer = recibiendo_super_paquete(fd_cpu);
//					free(unBuffer);
					//
					break;
				case PETICION_DE_INSTRUCCIONES_CM:
					unBuffer = recibiendo_super_paquete(fd_cpu); //recibo el [pId] y el [PC]
					int pid_buffer = recibir_int_del_buffer(unBuffer);
					int ip_buffer = recibir_int_del_buffer(unBuffer);
					enviar_instrucciones_a_cpu(pid_buffer,ip_buffer);

					break;
				case PETICION_DE_EJECUCION_CM:
					unBuffer = recibiendo_super_paquete(fd_cpu);
//					free(unBuffer);

					break;
				case CONSULTA_DE_PAGINA_CM:
					unBuffer = recibiendo_super_paquete(fd_cpu);
//					free(unBuffer);
					//
					break;
			case -1:
				log_error(memoria_logger, "[DESCONEXION]: CPU");
				close(cliente_socket);
				control_key = 0;
	//            exit(EXIT_FAILURE);
				// Otras acciones que quieras tomar en caso de desconexión
				return;  // Salimos de la función

			default:
				log_error(memoria_logger, "Operacion desconocida CPU");
				break;
			}
		free(unBuffer);
	}
}


void atender_filesystem(int cliente_socket){
    int control_key = 1;
	while(control_key){
	t_buffer* unBuffer;
	int cod_op = recibir_operacion(cliente_socket);

	switch(cod_op) {
		case PETICION_ASIGNACION_BLOQUE_SWAP_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			asignar_posicions_de_SWAP_a_tabla_de_paginas(unBuffer);
			free(unBuffer);
			//
			break;
		case LIBERAR_PAGINAS_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
//				free(unBuffer);
			//
			break;
		case PETICION_PAGE_FAULT_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
//				free(unBuffer);
			//
			break;
		case CARGAR_INFO_DE_LECTURA_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
//				free(unBuffer);
			//
			break;
		case GUARDAR_INFO_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
//				free(unBuffer);
			//
			break;
		case -1:
			log_error(memoria_logger, "[DESCONEXION]: FILESYSTEM");
			close(cliente_socket);

//	            exit(EXIT_FAILURE);
			// Otras acciones que quieras tomar en caso de desconexión
			return;  // Salimos de la función

		default:
			log_error(memoria_logger, "Operacion desconocida FILESYSTEM");
			break;
		}
	free(unBuffer);
	}
}

void iterator(int *value) {
	log_info(memoria_logger, "%d", *value);
}

void saludar_cliente(void *void_args){
	int* conexion = (int*) void_args;
	//int cliente_socket = *args;

	int code_op = recibir_operacion(*conexion);
	switch (code_op) {
		case HANDSHAKE:
			void* coso_a_enviar = malloc(sizeof(int));
			int respuesta = 1;
			memcpy(coso_a_enviar, &respuesta, sizeof(int));
			send(*conexion, coso_a_enviar, sizeof(int),0);
			free(coso_a_enviar);

			procesar_conexion(conexion);
			break;
		case -1:
			log_error(memoria_logger, "Desconexion en HANDSHAKE");
			break;
		default:
			log_error(memoria_logger, "ERROR EN HANDSHAKE: Operacion desconocida");
			break;
	}
}



int server_escucha(){
	server_name = "Memoria";
	log_info(memoria_logger, "Iniciando servidor %s",server_name);
	while(1) {
		int cliente_socket = esperar_cliente(memoria_logger, server_name, server_fd_memoria );
		if(cliente_socket != -1){
			pthread_t hilo_cliente;
			int *args = malloc(sizeof(int));
			*args = cliente_socket;
			pthread_create(&hilo_cliente, NULL, (void*) saludar_cliente, args); // Pasamos args directamente
			log_info(memoria_logger, "[THREAD] Creo hilo para atender");
			pthread_detach(hilo_cliente);
		}
	}
	return EXIT_SUCCESS;
}



/******************************INSTRUCCIONES*****************************/
//Movido a su modulo proceso_recibido


/******************************CARGAR INSTRUCCIONES*****************************/

/******************************FUNCIONES PARA PROCESOS*****************************/

//Movido a su modulo proceso_recibido





/******************************FUNCIONES PARA CPU*****************************/

void enviar_instrucciones_a_cpu(int pid_buffer,int ip_buffer){
	t_paquete* paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);

	proceso_recibido* un_proceso = obtener_proceso_por_id(pid_buffer, list_procss_recibidos);
	char* instruccion = obtener_instruccion_por_indice(ip_buffer, un_proceso->instrucciones);
	cargar_string_al_super_paquete(paquete, instruccion);
	enviar_paquete(paquete, fd_cpu);
	eliminar_paquete(paquete);
}




/******************************FUNCIONES AUXILIARES*****************************/

void bloquear_lista_tablas(){
	pthread_mutex_lock(&m_tablas);
	log_info(memoria_log_obligatorio, "[SEMAFORO]: Bloqueo lista de tabla \n");
}
void desbloquear_lista_tablas(){
	log_info(memoria_log_obligatorio, "[SEMAFORO]: Desbloqueo lista de tabla \n");
	pthread_mutex_unlock(&m_tablas);
}

