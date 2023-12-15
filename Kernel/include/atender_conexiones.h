#ifndef ATENDER_CONEXIONES_H_
#define ATENDER_CONEXIONES_H_

#include "k_gestor.h"
#include "pcb.h"
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"
#include "atender_instrucciones_CPU.h"


void iniciar_conexiones();
void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();

void _gestionar_peticiones_de_memoria();
void _gestionar_peticiones_de_cpu_dispatch();
void _gestionar_interrupt();
void _gestionar_peticiones_de_filesystem();

// ----- Recibir preoceso de CPU -----
t_pcb* _recibir_proceso_desalojado(t_buffer* un_buffer);
void _atender_motivo_desalojo(char* motivo_desalojo, t_buffer* un_buffer, t_pcb* un_pcb);
void _desalojar_proceso(t_pcb* un_pcb);
void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb);
t_buffer* recibir_mochila_del_buffer(t_buffer* buffer);

// ----- PAGE FAULT -----
void _atender_page_fault(t_page_fault* un_page_fault);

// ----- RESPUESAS DE MEMORIA -----
void recibir_confirmacion_de_memoria(t_buffer* unBuffer);

void log_blocked_proceso(int pid_process, char* motivo_block);



#endif /* ATENDER_CONEXIONES_H_ */
