#ifndef K_SERVICIOS_KERNEL_H_
#define K_SERVICIOS_KERNEL_H_

#include "k_gestor.h"




void pausador();
void cambiar_estado(t_pcb* una_pcb, est_pcb nex_state);
int generar_ticket();

#endif /* K_SERVICIOS_KERNEL_H_ */
