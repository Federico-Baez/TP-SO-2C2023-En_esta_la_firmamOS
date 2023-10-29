#include <stdio.h>
#include <stdlib.h>

#include "../include/CPU.h"

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

}

void iniciar_semaforos(){
	sem_init(&sem_control_fetch_decode, 0, 0);
	sem_init(&sem_control_decode_execute, 0, 0);
	sem_init(&sem_control_peticion_marco_a_memoria, 0, 0);
}

void destruir_semaforos(){
	sem_destroy(&sem_control_fetch_decode);
	sem_destroy(&sem_control_decode_execute);
	sem_destroy(&sem_control_peticion_marco_a_memoria);
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

/*No usar esta funcion - Solo es para prueba
 * Si la usas, te vacia el buffer*/
static void _print_proceso_recibido(t_buffer* un_buffer){
	int temporal_pid = recibir_int_del_buffer(un_buffer);
	int temporal_ticket = recibir_int_del_buffer(un_buffer);
	int temporal_ip = recibir_int_del_buffer(un_buffer);
	uint32_t* t_AX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	uint32_t* t_BX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	uint32_t* t_CX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);
	uint32_t* t_DX = (uint32_t*)recibir_choclo_del_buffer(un_buffer);

	log_info(cpu_logger, "<PID:%d>[T:%d][IP:%d][%u|%u|%u|%u]",
			temporal_pid,
			temporal_ticket,
			temporal_ip,
			*t_AX,
			*t_BX,
			*t_CX,
			*t_DX);
	printf("------- <%d>\n", un_buffer->size);
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
//			atender_proceso_del_kernel(unBuffer);

			//----- Solo para pruebas - Hay que boorar esta parte---
			_print_proceso_recibido(unBuffer);
			//-----------------------------

			free(unBuffer);
			break;
		case MENSAJES_POR_CONSOLA:
			unBuffer = recibiendo_super_paquete(fd_kernel_dispatch);
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


static void _print_interrupcion(t_buffer* un_buffer){
	int temporal_pid = recibir_int_del_buffer(un_buffer);
	int temporal_ticket = recibir_int_del_buffer(un_buffer);
	char* motivo_interrupcion = recibir_string_del_buffer(un_buffer);
	log_warning(cpu_logger, "<PID:%d>[T:%d][%s]",
							temporal_pid,
							temporal_ticket,
							motivo_interrupcion);
	printf("------- <%d>\n", un_buffer->size);
}

void atender_cpu_interrupt(){
	fd_kernel_interrupt = esperar_cliente(cpu_logger, "Kernel por interrupt", server_fd_cpu_interrupt);
	gestionar_handshake_como_server(fd_kernel_interrupt, cpu_logger);
		log_info(cpu_logger, "::::::::::: KERNEL CONECTADO POR INTERRUPT ::::::::::::");
		int control_key = 1;
		while(control_key){
			int cod_op = recibir_operacion(fd_kernel_interrupt);
			t_buffer* unBuffer;
			//log_info(cpu_logger, "Se recibio algo de KERNEL");

			switch (cod_op) {
			case FORZAR_DESALOJO_KC:
				unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);

				//------Prueba--
				_print_interrupcion(unBuffer);
				//----------

				free(unBuffer);
				break;
			case MENSAJES_POR_CONSOLA:
				unBuffer = recibiendo_super_paquete(fd_kernel_interrupt);
				atender_mensajes_kernel_v2(unBuffer, "Interrupt");
				break;
			case -1:
				log_error(cpu_logger, "[DESCONEXION]: KERNEL_Interrupt");
				control_key = 0;
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
			//
			break;
		case PETICION_DE_INSTRUCCIONES_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			atender_recepcion_de_instruccion(unBuffer);
			break;
		case PETICION_DE_EJECUCION_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
			//
			break;
		case CONSULTA_DE_PAGINA_CM:
			unBuffer = recibiendo_super_paquete(fd_memoria);
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

	//Inicicar ciclo de instruccion
	iniciar_ciclo_de_instruccion();

	//Desalojar contexto de CPU / decidir el motivo de desalojo
	//desalojar_proceso(); //[FALTA]Decidir como sumarle el motivo de desalojo

	destruir_estructuras_del_contexto_acttual();
}



void iniciar_ciclo_de_instruccion(){

	bool key_for_control = true;
	while(key_for_control){
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

		//Pregunta si hay interrupciones

	}
}

void ciclo_de_instruccion_fetch(){
	t_paquete* un_paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);
	cargar_int_al_super_paquete(un_paquete, *proceso_pid);
	cargar_int_al_super_paquete(un_paquete, *proceso_ip);
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
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		*proceso_ip = *proceso_ip + 1;
		uint32_t* registro_referido = detectar_registro(instruccion_split[1]);
		*registro_referido = strtoul(instruccion_split[1], NULL, 10);

	}else if(strcmp(instruccion_split[0], "SUM") == 0){//[SUM][destino:AX][origen:BX]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		*proceso_ip = *proceso_ip + 1;
		uint32_t* registro_referido_destino = detectar_registro(instruccion_split[1]);
		uint32_t* registro_referido_origen = detectar_registro(instruccion_split[2]);
		*registro_referido_destino += *registro_referido_origen;

	}else if(strcmp(instruccion_split[0], "SUB") == 0){//[SUB][destino:AX][origen:BX]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		*proceso_ip = *proceso_ip + 1;
		uint32_t* registro_referido_destino = detectar_registro(instruccion_split[1]);
		uint32_t* registro_referido_origen = detectar_registro(instruccion_split[2]);
		*registro_referido_destino -= *registro_referido_origen;

	}else if(strcmp(instruccion_split[0], "JNZ") == 0){// [JNZ][Registro][Instruccion]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		*proceso_ip = *proceso_ip + 1;
		uint32_t* registro_referido = detectar_registro(instruccion_split[1]);
		if(*registro_referido != 0) *proceso_ip = atoi(instruccion_split[2]);
		else{
			//[FALTA] Que hacer ?????
		}

	}else if(strcmp(instruccion_split[0], "SLEEP") == 0){// [SLEEP][tiempo]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		 /* Esta instrucción representa una syscall bloqueante.
		 * Se deberá devolver el Contexto de Ejecución actualizado al Kernel
		 * junto a la cantidad de segundos que va a bloquearse el proceso.*/
		//Enviar al KERNEL: [PID][IP][AX][BX][CX][DX]["SLEEP"]
		*proceso_ip = *proceso_ip + 1;
		t_paquete* unPaquete = alistar_paquete_de_desalojo(EJECUTAR_PROCESO_KC);
		cargar_string_al_super_paquete(unPaquete, "SLEEP"); //Motivo del desalojo
		cargar_int_al_super_paquete(unPaquete, atoi(instruccion_split[1])); //ALgun otro perametro necesario
		enviar_paquete(unPaquete, fd_kernel_dispatch);
		eliminar_paquete(unPaquete);

	}else if(strcmp(instruccion_split[0], "WAIT") == 0){// [WAIT][char* Recurso]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		/*Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.*/
		*proceso_ip = *proceso_ip + 1;
		t_paquete* unPaquete = alistar_paquete_de_desalojo(EJECUTAR_PROCESO_KC);
		cargar_string_al_super_paquete(unPaquete, "WAIT"); //Motivo del desalojo
		cargar_string_al_super_paquete(unPaquete, instruccion_split[1]); //ALgun otro perametro necesario
		enviar_paquete(unPaquete, fd_kernel_dispatch);
		eliminar_paquete(unPaquete);

	}else if(strcmp(instruccion_split[0], "SIGNAL") == 0){// [SIGNAL][char* Recurso]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		/*Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.*/
		*proceso_ip = *proceso_ip + 1;
		t_paquete* unPaquete = alistar_paquete_de_desalojo(EJECUTAR_PROCESO_KC);
		cargar_string_al_super_paquete(unPaquete, "SIGNAL"); //Motivo del desalojo
		cargar_string_al_super_paquete(unPaquete, instruccion_split[1]); //ALgun otro perametro necesario
		enviar_paquete(unPaquete, fd_kernel_dispatch);
		eliminar_paquete(unPaquete);

	}else if(strcmp(instruccion_split[0], "MOV_IN") == 0){// [MOV_IN][RX][Dir_logica]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		/*Lee el valor de memoria correspondiente a la Dirección Lógica y lo almacena en el Registro.*/

		/*
		 * 1.Deberia llamar a la MMU(La MMU hace una peticion para recibir el nuemero de marco)
		 * para que traduzca la direccion logica a dir. fisica.
		 * 2. Una vez obtenida la direccion fisica, pedirle a memoria la info relevante correspondiente
		 * a la direccion fisica.
		 * 3. Aca hay que evaluar si retorna PAGEFAULT.
		 * 4. Asumiendo que no haya PAGEFAULT, memoria devolveria el dato requerido y se almacenaria
		 * en el registro respectivo.
		*/

		//No DESCOMENTAR HASTA QUE ESTA TERMINADO LO QUE FALTA:
//		int direccion_fisica = MMU(atoi(instruccion_split[2]));
//		//[FALTA] Pedir valor a memoria correspondiente a la direccion fisica
//		//Verificar que no sea PAGEFAULT
//		uint32_t* registro_referido = detectar_registro(instruccion_split[1]);
//		//Guardar el valor de memoria en el registro;

	}else if(strcmp(instruccion_split[0], "MOV_OUT") == 0){// [MOV_OUT][Dir_logica][RX]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		/*Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida
		 * a partir de la Dirección Lógica.*/

		/*
		 * 1. Deberia llamar a la MMU para que me devuelva la dir fisica
		 * 2. Pedirle a memoria que alamcene el valor del Registro RX en la direccion fisica que nos dio la MMU
		 * 3. Verificar que no sea PAGEFAULT
		 * 4. Si no es PAGEFAULT, proceder pedirle a memoria que guarde el valor del registro RX,
		 * en la direccion fisica obtenida de la MMU.
		 * */


	}else if(strcmp(instruccion_split[0], "F_OPEN") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "F_CLOSE") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "F_SEEK") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "F_READ") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "F_WRITE") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "F_TRUNCATE") == 0){
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
	}else if(strcmp(instruccion_split[0], "EXIT") == 0){// [EXIT]
		log_info(cpu_logger, "Ejecutando: <%s>", instruccion_split[0]);
		t_paquete* unPaquete = alistar_paquete_de_desalojo(EJECUTAR_PROCESO_KC);
		cargar_string_al_super_paquete(unPaquete, "EXIT"); //Motivo del desalojo
		enviar_paquete(unPaquete, fd_kernel_dispatch);
		eliminar_paquete(unPaquete);

	}else{
		log_error(cpu_logger, "Nunca se deberia llegar aqui :(");
	}
}

//int MMU(int dir_logica, int* dir_fisica){ //[FALTA] Consolidar ideas
//	bool respuesta = true;
//	int tamanio_pagina; //[FALTA] De donde obtengo este dato?
//	int desplazamiento;//Sera este la Direccion Fisica??
//	int numero_pagina; //[FALTA] Aparentemente se lo tiene que pedir a MEMORIA
//
//	//Pidiendo numero de pagina a MEMORIA
//	t_paquete* unPaquete = crear_super_paquete(CONSULTA_DE_PAGINA_CM);
//	cargar_int_al_super_paquete(unPaquete, proceso_pid);
//	cargar_int_al_super_paquete(unPaquete, dir_logica);
//
//
//	sem_wait(&sem_control_peticion_marco_a_memoria);
//
//	numero_pagina = floor(dir_logica / tamanio_pagina);
//	desplazamiento = dir_logica - numero_pagina * tamanio_pagina;
//
//	*dir_fisica = desplazamiento; //Asumiendo que esto sea la direccion fisica
//
//	return respuesta;
//}

void iniciar_estructuras_para_atender_al_proceso(t_buffer*  unBuffer){
	proceso_pid = malloc(sizeof(int));
	proceso_ip = malloc(sizeof(int));

	*proceso_pid = recibir_int_del_buffer(unBuffer);
	*proceso_ip = recibir_int_del_buffer(unBuffer);
	AX = (uint32_t*)recibir_choclo_del_buffer(unBuffer);
	BX = (uint32_t*)recibir_choclo_del_buffer(unBuffer);
	CX = (uint32_t*)recibir_choclo_del_buffer(unBuffer);
	DX = (uint32_t*)recibir_choclo_del_buffer(unBuffer);
}

void destruir_estructuras_del_contexto_acttual(){
	free(proceso_pid);
	free(proceso_ip);
	free(AX);
	free(BX);
	free(CX);
	free(DX);

	string_array_destroy(instruccion_split);
}

uint32_t* detectar_registro(char* RX){
	if(strcmp(RX, "AX") == 0){
		return AX;
	}else if(strcmp(RX, "BX") == 0){
		return BX;
	}else if(strcmp(RX, "CX") == 0){
		return CX;
	}else if(strcmp(RX, "DX") == 0){
		return DX;
	}else{
		log_error(cpu_logger, "Registro desconocido: %s", RX);
		exit(EXIT_FAILURE);
	}
}

void atender_recepcion_de_instruccion(t_buffer* unBuffer){
	char* instruccion_actual_string = recibir_string_del_buffer(unBuffer);
	free(unBuffer);
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
//	cargar_int_al_super_paquete(unPaquete, *proceso_pid);
	cargar_int_al_super_paquete(unPaquete, *proceso_ip);
	cargar_choclo_al_super_paquete(unPaquete, AX, sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, BX, sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, CX, sizeof(uint32_t));
	cargar_choclo_al_super_paquete(unPaquete, DX, sizeof(uint32_t));

	return unPaquete;
}



void print_proceso(){
	log_info(cpu_logger, "[PID: %d][PC: %d][Registros: %u|%u|%u|%u]",
			*proceso_pid,
			*proceso_ip,
			*AX,
			*BX,
			*CX,
			*DX);
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












