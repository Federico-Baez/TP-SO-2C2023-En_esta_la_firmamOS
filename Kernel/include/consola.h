#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <readline/readline.h>
#include "k_gestor.h"
#include "pcb.h"
#include "planificador_largo_plazo.h"
#include "k_servicios_kernel.h"

typedef enum{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO,
	HELP,
	PRINT
}t_op_instruction;

typedef struct{
	char* instruction_name;
	t_op_instruction op_instruction;
	int instruction_n_param;
}t_instruction;

void leer_consola(void);


#endif /* CONSOLA_H_ */
