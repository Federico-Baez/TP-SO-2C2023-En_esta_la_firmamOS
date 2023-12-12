#ifndef MANEJO_DEADBLOCKS_H_
#define MANEJO_DEADBLOCKS_H_

#include "k_gestor.h"
#include "pcb.h"
#include "k_servicios_kernel.h"



void deteccion_deadlock();
void obtener_lista_pcbs_block_recursos(t_list** lista_posibles_deadlocks_recurso );
t_list* obtener_lista_pcbs_block_archivos();
void logear_proceso_en_deadlock(t_pcb* pcb);


#endif /* MANEJO_DEADBLOCKS_H_ */
