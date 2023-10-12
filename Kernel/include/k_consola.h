#ifndef K_CONSOLA_H_
#define K_CONSOLA_H_

#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>


typedef enum{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO
}t_op_instruction;

typedef struct{
	char* instruction_name;
	t_op_instruction op_instruction;
	int instruction_n_param;
}t_instruction;





void leer_consola(void);
void add_instruction_list(t_list* lista, char* instruccion, t_op_instruction op_instruction_code, int n_param);
void imprimir_instrucciones(t_instruction* instruction);
void destruir_elementos(t_instruction* instruction);
bool validar_instruccion(char* leido, t_list* list_instructions);





#endif /* K_CONSOLA_H_ */
