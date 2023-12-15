#include "../include/consola.h"

static void _destruir_elementos(t_instruction* instruction){
	free(instruction->instruction_name);
	free(instruction);
}

static void add_instruction_list(t_list* lista, char* instruccion, t_op_instruction op_instruction_code, int n_param){
	t_instruction* instruction = malloc(sizeof(t_instruction));
	instruction->instruction_name = malloc(sizeof(char)*(strlen(instruccion) + 1));
	memcpy(instruction->instruction_name, instruccion, sizeof(char)*(strlen(instruccion) + 1));
//	instruction->instruction_name = instruccion;
	instruction->op_instruction = op_instruction_code;
	instruction->instruction_n_param = n_param;

	list_add(lista, instruction);
}

//static void imprimir_instrucciones(t_instruction* instruction){
//	printf("[NAME: %s][OP_CIDE: %d][N° PRAM: %d]\n", instruction->instruction_name, instruction->op_instruction, instruction->instruction_n_param);
//}
static void _inicializar_consola(){

	//CARGAR INSTRUCCIONES VALIDAS EN UNA LISTA
	lista_instructions = list_create();
	add_instruction_list(lista_instructions, "INICIAR_PROCESO", INICIAR_PROCESO, 3);
	add_instruction_list(lista_instructions, "FINALIZAR_PROCESO", FINALIZAR_PROCESO, 1);
	add_instruction_list(lista_instructions, "DETENER_PLANIFICACION", DETENER_PLANIFICACION, 0);
	add_instruction_list(lista_instructions, "INICIAR_PLANIFICACION", INICIAR_PLANIFICACION, 0);
	add_instruction_list(lista_instructions, "MULTIPROGRAMACION", MULTIPROGRAMACION, 1);
	add_instruction_list(lista_instructions, "PROCESO_ESTADO", PROCESO_ESTADO, 0);
	add_instruction_list(lista_instructions, "HELP", HELP, 0);
	add_instruction_list(lista_instructions, "PRINT", PRINT, 1);
}

static bool validar_instruccion(char* leido){
	char** comando_consola = string_split(leido, " ");
	int cantidad_de_parametros = string_array_size(comando_consola) - 1;
	int elemento_encontrado = 0;
	bool resultado = false;

	t_list_iterator* iterador_carousel = list_iterator_create(lista_instructions);
	while(list_iterator_has_next(iterador_carousel)){
		t_instruction* instruction = list_iterator_next(iterador_carousel);
		if(strcmp(instruction->instruction_name, comando_consola[0]) == 0){
			elemento_encontrado = 1;
			if(instruction->instruction_n_param == cantidad_de_parametros){
				elemento_encontrado = 2;

				//[FALTA] VALIDAR LA AUTENTICIDAD DE CADA PARAMETRO
				break;
			}
		}
	}

	switch (elemento_encontrado) {
	case 2:
//		printf("Comando válido\n");
		printf("> ");
		resultado = true;
		break;
	case 1:
//		printf("[ERROR]: Cantidad de parametros incorrectos\n");
		log_warning(kernel_logger, "N°  Param. Incorrecto");
		break;
	case 0:
//		printf("[ERROR]: Comando NO RECONOCIDO\n");
		log_error(kernel_logger, "Comando no reconocido");
		break;
	default:
//		printf("[ERROR]: Algo salio mal en el reconocimiento de Comandos\n");
		log_error(kernel_logger, "Algo salio mal en el reconocimiento de Comandos");
		break;
	}

	list_iterator_destroy(iterador_carousel);
	string_array_destroy(comando_consola);

	return resultado;
}
/*
static void _finalizar_proceso_por_PID(char* un_pid){
	t_pcb* un_pcb = buscar_pcb_por_pid(atoi(un_pid));
	if(un_pcb != NULL){
		pthread_mutex_lock(&mutex_lista_exec);
		if(esta_pcb_en_una_lista_especifica(lista_execute, un_pcb)){
			pthread_mutex_lock(&mutex_flag_finalizar_proceso);
			flag_finalizar_proceso = true;
			pthread_mutex_unlock(&mutex_flag_finalizar_proceso);
		}
		pthread_mutex_unlock(&mutex_lista_exec);
		plp_planificar_proceso_exit(un_pcb);
	}else{
		log_error(kernel_logger, "CONSOLA - No se encontro el PID en ningun lado");
		exit(EXIT_FAILURE);
	}
	//Se debe liberar essta parte o generar memory leaks
	free(un_pid);
}
*/

static void _finalizar_proceso_por_PID(char* un_pid){
	int pid = atoi(un_pid);
	plp_planificar_proceso_exit(pid);
//	ejecutar_en_un_hilo_nuevo_detach(plp_planificar_proceso_exit, pid);
	free(un_pid);
}

static void _pausar_planificadores(){
	if(var_pausa == 1){
		log_info(kernel_logger, "Los planificadores ya se encuentran pausados");
	}else{
		var_pausa = 1;
		pausador();
	}
}

static void _iniciar_planificadores(){
	if(var_pausa == 0){
//		El enunciado dice que se debe ignorar el mensaje en caso
//		de q' los planificadores no se encuentren pausados
//		log_info(kernel_logger, "");
	}else{
		var_pausa = 0;
		log_warning(kernel_log_obligatorio, "INICIO DE PLANIFICACIÓN");  // --> Tiene que ser log_info, por ahora lo dejamos asi para que se note
		sem_post(&sem_pausa);
	}
}

static void _cambiar_grado_de_multiprogramacion(char* un_valor){
	int nuevo_valor = atoi(un_valor);
	int valor_anterior = GRADO_MULTIPROGRAMACION_INI;
	int diferencia;

	if(nuevo_valor >= 1){
		GRADO_MULTIPROGRAMACION_INI = nuevo_valor;
		log_info(kernel_log_obligatorio, "Grado Anterior: %d - Grado Actual: %d" ,valor_anterior ,nuevo_valor);
		if(nuevo_valor > valor_anterior){
			diferencia = nuevo_valor - valor_anterior;
		}else{
			diferencia = valor_anterior - nuevo_valor;
		}
		if(diferencia > 0){
			while(diferencia > 0){
				plp_planificar_proceso_nuevo(NULL);
				diferencia--;
			}
		}
	}else{
		log_error(kernel_logger, "EL grado de multiprogramacion tiene que ser >= 1");
	}

	free(un_valor);
}


static void _imprimir_comandos_permitidos(){
	printf("============= COMANDOS AUTORIZADOS ============\n");
	printf("INICIAR_PROCESO [PATH] [SIZE] [PRIORIDAD]\n");
	printf("FINALIZAR_PROCESO [PID]\n");
	printf("DETENER_PLANIFICACION\n");
	printf("INICIAR_PLANIFICACION\n");
	printf("MULTIPROGRAMACION [VALOR]\n");
	printf("PROCESO_ESTADO\n");
	printf("===============================================\n");
}

static void atender_instruccion_validada(char* leido){
	char** comando_consola = string_split(leido, " ");

	//Obtener el componente de la instruccion respectiva
	bool __buscar_instruccion(t_instruction* instruction){
		if(strcmp(instruction->instruction_name, comando_consola[0]) == 0){
			return true;
		}
		return false;
	}
	t_instruction* instruction = list_find(lista_instructions, (void*)__buscar_instruccion);

	//Evaluar en el SWITCH CASE
	switch (instruction->op_instruction) {
		case INICIAR_PROCESO: //[PATH][SIZE][PRIORIDAD]
			t_pcb* un_pcb = crear_pcb(comando_consola[1], comando_consola[2], comando_consola[3]);
			//[FALTA]Verificar que la URL exista
//			imprimir_pcb(un_pcb);
			ejecutar_en_un_hilo_nuevo_detach((void*)plp_planificar_proceso_nuevo, un_pcb);
			break;
		case FINALIZAR_PROCESO: //[PID]
			char* copia_del_pid = string_duplicate(comando_consola[1]);
			ejecutar_en_un_hilo_nuevo_detach((void*)_finalizar_proceso_por_PID, copia_del_pid);
			break;
		case DETENER_PLANIFICACION: //_none
			ejecutar_en_un_hilo_nuevo_detach((void*)_pausar_planificadores, NULL);
			break;
		case INICIAR_PLANIFICACION: //_none
			ejecutar_en_un_hilo_nuevo_detach((void*)_iniciar_planificadores, NULL);
			break;
		case MULTIPROGRAMACION: //[int]
			char* copia_del_GMMP = string_duplicate(comando_consola[1]);
			ejecutar_en_un_hilo_nuevo_detach((void*)_cambiar_grado_de_multiprogramacion, copia_del_GMMP);
			break;
		case PROCESO_ESTADO: //_none
			public_imprimir_procesos_por_estado_v1();
			break;
		case HELP: //_none
			_imprimir_comandos_permitidos();
			break;
		case PRINT: //[PID]
			t_pcb* una_pcb_a_buscar = buscar_pcb_por_pid(atoi(comando_consola[1]));
			if(una_pcb_a_buscar != NULL)imprimir_pcb_v2(una_pcb_a_buscar);
			break;
		default:
			break;
	}


	string_array_destroy(comando_consola);
}


static void _leer_comandos(){
	//validaciones - El comando y la cantidad de parametros por consola sea el correcto
	char* leido;
	leido = readline("> ");
	while(strcmp(leido,"\0") != 0){
		if(validar_instruccion(leido)){
			printf("Comando válido\n");
			atender_instruccion_validada(leido);
		}

		free(leido);
		leido = readline("> ");
	}
	free(leido);
}

static void _finalizar_consola(){
	list_destroy_and_destroy_elements(lista_instructions,(void *)_destruir_elementos);
}

void leer_consola(){

	_inicializar_consola();

	pthread_create(&hilo_consola, NULL, (void*)_leer_comandos, NULL);
	pthread_join(hilo_consola, NULL);

	_finalizar_consola();
}
