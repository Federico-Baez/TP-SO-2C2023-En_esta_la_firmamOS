#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include "k_gestor.h"
#include "pcb.h"
#include "k_servicios_kernel.h"
#include "planificador_corto_plazo.h"

void plp_planificar_proceso_nuevo(t_pcb* una_pcb);
void plp_planificar_proceso_exit(t_pcb* una_pcb);


#endif /* PLANIFICADOR_LARGO_PLAZO_H_ */
