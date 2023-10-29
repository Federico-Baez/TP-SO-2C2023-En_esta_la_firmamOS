#ifndef PCB_H_
#define PCB_H_

#include "k_gestor.h"



t_pcb* crear_pcb(char* path, char* size, char* prioridad);
void destruir_pcb();
void imprimir_pcb(t_pcb* una_PCB);
void imprimir_pcb_v2(t_pcb* una_pcb);
t_pcb* buscar_pcb_por_pid(int un_pid);
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* una_pcb);
void liberar_todos_los_recursos_de_una_pcb(t_pcb* una_pcb);
void avisar_a_memoria_para_liberar_estructuras(t_pcb* una_pcb);

#endif /* PCB_H_ */
