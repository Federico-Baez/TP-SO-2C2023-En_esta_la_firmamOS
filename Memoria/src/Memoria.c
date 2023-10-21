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
	leer_log();

	list_procss_recibidos = list_create();
	//TODO: verificar como inicializar memoria
	inicializar_memoria();
	server_fd_memoria = iniciar_servidor(memoria_logger, IP_MEMORIA, PUERTO_ESCUCHA);
	while(server_escucha())

//	log_info(memoria_logger, "Finaliza servidor de Memoria");
//	instrucciones_para_cpu = leer_archivo_y_cargar_instrucciones(PATH_INSTRUCCIONES);
//	liberar_memoria_de_instrucciones(instrucciones_para_cpu);
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
//	log_info(memoria_logger, "Inicializando memoria");
//	espacio_usuario = malloc(TAM_MEMORIA);
//	if(espacio_usuario == NULL){
//			log_error(memoria_logger, "Fallo Malloc");
//	    	exit(1);
//	    }
//	lst_marco = list_create();
//	int cant_marcos = TAM_MEMORIA/TAM_PAGINA;
//	log_info(memoria_logger, "Tamanio de memoria %d y la cantidad de marcos %d", TAM_MEMORIA, cant_marcos);
//
//	for(int i=0;i<= cant_marcos;i++){
//		tabla_pagina* nuevo_marco  = crear_marco(TAM_PAGINA*i, true);
//
//		list_add(lst_marco,nuevo_marco);

	}
//
//}
//
//tabla_pagina* crear_marco(int base, bool presente){
//	tabla_pagina *nuevo_marco = malloc(sizeof(tabla_pagina));
//	nuevo_marco->marco = base;
//	nuevo_marco->presente = presente;
//	return nuevo_marco;
//}
//
//void destruir_marco(tabla_pagina* marco) {
//    free(marco);
//}

/*
tabla_paginas* crear_tabla_paginas(int pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
	log_debug(memoria_log_obligatorio,"[PAG]: Creo tabla de paginas PID %d", pid);
	char* spid[4];
	string_from_format(spid, "%d", pid);
	nueva_tabla->page = list_create();

	//es una variable que deberia de bloquear
	dictionary_put(tablas,spid, nueva_tabla);
	// aca deberia de desbloquear este recurso
	return nueva_tabla;


}
*/
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

//void atender_mensajes_filesystem(t_buffer* buffer){
//	int tamanio = recibir_int_del_buffer(buffer);
//	char* mensaje = recibir_string_del_buffer(buffer);
//	log_info(memoria_logger, "[FILESYSTEM]> ");
//	free(mensaje);
//	free(buffer);
//}
//void atender_mensajes_cpu(t_buffer* buffer){
//	int tamanio = recibir_int_del_buffer(buffer);
//	char* mensaje = recibir_string_del_buffer(buffer);
//	log_info(memoria_logger, "[CPU]> ");
//	free(mensaje);
//	free(buffer);
//}
/*----------------TODO COMUNICACION SOCKETS --------*/

void identificar_modulo(t_buffer* unBuffer, int conexion){
	int modulo_id = recibir_int_del_buffer(unBuffer);
	switch (modulo_id) {
		case KERNEL:
			fd_kernel = conexion;
			log_info(memoria_logger, "!!!!! KERNEL CONECTADO !!!!!");
			break;
		case CPU:
			fd_cpu = conexion;
			log_info(memoria_logger, "!!!!! CPU CONECTADO !!!!!");
			break;
		case FILESYSTEM:
			fd_filesystem = conexion;
			log_info(memoria_logger, "!!!!! FILESYSTEM CONECTADO !!!!!");
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
	int control_key = 1;
	while(control_key){
		int cod_op = recibir_operacion(cliente_socket);
		t_buffer* unBuffer;
		switch(cod_op){
		case MENSAJE:
			recibir_mensaje(memoria_logger, cliente_socket);
			break;
		case PAQUETE:
			//int* numero_xd = recibir_int(logger, coso)

			t_list* paquete_recibido = recibir_paquete_int(cliente_socket);
			//t_list* paquete_recibido = recibir_paquete(cliente_socket);
			log_info(memoria_logger, "Se reciben los siguientes paquetes: ");
			list_iterate(paquete_recibido, (void*)iterator);

			break;
		case ADMINISTRAR_PAGINA_MEMORIA:

			break;
		case IDENTIFICACION:
			unBuffer = recibiendo_super_paquete(cliente_socket);
			identificar_modulo(unBuffer, cliente_socket);
			break;
		//======= KERNEL ===========================
		case INICIAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			//
			agregar_proceso_a_listado(unBuffer, list_procss_recibidos);
			break;
		case LIBERAR_ESTRUCTURA_KM:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			//
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel);
			atender_mensajes_kernel(unBuffer);
			break;
		//======= CPU ===========================
		case PETICION_INFO_RELEVANTE_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
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
			//
			break;
		case CONSULTA_DE_PAGINA_CM:
			unBuffer = recibiendo_super_paquete(fd_cpu);
			//
			break;
		//======= FILESYSTEM ===========================
		case PETICION_ASIGNACION_BLOQUE_SWAP_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case LIBERAR_PAGINAS_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case PETICION_PAGE_FAULT_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case CARGAR_INFO_DE_LECTURA_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		case GUARDAR_INFO_FM:
			unBuffer = recibiendo_super_paquete(fd_filesystem);
			//
			break;
		//==================================================
		case PRUEBAS:

			break;

		case -1:
			control_key = 0;
			log_error(memoria_logger, "CLIENTE DESCONCETADO");
			break;
		default:
			log_error(memoria_logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}

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
	int cliente_socket = esperar_cliente(memoria_logger, server_name, server_fd_memoria );
	if(cliente_socket != -1){
		pthread_t hilo_cliente;
		int *args = malloc(sizeof(int));
		args = &cliente_socket;
//		*args = cliente_socket;
		pthread_create(&hilo_cliente, NULL, (void*) saludar_cliente, (void*)args);
		log_info(memoria_logger, "[THREAD] Creo hilo para atender");
		pthread_detach(hilo_cliente);
		return 1;
	}
	log_info(memoria_logger, "Se activa el servidor %s ", server_name);
	return EXIT_SUCCESS;
}


/******************************INSTRUCCIONES*****************************/
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo) {
    FILE* archivo = fopen(path_archivo, "rt");
    t_list* instrucciones = list_create();
    char* instruccion_formateada = NULL;
    int i = 0;

    if (archivo == NULL) {
        perror("No se encontró el archivo");
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(char));
    while (fgets(linea_instruccion, 256, archivo)) {
        char** l_instrucciones = string_split(linea_instruccion, " ");

        while (l_instrucciones[i]) {
            i++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->pseudo_c = strdup(l_instrucciones[0]);
        pseudo_cod->fst_param = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->snd_param = (i > 2) ? strdup(l_instrucciones[2]) : NULL;

        if (i == 3) {
            instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param);
        } else {
            instruccion_formateada = strdup(pseudo_cod->pseudo_c);
        }

        log_info(memoria_logger, "Se carga la instrucción %s", instruccion_formateada);
        list_add(instrucciones, instruccion_formateada);

        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }
        free(l_instrucciones);
        free(pseudo_cod->pseudo_c);
		if(pseudo_cod->fst_param) free(pseudo_cod->fst_param);
		if(pseudo_cod->snd_param) free(pseudo_cod->snd_param);
		free(pseudo_cod);
        i = 0; // Restablece la cuenta para la próxima iteración
    }

    fclose(archivo);
    free(linea_instruccion);
    return instrucciones;
}

void liberar_memoria_de_instrucciones(t_list* instrucciones){
	list_destroy_and_destroy_elements(instrucciones, free);
}

char* obtener_instruccion_por_indice(int indice_instruccion, t_list* instrucciones){

	char* instruccion_actual;
	return (indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones))
			? instruccion_actual = list_get(instrucciones,indice_instruccion) : NULL;

}
/******************************CARGAR INSTRUCCIONES*****************************/

/******************************FUNCIONES PARA PROCESOS*****************************/

proceso_recibido* obtener_proceso_por_id(int pid, t_list* lst_procesos){
	bool buscar_el_pid(void* proceso){
		return ((proceso_recibido*)proceso)->pid == pid;
	}
	proceso_recibido* un_proceso = list_find(lst_procesos, buscar_el_pid);
	return un_proceso;
}


void agregar_proceso_a_listado(t_buffer* unBuffer, t_list* lst_procesos_recibido){
	proceso_recibido* un_proceso = malloc(sizeof(proceso_recibido));
	if (un_proceso == NULL) {
	        perror("Error al reservar memoria para el proceso");
	        exit(EXIT_FAILURE);
	}
	un_proceso->pathInstrucciones = recibir_string_del_buffer(unBuffer);
	un_proceso->size = recibir_int_del_buffer(unBuffer);
	un_proceso->pid =recibir_int_del_buffer(unBuffer);
	log_info(memoria_logger, "Recibi el proceso con los siguientes datos archivo: %s, el tamanio: %d y el pid: %d ",un_proceso->pathInstrucciones, un_proceso->size, un_proceso->pid);
	list_add(lst_procesos_recibido, un_proceso);
	free(unBuffer);
	handhsake_modules(fd_kernel,"[MEMORIA]>Proceso cargado en Memoria OK");


}

void liberar_proceso(proceso_recibido* proceso) {
    for (int i = 0; i < list_size(proceso->instrucciones); i++) {
        char* instruccion = list_get(proceso->instrucciones, i);
        free(instruccion);
    }
    list_destroy(proceso->instrucciones);
    free(proceso->pathInstrucciones);
    free(proceso);
}
void liberar_listado_procesos(t_list* lst_procesos) {
    for (int i = 0; i < list_size(lst_procesos); i++) {
        proceso_recibido* proceso = list_get(lst_procesos, i);
        liberar_proceso(proceso);
    }
    list_destroy(lst_procesos);
}

/******************************FUNCIONES PARA CPU*****************************/

void enviar_instrucciones_a_cpu(int pid_buffer,int ip_buffer){
	t_paquete* paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);

	proceso_recibido* un_proceso = obtener_proceso_por_id(pid_buffer, list_procss_recibidos);
	char* instruccion = obtener_instruccion_por_indice(ip_buffer, un_proceso->instrucciones);
	cargar_string_al_super_paquete(paquete, instruccion);
	enviar_paquete(paquete, fd_cpu);
	eliminar_paquete(paquete);
}
