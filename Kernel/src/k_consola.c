#include "../include/k_consola.h"

void leer_consola(){

	t_list* list_instructions = list_create();

	add_instruction_list(list_instructions, "INICIAR_PROCESO", INICIAR_PROCESO, 3);
	add_instruction_list(list_instructions, "FINALIZAR_PROCESO", FINALIZAR_PROCESO, 1);
	add_instruction_list(list_instructions, "DETENER_PLANIFICACION", DETENER_PLANIFICACION, 0);
	add_instruction_list(list_instructions, "INICIAR_PLANIFICACION", INICIAR_PLANIFICACION, 0);
	add_instruction_list(list_instructions, "MULTIPROGRAMACION", MULTIPROGRAMACION, 1);
	add_instruction_list(list_instructions, "PROCESO_ESTADO", PROCESO_ESTADO, 0);

	//list_iterate(list_instructions, (void*)imprimir_instrucciones);

	//validaciones - El comando y la cantidad de parametros por consola sea el correcto
	char* leido;
	leido = readline("> ");
	while(strcmp(leido,"\0") != 0){
//		printf("Mensaje: %s \n", leido);

		if(validar_instruccion(leido, list_instructions)){
			printf("Comando válido\n");
		}



//		t_paquete* paquete = crear_super_paquete(MENSAJES_POR_CONSOLA);
//		cargar_int_al_super_paquete(paquete, strlen(leido) + 1);
//		cargar_string_al_super_paquete(paquete, leido);
//		enviar_paquete(paquete, fd_memoria);
//		enviar_paquete(paquete, fd_filesystem);
//		enviar_paquete(paquete, fd_cpu_dispatcher);
//		enviar_paquete(paquete, fd_cpu_interrupt);
//		eliminar_paquete(paquete);

		free(leido);
		leido = readline("> ");
	}

	list_destroy_and_destroy_elements(list_instructions,(void *)destruir_elementos);
	free(leido);
}

void destruir_elementos(t_instruction* instruction){
	free(instruction->instruction_name);
	free(instruction);
}

void add_instruction_list(t_list* lista, char* instruccion, t_op_instruction op_instruction_code, int n_param){
	t_instruction* instruction = malloc(sizeof(t_instruction));
	instruction->instruction_name = malloc(sizeof(char)*(strlen(instruccion) + 1));
	memcpy(instruction->instruction_name, instruccion, sizeof(char)*(strlen(instruccion) + 1));
	//instruction->instruction_name = instruccion;
	instruction->op_instruction = op_instruction_code;
	instruction->instruction_n_param = n_param;

	list_add(lista, instruction);
}

void imprimir_instrucciones(t_instruction* instruction){
	printf("[NAME: %s][OP_CIDE: %d][N° PRAM: %d]\n", instruction->instruction_name, instruction->op_instruction, instruction->instruction_n_param);
}

bool validar_instruccion(char* leido, t_list* list_instructions){
	char** comando_consola = string_split(leido, " ");
	int cantidad_de_parametros = string_array_size(comando_consola) - 1;
	int elemento_encontrado = 0;
	bool resultado = false;

	t_list_iterator* iterador_carousel = list_iterator_create(list_instructions);
	while(list_iterator_has_next(iterador_carousel)){
		t_instruction* instruction = list_iterator_next(iterador_carousel);
		if(strcmp(instruction->instruction_name, comando_consola[0]) == 0){
			elemento_encontrado++;
			if(instruction->instruction_n_param == cantidad_de_parametros){
				elemento_encontrado++;
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
		printf("[ERROR]: Cantidad de parametros incorrectos\n");
		break;
	case 0:
		printf("[ERROR]: Comando NO RECONOCIDO\n");
		break;
	default:
		printf("[ERROR]: Algo salio mal en el reconocimiento de Comandos\n");
		break;
	}

	list_iterator_destroy(iterador_carousel);
	string_array_destroy(comando_consola);

	return resultado;
}


