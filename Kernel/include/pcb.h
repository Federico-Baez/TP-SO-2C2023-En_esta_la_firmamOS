#ifndef PCB_H_
#define PCB_H_

#include "k_gestor.h"
#include "k_servicios_kernel.h"


char* estado_to_string(int valor_estado);
t_pcb* crear_pcb(char* path, char* size, char* prioridad);
void destruir_pcb();
void cambiar_estado(t_pcb* una_pcb, est_pcb nex_state);
void imprimir_pcb(t_pcb* una_PCB);
void imprimir_pcb_v2(t_pcb* una_pcb);
t_pcb* buscar_pcb_por_pid(int un_pid);
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* una_pcb);
void liberar_todos_los_recursos_de_un_pcb(t_pcb* una_pcb);
void avisar_a_memoria_para_liberar_estructuras(t_pcb* una_pcb);
void transferir_from_actual_to_siguiente(t_pcb* un_pcb, t_list* lista_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente);
void agregar_pcb_lista(t_pcb* pcb, t_list* lista_estado, pthread_mutex_t mutex_lista);
char* lista_pids_en_estado(t_list* lista_estado, pthread_mutex_t mutex_lista);
t_pcb* recibir_contexto_de_ejecucion(t_buffer* un_buffer);
void liberar_recursos_pcb(t_pcb* pcb);
void asignar_recurso_liberado_pcb(t_recurso* recurso);


#endif /* PCB_H_ */
