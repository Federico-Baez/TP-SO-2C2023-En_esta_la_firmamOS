#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include "k_gestor.h"
#include "pcb.h"
#include "k_servicios_kernel.h"
#include "planificador_corto_plazo.h"

void plp_planificar_proceso_nuevo(t_pcb* un_pcb);
//void plp_planificar_proceso_exit(t_pcb* un_pcb);
void plp_planifica();
t_pcb* buscar_y_remover_pcb_por_pid(int un_pid);
void plp_planificar_proceso_exit(int pid);

#endif /* PLANIFICADOR_LARGO_PLAZO_H_ */
