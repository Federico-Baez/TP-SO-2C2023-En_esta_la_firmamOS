#ifndef ATENDER_CONEXIONES_H_
#define ATENDER_CONEXIONES_H_

#include "k_gestor.h"
#include "pcb.h"
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"

void iniciar_conexiones();
void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();



#endif /* ATENDER_CONEXIONES_H_ */
