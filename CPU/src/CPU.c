#include <stdio.h>
#include <stdlib.h>

#include "../include/CPU.h"
#include "../include/CPU.h"
#include "../../Shared/include/protocolo.h"

int main(int argc, char** argv) {
	cpu_logger = log_create("cpu.log", "[CPU]", 1, LOG_LEVEL_INFO);
	cpu_log_obligatorio = log_create("cpu_log_obligatorio.log", "[CPU - Log obligatorio]", 1, LOG_LEVEL_INFO);

	cpu_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//cpu_config = config_create("cpu.config");

	if(cpu_config == NULL){
		log_error(cpu_logger, "No se encontro el path del config\n");
		config_destroy(cpu_config);
		log_destroy(cpu_logger);
		log_destroy(cpu_log_obligatorio);
		exit(2);
	}

	leer_config(cpu_config);

	iniciar_estructuras();
	iniciar_semaforos();

	server_fd_cpu_dispatch = iniciar_servidor(cpu_logger, IP_CPU, PUERTO_ESCUCHA_DISPATCH);
	server_fd_cpu_interrupt = iniciar_servidor(cpu_logger, IP_CPU, PUERTO_ESCUCHA_INTERRUPT);

	//log_info(cpu_logger, "Servidor listo para recibir a Kernel\n");

	pthread_t hilo_cpu_dispatch, hilo_cpu_interrupt, hilo_cpu_memoria;

	pthread_create(&hilo_cpu_memoria, NULL, (void*)atender_cpu_memoria, NULL);
	pthread_detach(hilo_cpu_memoria);

	pthread_create(&hilo_cpu_interrupt, NULL, (void*)atender_cpu_interrupt, NULL);
	pthread_detach(hilo_cpu_interrupt);

	//--------------------
//	pthread_t hilo_pruebas;
//	pthread_create(&hilo_pruebas, NULL, (void*)simulador_de_eventos, NULL);
//	pthread_detach(hilo_pruebas);
	//---------------------

	pthread_create(&hilo_cpu_dispatch, NULL, (void*)atender_cpu_dispatch, NULL);
	pthread_join(hilo_cpu_dispatch, NULL);

	finalizar_CPU();
	return EXIT_SUCCESS;
}

void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
}


void iterator(char* value) {
	log_info(cpu_logger,"%s", value);
}

void iniciar_estructuras(){
	opcode_strings= string_array_new();
	string_array_push(&opcode_strings, "SET");
	string_array_push(&opcode_strings, "SUM");
	string_array_push(&opcode_strings, "SUB");
	string_array_push(&opcode_strings, "JNZ");
	string_array_push(&opcode_strings, "SLEEP");
	string_array_push(&opcode_strings, "WAIT");
	string_array_push(&opcode_strings, "SIGNAL");
	string_array_push(&opcode_strings, "MOV_IN");
	string_array_push(&opcode_strings, "MOV_OUT");
	string_array_push(&opcode_strings, "F_OPEN");
	string_array_push(&opcode_strings, "F_CLOSE");
	string_array_push(&opcode_strings, "F_SEEK");
	string_array_push(&opcode_strings, "F_READ");
	string_array_push(&opcode_strings, "F_WRITE");
	string_array_push(&opcode_strings, "F_TRUNCATE");
	string_array_push(&opcode_strings, "EXIT");

	hay_que_desalojar = false;
	hay_que_desalojar_sin_mensaje = false;

//	motivo_desalojo = NULL;

	interrupt_proceso_id = NULL;
	interrupt_proceso_ticket = NULL;
	interrupt_motivo = NULL;

	mochila = NULL;

}

void iniciar_semaforos(){
	sem_init(&sem_control_fetch_decode, 0, 0);
	sem_init(&sem_control_decode_execute, 0, 0);
	sem_init(&sem_control_peticion_marco_a_memoria, 0, 0);
	sem_init(&sem_control_peticion_lectura_a_memoria, 0, 0);
	sem_init(&sem_control_peticion_escritura_a_memoria, 0, 0);
	sem_init(&sem_control_respuesta_kernel, 0, 0);

	pthread_mutex_init(&mutex_interruptFlag, NULL);
}

void destruir_semaforos(){
	sem_destroy(&sem_control_fetch_decode);
	sem_destroy(&sem_control_decode_execute);
	sem_destroy(&sem_control_peticion_marco_a_memoria);
	sem_destroy(&sem_control_peticion_lectura_a_memoria);
	sem_destroy(&sem_control_peticion_escritura_a_memoria);
	sem_destroy(&sem_control_respuesta_kernel);

	pthread_mutex_destroy(&mutex_interruptFlag);

	log_info(cpu_logger, "Semaforos destruidos");
}

void finalizar_CPU(){
	destruir_semaforos();
	string_array_destroy(opcode_strings);
}

static void atender_mensajes_kernel_v2(t_buffer* buffer, char* tipo_de_hilo){
	int tamanio = recibir_int_del_buffer(buffer);
//	char* mensaje = recibir_string_del_buffer(buffer);

	log_info(cpu_logger, "[KERNEL_%s]> [%d]", tipo_de_hilo, tamanio);
//	free(mensaje);
	//free(buffer->stream);
	free(buffer);
}

void atender_cpu_dispatch(){
	fd_kernel_dispatch = esperar_cliente(cpu_logger, "Kernel por dispatch", server_fd_cpu_dispatch);
	gestionar_handshake_como_server(fd_kernel_dispatch, cpu_logger);
	log_info(cpu_logger, "::::::::::: KERNEL CONECTADO POR DISPATCH ::::::::::::");
	while(1){
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		t_buffer* unBuffer;
		//log_info(cpu_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case EJECUTAR_PROCESO_KC: //Me debe llegar: [---][PID][Ticket][PC_program_counter][AX][BX][CX][DX]
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
			ejecutar_en_un_hilo_nuevo_detach((void*)atender_proceso_del_kernel, unBuffer);

			break;
		case RESPUESTA_INSTRUCCION_KC: //POr aca contesta el kernel los pedido de SIGNAL/WAIT
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
			//TODO: completar todos los caminos posibles de las respuestas a las instrucciones

			// recibo motivo[string] , estado[int]
			//char* instruccion = recibir_string_del_buffer(unBuffer);  por ahora no es necesario, si se agrega avisar a luca
			int estadoInstruccion = recibir_int_del_buffer(unBuffer);
			if(estadoInstruccion == -1){
				hay_que_desalojar_sin_mensaje = true;
			}
			sem_post(&sem_control_respuesta_kernel);
//			if(strcmp(instruccion, "WAIT") == 0){
//				if(estadoInstruccion == -1){
//					hay_que_desalojar_sin_mensaje = true;
//				}else{
//					sem_post(&sem_control_respuesta_kernel);
//				}
//			}else if(strcmp(instruccion, "SIGNAL") == 0){
//				if(estadoInstruccion == -1){
//					hay_que_desalojar_sin_mensaje = true;
//				}else{
//					sem_post(&sem_control_respuesta_kernel);
//				}
//			}else if(strcmp(instruccion, "F_OPEN") == 0){
//				if(estadoInstruccion == -1){
//					hay_que_desalojar_sin_mensaje = true;
//				}else{
//					sem_post(&sem_control_respuesta_kernel);
//				}
//			}else if(strcmp(instruccion, "F_CLOSE") == 0){
//				if(estadoInstruccion == -1){
//					hay_que_desalojar_sin_mensaje = true;
//				}else{
//					sem_post(&sem_control_respuesta_kernel);
//				}
//			}else if(strcmp(instruccion, "F_SEEK") == 0){
//				if(estadoInstruccion == -1){
//					hay_que_desalojar_sin_mensaje = true;
//				}else{
//					sem_post(&sem_control_respuesta_kernel);
//				}
//			}
			break;
		case MENSAJES_POR_CONSOLA: //Por aca contesta el kernel los pedido de SIGNAL/WAIT
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
			//post(sem)
			//atender_mensajes_kernel_v2(unBuffer, "Dispatch");
			break;
		case -1:
			log_error(cpu_logger, "[DESCONEXION]: KERNEL_Dispatch");
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida KERNEL_Dispatch");
			//free(unBuffer);
			break;
		}
	}
	log_info(cpu_logger, "Saliendo del hilo de CPU_DISPATCH - KERNEL");
}


static void _manejar_interrupcion(t_buffer* un_buffer){

	int* punteros[2];
	punteros[0] = malloc(sizeof(int));
	punteros[1] = malloc(sizeof(int));
	*punteros[0] = recibir_int_del_buffer(un_buffer);
	*punteros[1] = recibir_int_del_buffer(un_buffer);
	char* puntero_string = recibir_string_del_buffer(un_buffer);

	if(contexto != NULL){

		if(interrupt_proceso_id == NULL){
			//Esto es para atender una interrupcion para el contexto actual por primera vez
			interrupt_proceso_id = punteros[0];
			interrupt_proceso_ticket = punteros[1];
			interrupt_motivo = puntero_string;
			log_warning(cpu_logger, "INTERRUPCION RECIBIDA: <PID:%d>[T:%d][%s]",
									*interrupt_proceso_id,
									*interrupt_proceso_ticket,
									interrupt_motivo);
		}else{
			//ESto es para dar prioridad la desalojo por consola, por que es de eliminacion de PCB
			if(strcmp(puntero_string, "DESALOJO_POR_CONSOLA") == 0 || strcmp(puntero_string, "ALGORITMO_PRIORIDADES") == 0 || strcmp(puntero_string, "ALGORITMO_QUANTUM") == 0){
				free(interrupt_motivo);
				interrupt_motivo = puntero_string;
				free(punteros[0]);
				free(punteros[1]);
				log_warning(cpu_logger, "BATISENAL DE DESALOJO RECIBIDA");
			}
			// este else habria que sacarlo, consultarlo
			else{
				//Si es una interrupcion por otro motivo, basicamente la ignoramos porque ya hay una interrupcion vigente y de todas maneras va a desalojarse
				free(punteros[0]);
				free(punteros[1]);
				free(puntero_string);
				log_error(cpu_logger, "Ignnorar a esta interrupcion - atender esta advertencia porque se supone que el control de interrupcions va desde el kernel");
			}
		}

	}else{
		log_info(cpu_logger, "INTERRUPCION RECHAZADA XQ NO HAY PROCESOS CORRIENDO EN CPU ACTUALMENTE");
		free(punteros[0]);
		free(punteros[1]);
		free(puntero_string);
	}
}

void atender_cpu_interrupt(){
	fd_kernel_interrupt = esperar_cliente(cpu_logger, "Kernel por interrupt", server_fd_cpu_interrupt);
	gestionar_handshake_como_server(fd_kernel_interrupt, cpu_logger);
	log_info(cpu_logger, "::::::::::: KERNEL CONECTADO POR INTERRUPT ::::::::::::");
	while(1){
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		t_buffer* unBuffer;
		//log_info(cpu_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case FORZAR_DESALOJO_KC:
			unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);
			_manejar_interrupcion(unBuffer);
			pthread_mutex_lock(&mutex_interruptFlag);
			if(*interrupt_proceso_id == contexto->proceso_pid){
				interruptFlag = true;
			}
			pthread_mutex_unlock(&mutex_interruptFlag);
			free(unBuffer);
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);
			atender_mensajes_kernel_v2(unBuffer, "Interrupt");
			break;
		case -1:
			log_error(cpu_logger, "[DESCONEXION]: KERNEL_Interrupt");
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida KERNEL_Interrupt");
			//free(unBuffer);
			break;
		}
	}
	log_info(cpu_logger, "Saliendo del hilo de CPU_INTERRUPT - KERNEL");
}

void atender_cpu_memoria(){
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
	gestionar_handshake_como_cliente(fd_memoria, "MEMORIA", cpu_logger);
	identificarme_con_memoria(fd_memoria, CPU);
	log_info(cpu_logger, "HANDSHAKE CON MEMORIA [EXITOSO]");
	int seguir=1;
	while(seguir){
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer;
		//log_info(cpu_logger, "Se recibio algo de KERNEL");

		switch (cod_op) {
		case PETICION_INFO_RELEVANTE_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			tam_pagina = recibir_int_del_buffer(unBuffer);
			//
			break;
		case PETICION_DE_INSTRUCCIONES_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_recepcion_de_instruccion(unBuffer);
			free(unBuffer);
			break;
		case PETICION_DE_EJECUCION_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case CONSULTA_DE_PAGINA_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			marco =  recibir_int_del_buffer(unBuffer);
			sem_post(&sem_control_peticion_marco_a_memoria);
			//
			break;
		case LECTURA_BLOQUE_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			valorMarco = recibir_int_del_buffer(unBuffer);
			sem_post(&sem_control_peticion_lectura_a_memoria);
			//
			break;
		case ESCRITURA_BLOQUE_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			sem_post(&sem_control_peticion_escritura_a_memoria);
			//
			break;
		case -1:
			log_error(cpu_logger, "[DESCONEXION]: MEMORIA");
			seguir = 0;
			exit(EXIT_FAILURE);
			break;
		default:
			log_warning(cpu_logger, "Operacion desconocida MEMORIA");
			//free(unBuffer);
			break;
		}
	}
	log_info(cpu_logger, "Saliendo del hilo de CPU - MEMORIA");
}


// =======================================================

//Me debe llegar: [---][PID][PC_program_counter][AX][BX][CX][DX]
void atender_proceso_del_kernel(t_buffer* unBuffer){
	iniciar_estructuras_para_atender_al_proceso(unBuffer);

	print_proceso();
	int contador_prueba = 0;

	log_info(cpu_logger, "pid proceso pre while 1: %d", contexto -> proceso_pid);
	while(1){

		//Inicicar ciclo de instruccion
//		iniciar_ciclo_de_instruccion(); TODO: descomentar y probar

		printf(">>> Simulando que se ejecuto una instruccion [%d]\n", contador_prueba);
		contador_prueba++;
		sleep(4);



		// check interrupt

		//Controlar si requiere un desalojo sin mensaje a kernel, ya que este ya posee el conexto
		if(hay_que_desalojar_sin_mensaje){
			break;
		}

		//Controlar si requiere un desalojo voluntario
		if(hay_que_desalojar){
			break;
		}
		//Controlar si hay interrupciones
		pthread_mutex_lock(&mutex_interruptFlag);
		if(interruptFlag){
			break;
		}
		pthread_mutex_unlock(&mutex_interruptFlag);

	}
	printf("Saliste del while ---------------------\n");

	t_paquete* un_paquete = alistar_paquete_de_desalojo(DESALOJO_PROCESO_CPK);

	pthread_mutex_lock(&mutex_interruptFlag);
	if(interruptFlag){
		cargar_string_al_super_paquete(un_paquete, interrupt_motivo);

	}else{
		//La mochila debe incluir el motivo del desalojo}
		if(strcmp(instruccion_split[0], "SLEEP") == 0){
			contexto->proceso_ip = contexto->proceso_ip + 1;
		}
		cargar_choclo_al_super_paquete(un_paquete, mochila->buffer->stream, mochila->buffer->size);
	}
	pthread_mutex_unlock(&mutex_interruptFlag);

	if(!hay_que_desalojar_sin_mensaje){
		enviar_paquete(un_paquete, fd_kernel_dispatch);
	}

	eliminar_paquete(un_paquete);

	log_warning(cpu_logger, "Proceso_desalojado <PID:%d>", contexto->proceso_pid);
	destruir_estructuras_del_contexto_acttual();

	log_info(cpu_logger, "Todo el contexto se elimino correctamente .....");

}

//bool preguntando_si_hay_interrupciones_vigentes(){
//	bool respuesta = false;
//	if(interrupt_proceso_id != NULL){
//		if(strcmp(interrupt_motivo, "DESALOJO_POR_CONSOLA") == 0 || strcmp(interrupt_motivo, "ALGORITMO_PRIORIDADES") == 0 || strcmp(interrupt_motivo, "ALGORITMO_QUANTUM") == 0){
//			//validar solo PID
//			if(*interrupt_proceso_id == contexto->proceso_pid){
//				respuesta = true;
//			}
//		}
//	}
//	return respuesta;
//}



void iniciar_ciclo_de_instruccion(){

	//FETCH
	ciclo_de_instruccion_fetch();

	//DECODE
	sem_wait(&sem_control_fetch_decode);
	log_info(cpu_logger, "Paso el semaforo de FETCH -> DECODE");
	ciclo_de_instruccion_decode();

	//EXECUTE
	sem_wait(&sem_control_decode_execute);
	log_info(cpu_logger, "Paso el semaforo de DECODE -> EXECUTE");
	ciclo_de_instruccion_execute();

	string_array_destroy(instruccion_split);

}

void ciclo_de_instruccion_fetch(){
	log_info(cpu_logger, "PID: <%d> - FETCH - Program Counter: <%d>", contexto->proceso_pid, contexto->proceso_ip);
	t_paquete* un_paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);
	cargar_int_al_super_paquete(un_paquete, contexto->proceso_pid);
	cargar_int_al_super_paquete(un_paquete, contexto->proceso_ip);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}

void ciclo_de_instruccion_decode(){

	//Validando que exista el header de la instruccion
	if(validador_de_header(instruccion_split[0])){
		log_info(cpu_logger, "Instruccion Validada: [%s] -> OK", instruccion_split[0]);
		sem_post(&sem_control_decode_execute);
	}else{
//		log_error(cpu_logger, "Instruccion no encontrada: [PC: %d][Instruc_Header: %s]", *proceso_ip, instruccion_split[0]);
		printf("Instruccion no encontrada: [[%s]]\n", instruccion_split[0]);
		exit(EXIT_FAILURE); //[FALTA] Repensar como terminar el programa y destruir estructuras
	}

	//[FALTA] Solo atender esta parte las instrucciones Que requieran traduccion de direccion logica a fisica?
	//F_READ y F_WRITE

}

void ciclo_de_instruccion_execute(){
	if(strcmp(instruccion_split[0], "SET") == 0){//[SET][AX][22]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;
		uint32_t* registro_referido = detectar_registro(instruccion_split[1]);
		*registro_referido = strtoul(instruccion_split[1], NULL, 10);

	}else if(strcmp(instruccion_split[0], "SUM") == 0){//[SUM][destino:AX][origen:BX]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;
		uint32_t* registro_referido_destino = detectar_registro(instruccion_split[1]);
		uint32_t* registro_referido_origen = detectar_registro(instruccion_split[2]);
		*registro_referido_destino += *registro_referido_origen;

	}else if(strcmp(instruccion_split[0], "SUB") == 0){//[SUB][destino:AX][origen:BX]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;
		uint32_t* registro_referido_destino = detectar_registro(instruccion_split[1]);
		uint32_t* registro_referido_origen = detectar_registro(instruccion_split[2]);
		*registro_referido_destino -= *registro_referido_origen;

	}else if(strcmp(instruccion_split[0], "JNZ") == 0){// [JNZ][Registro][Instruccion]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		uint32_t* registro_referido = detectar_registro(instruccion_split[1]);
		if(*registro_referido != 0) {
			contexto->proceso_ip = atoi(instruccion_split[2]);
		}else{
			contexto->proceso_ip ++;
		}

	}else if(strcmp(instruccion_split[0], "SLEEP") == 0){// [SLEEP][tiempo]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1]);
		 /* Esta instrucción representa una syscall bloqueante.
		 * Se deberá devolver el Contexto de Ejecución actualizado al Kernel
		 * junto a la cantidad de segundos que va a bloquearse el proceso.*/
		//Enviar al KERNEL: [PID][IP][AX][BX][CX][DX]["SLEEP"][Tiempo]
		mochila = crear_super_paquete(100);
		cargar_string_al_super_paquete(mochila, "SLEEP"); //Motivo del desalojo
		cargar_int_al_super_paquete(mochila, atoi(instruccion_split[1])); //otros perametros necesarios, en este caso el tiempo

		hay_que_desalojar = true;

	}else if(strcmp(instruccion_split[0], "WAIT") == 0){// [WAIT][char* Recurso] /*Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.*/
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1]);

		contexto->proceso_ip = contexto->proceso_ip + 1;

		t_paquete* infoExtra = crear_super_paquete(100);
		cargar_string_al_super_paquete(infoExtra, instruccion_split[1]);// instruccion_split[1]: recurso

		//Envia a Kernel con motivo de WAIT de algun recurso
		enviarPaqueteKernelConInfoExtra("WAIT", infoExtra);

		sem_wait(&sem_control_respuesta_kernel);

		eliminar_paquete(infoExtra);

	}else if(strcmp(instruccion_split[0], "SIGNAL") == 0){// [SIGNAL][char* Recurso] /*Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.*/
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1]);

		contexto->proceso_ip = contexto->proceso_ip + 1;

		t_paquete* infoExtra = crear_super_paquete(100);
		cargar_string_al_super_paquete(infoExtra, instruccion_split[1]);// instruccion_split[1]: recurso

		//Envia a Kernel con motivo de SIGNAL de algun recurso
		enviarPaqueteKernelConInfoExtra("SIGNAL", infoExtra);

		sem_wait(&sem_control_respuesta_kernel);

		eliminar_paquete(infoExtra);

	}else if(strcmp(instruccion_split[0], "MOV_IN") == 0){// [MOV_IN][Registro][Direccion Logica]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

	    uint32_t* registro_destino = detectar_registro(instruccion_split[1]);
	    int direccion_logica = atoi(instruccion_split[2]);
	    uint32_t valor = solicitar_valor_memoria(direccion_logica);
	    if(valor != -1){ // NO hubo PF
			*registro_destino = valor;
	    }

	}else if(strcmp(instruccion_split[0], "MOV_OUT") == 0){// [MOV_OUT][Dir_logica][RX]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

		int direccion_logica = atoi(instruccion_split[1]);
		uint32_t* registro_partida = detectar_registro(instruccion_split[2]);
		int valorAEscribir = *registro_partida;
		// el chequeo del page fault lo hace mmu dentro de esta funcion, de lo contrario envia el mensaje a memoria para la escritura
		escribir_valor_memoria(direccion_logica, valorAEscribir);

	}else if(strcmp(instruccion_split[0], "F_OPEN") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;

		t_paquete* infoExtra = crear_super_paquete(100);
		cargar_string_al_super_paquete(infoExtra, instruccion_split[1]);// instruccion_split[1]: nombre archivo
		cargar_string_al_super_paquete(infoExtra, instruccion_split[2]);// instruccion_split[2]: modo apertura
		enviarPaqueteKernelConInfoExtra("F_OPEN", infoExtra);

		sem_wait(&sem_control_respuesta_kernel);

	}else if(strcmp(instruccion_split[0], "F_CLOSE") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1]);
		contexto->proceso_ip = contexto->proceso_ip + 1;

		t_paquete* infoExtra = crear_super_paquete(100);
		cargar_string_al_super_paquete(infoExtra, instruccion_split[1]);// instruccion_split[1]: nombre archivo
		enviarPaqueteKernelConInfoExtra("F_CLOSE", infoExtra);

		sem_wait(&sem_control_respuesta_kernel);

	}else if(strcmp(instruccion_split[0], "F_SEEK") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;

		t_paquete* infoExtra = crear_super_paquete(100);
		cargar_string_al_super_paquete(infoExtra, instruccion_split[1]);// instruccion_split[1]: nombre archivo
		cargar_int_al_super_paquete(infoExtra, atoi(instruccion_split[2]));// instruccion_split[2]: posicion
		enviarPaqueteKernelConInfoExtra("F_SEEK", infoExtra);

		sem_wait(&sem_control_respuesta_kernel);

	}else if(strcmp(instruccion_split[0], "F_READ") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

		int direccion_logica = atoi(instruccion_split[2]);
		int dir_fisica = MMU(direccion_logica);


		if(dir_fisica != -1){ //NO hubo PF
			mochila = crear_super_paquete(100);
			cargar_string_al_super_paquete(mochila, "F_READ"); //Motivo del desalojo
			cargar_string_al_super_paquete(mochila, instruccion_split[1]);// instruccion_split[1]: nombre archivo
			cargar_int_al_super_paquete(mochila, dir_fisica);

			hay_que_desalojar = true;
		}

	}else if(strcmp(instruccion_split[0], "F_WRITE") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

		int direccion_logica = atoi(instruccion_split[2]);
		int dir_fisica = MMU(direccion_logica);

		if(dir_fisica != -1){ //NO hubo PF
			mochila = crear_super_paquete(100);
			cargar_string_al_super_paquete(mochila, "F_WRITE"); //Motivo del desalojo
			cargar_string_al_super_paquete(mochila, instruccion_split[1]);// instruccion_split[1]: nombre archivo
			cargar_int_al_super_paquete(mochila, dir_fisica);

			hay_que_desalojar = true;
		}

	}else if(strcmp(instruccion_split[0], "F_TRUNCATE") == 0){
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", contexto->proceso_pid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
		contexto->proceso_ip = contexto->proceso_ip + 1;

		mochila = crear_super_paquete(100);
		cargar_string_al_super_paquete(mochila, "F_TRUNCATE"); //Motivo del desalojo
		cargar_string_al_super_paquete(mochila, instruccion_split[1]);// instruccion_split[1]: nombre archivo
		cargar_int_al_super_paquete(mochila, atoi(instruccion_split[2]));// instruccion_split[2]: tamaño nuevo

		hay_que_desalojar = true;

	}else if(strcmp(instruccion_split[0], "EXIT") == 0){// [EXIT]
		log_info(cpu_logger, "PID: <%d> - Ejecutando: <%s>", contexto->proceso_pid, instruccion_split[0]);

		mochila = crear_super_paquete(100);
		cargar_string_al_super_paquete(mochila, "EXIT"); //Motivo del desalojo
		hay_que_desalojar = true;
	}else{
		log_error(cpu_logger, "Nunca se deberia llegar aqui :(");
	}
}

//----------MMU------------//devuelve la direccion fisica o un -1 si hubo PF
int MMU(int dir_logica){

	int num_pagina = floor(dir_logica / tam_pagina); //[TODO] obtener el tam de pag en el mensaje de conexion con memoria
	int desplazamiento = dir_logica - num_pagina * tam_pagina;

	//Pidiendo pagina a MEMORIA, si la tiene, devuelve su marco, de lo contrario devuelve page fault
	t_paquete* paqueteMemoria = crear_super_paquete(CONSULTA_DE_PAGINA_CM);
	cargar_int_al_super_paquete(paqueteMemoria, contexto->proceso_pid);
	cargar_int_al_super_paquete(paqueteMemoria, num_pagina);

	enviar_paquete(paqueteMemoria, fd_memoria);
	eliminar_paquete(paqueteMemoria);

	sem_wait(&sem_control_peticion_marco_a_memoria);

	if(marco >= 0){
		log_info(cpu_log_obligatorio, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", contexto->proceso_pid, num_pagina, marco);
		int dir_fisica = marco *tam_pagina + desplazamiento;
		contexto->proceso_ip = contexto->proceso_ip + 1;
		return dir_fisica;
	}else{
		log_info(cpu_log_obligatorio, "Page Fault PID: <%d> - Página: <%d>", contexto->proceso_pid, num_pagina);
		mochila = crear_super_paquete(100);
		cargar_string_al_super_paquete(mochila, "PAGE_FAULT"); //Motivo del desalojo
		cargar_int_al_super_paquete(mochila, num_pagina); //Pagina que hizo PF
		hay_que_desalojar = true;

		return -1;
	}

}

int solicitar_valor_memoria(int dir_logica){
	int dir_fisica = MMU(dir_logica);

	if(dir_fisica == -1){
		return -1;
	}else{
		//Le pido a memoria el contenido del marco
		t_paquete* paqueteLecturaMemoria = crear_super_paquete(LECTURA_BLOQUE_CM);
		cargar_int_al_super_paquete(paqueteLecturaMemoria, contexto->proceso_pid);
		cargar_int_al_super_paquete(paqueteLecturaMemoria, dir_fisica);
		enviar_paquete(paqueteLecturaMemoria, fd_memoria);
		eliminar_paquete(paqueteLecturaMemoria);

		sem_wait(&sem_control_peticion_lectura_a_memoria);

		log_info(cpu_logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>", contexto->proceso_pid, dir_fisica, valorMarco);

		return valorMarco;
	}
}


void escribir_valor_memoria(int dir_logica, int valorAEscribir){
	int dir_fisica = MMU(dir_logica);

	if(dir_logica != -1){
		//Le pido a memoria escribir el contenido del registro en la direccion fisica
		t_paquete* paqueteEscrituraMemoria = crear_super_paquete(ESCRITURA_BLOQUE_CM);
		cargar_int_al_super_paquete(paqueteEscrituraMemoria, contexto->proceso_pid);
		cargar_int_al_super_paquete(paqueteEscrituraMemoria, dir_fisica);
		cargar_int_al_super_paquete(paqueteEscrituraMemoria, valorAEscribir);
		enviar_paquete(paqueteEscrituraMemoria, fd_memoria);
		eliminar_paquete(paqueteEscrituraMemoria);

		sem_wait(&sem_control_peticion_escritura_a_memoria);

		log_info(cpu_logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", contexto->proceso_pid, dir_fisica, valorAEscribir);
	}
}


/*Descargando contenido del buffer y actualizando el contenido de los registros*/
void iniciar_estructuras_para_atender_al_proceso(t_buffer*  unBuffer){
	contexto= (t_contexto*) malloc(sizeof(t_contexto));

	void* bufferRecibido = unBuffer->stream;

	int offset = 0;

	// TODO: Confirmar que kernel efectivamente mande el mensaje en este orden
	memcpy(&(contexto -> proceso_pid), (bufferRecibido + offset), sizeof(int));
	offset += sizeof(int);
	memcpy(&(contexto -> proceso_ticket), (bufferRecibido + offset), sizeof(int));
	offset += sizeof(int);
	memcpy(&(contexto -> proceso_ip), (bufferRecibido + offset), sizeof(int));
	offset += sizeof(int);
	memcpy(&(contexto -> AX), (bufferRecibido + offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(contexto -> BX), (bufferRecibido + offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(contexto -> CX), (bufferRecibido + offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(contexto -> DX), (bufferRecibido + offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	free(unBuffer);

//	contexto->proceso_pid = recibir_int_del_buffer(unBuffer);
//	contexto->proceso_ticket = recibir_int_del_buffer(unBuffer);
//	contexto->proceso_pid = recibir_int_del_buffer(unBuffer);
//	contexto->AX = recibir_choclo_del_buffer(unBuffer);
//	contexto->BX = (uint32_t)recibir_choclo_del_buffer(unBuffer);
//	contexto->CX = (uint32_t)recibir_choclo_del_buffer(unBuffer);
//	contexto->DX = (uint32_t)recibir_choclo_del_buffer(unBuffer);
}

/*Libera memoria de las estructuras inciiadas para desalojar al proceso*/
void destruir_estructuras_del_contexto_acttual(){
	free(contexto);
	contexto = NULL;

	if(interrupt_proceso_id != NULL){
		free(interrupt_proceso_id);
		free(interrupt_proceso_ticket);
		free(interrupt_motivo);
		interrupt_proceso_id = NULL;
		interrupt_proceso_ticket = NULL;
		interrupt_motivo = NULL;
	}

	if(mochila != NULL){
		eliminar_paquete(mochila);
		mochila = NULL;
	}
	pthread_mutex_lock(&mutex_interruptFlag);
	interruptFlag = false;
	pthread_mutex_unlock(&mutex_interruptFlag);
	hay_que_desalojar = false;
	hay_que_desalojar_sin_mensaje = false;

}

uint32_t* detectar_registro(char* RX){
	if(strcmp(RX, "AX") == 0){
		return &(contexto->AX);
	}else if(strcmp(RX, "BX") == 0){
		return &(contexto->BX);
	}else if(strcmp(RX, "CX") == 0){
		return &(contexto->CX);
	}else if(strcmp(RX, "DX") == 0){
		return &(contexto->DX);
	}else{
		log_error(cpu_logger, "Registro desconocido: %s", RX);
		exit(EXIT_FAILURE);
	}
}

void atender_recepcion_de_instruccion(t_buffer* unBuffer){
	char* instruccion_actual_string = recibir_string_del_buffer(unBuffer);
	log_info(cpu_logger, "Instruccion recibida: [%s]", instruccion_actual_string);

	instruccion_split = string_split(instruccion_actual_string, " ");
	sem_post(&sem_control_fetch_decode);

//	string_array_destroy(instruccion_split);
	free(instruccion_actual_string);
}

bool validador_de_header(char* header_string){
	log_info(cpu_logger, "String a evaluar: %s", header_string);
	bool respuesta = false;
	int i = 0;
	while(opcode_strings[i] != NULL){
		if(strcmp(opcode_strings[i], header_string) == 0) respuesta = true;
		i++;
	}
	log_info(cpu_logger, "Valor del bool: %d", respuesta);
	return respuesta;
}


t_paquete* alistar_paquete_de_desalojo(op_code code_op){
	t_paquete* unPaquete = crear_super_paquete(code_op);
	cargar_int_al_super_paquete(unPaquete, contexto->proceso_pid);
	cargar_int_al_super_paquete(unPaquete, contexto->proceso_ip);
	cargar_choclo_al_super_paquete(unPaquete, &(contexto->AX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, &(contexto->BX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, &(contexto->CX), sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, &(contexto->DX), sizeof(uint32_t));

	return unPaquete;
}

void enviarPaqueteKernel(char* motivo){
	t_paquete* paqueteManejoRecursos = alistar_paquete_de_desalojo(ATENDER_INSTRUCCION_CPK);
	// luego de enviar el contexto, envio el motivo(que instruccion es la que le manda)
	cargar_string_al_super_paquete(paqueteManejoRecursos, motivo);
	enviar_paquete(paqueteManejoRecursos, fd_kernel_dispatch);
	eliminar_paquete(paqueteManejoRecursos);
}

void enviarPaqueteKernelConInfoExtra(char* motivo, t_paquete* infoExtra){
	t_paquete* paqueteInstruccionKernel = alistar_paquete_de_desalojo(ATENDER_INSTRUCCION_CPK);
	// luego de enviar el contexto, envio el motivo(que instruccion es la que le manda), y la mochila con toda la info extra
	cargar_string_al_super_paquete(paqueteInstruccionKernel, motivo);
	cargar_choclo_al_super_paquete(paqueteInstruccionKernel, infoExtra->buffer->stream, infoExtra->buffer->size);
	enviar_paquete(paqueteInstruccionKernel, fd_kernel_dispatch);
	eliminar_paquete(paqueteInstruccionKernel);
}



void print_proceso(){
	log_info(cpu_logger, "[PID: %d][PC: %d][Registros: %u|%u|%u|%u]",
			contexto->proceso_pid,
			contexto->proceso_ip,
			contexto->AX,
			contexto->BX,
			contexto->CX,
			contexto->DX);
}


/* ============================================================================= */
/* =======================ESTA PARTE ES PARA LAS PRUEBAS======================== */
/* ============================================================================= */

//static t_buffer* simular_recepcion_de_buffer_de_kernel(
//		int pid,
//		int pc,
//		uint32_t ax,
//		uint32_t bx,
//		uint32_t cx,
//		uint32_t dx
//){
//	t_paquete* un_paquete = crear_super_paquete(PRUEBAS);
//	cargar_int_al_super_paquete(un_paquete, pid);
//	cargar_int_al_super_paquete(un_paquete, pc);
//
//	cargar_choclo_al_super_paquete(un_paquete, &ax, sizeof(uint32_t));
//	cargar_choclo_al_super_paquete(un_paquete, &bx, sizeof(uint32_t));
//	cargar_choclo_al_super_paquete(un_paquete, &cx, sizeof(uint32_t));
//	cargar_choclo_al_super_paquete(un_paquete, &dx, sizeof(uint32_t));
//
//	t_buffer* un_buffer = un_paquete->buffer;
//	free(un_paquete);
//	return un_buffer;
//}
//
//static t_buffer* simular_recepcion_de_buffer_de_instruccion_desde_memoria(char* una_instruccion){
//	t_paquete* un_paquete = crear_super_paquete(PRUEBAS);
//	cargar_string_al_super_paquete(un_paquete, una_instruccion);
//
//	t_buffer* un_buffer = un_paquete->buffer;
//	free(un_paquete);
//	return un_buffer;
//}
//
//void simulador_de_eventos(){
//
////	sleep(2);
////	t_buffer* un_buffer_1 = simular_recepcion_de_buffer_de_kernel(12, 2, 12, 13, 14,15);
////	atender_proceso_del_kernel(un_buffer_1);
//
//	sleep(2);
//	t_buffer* un_buffer_2 = simular_recepcion_de_buffer_de_instruccion_desde_memoria("SET AX 100");
//	uint32_t prueba_conversion = strtoul("34", NULL, 10);
//	printf(">>>>>>> %u\n", prueba_conversion);
//	atender_recepcion_de_instruccion(un_buffer_2);
//
//}












