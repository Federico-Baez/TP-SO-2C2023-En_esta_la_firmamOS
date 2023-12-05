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

// ----- Funciones de manejo de INSTRUCCIONES del CPU ------
// ----- SLEEP -----
void atender_sleep(t_pcb* pcb, int seconds_blocked, char* motivo_block);
void manejar_tiempo_sleep(t_sleep* pcb_sleep);
// ----- WAIT  -----
void atender_wait(t_pcb* pcb,char* recurso_solicitado);
// ----- SIGNAL -----
void atender_signal(t_pcb* pcb,char* recurso_a_liberar);



t_recurso* buscar_recurso(char* recurso_solicitado);
void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recruso, pthread_mutex_t mutex_recruso);


#endif /* ATENDER_CONEXIONES_H_ */
