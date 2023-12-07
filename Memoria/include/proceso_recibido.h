#ifndef PROCESO_RECIBIDO_H_
#define PROCESO_RECIBIDO_H_

#include "m_gestor.h"
#include "swap.h"
#include "marcos.h"

void agregar_proceso_a_listado(t_buffer* unBuffer, t_list* lst_procesos_recibido);
void liberar_proceso(proceso_recibido* proceso);
void liberar_listado_procesos(t_list* lst_procesos);
proceso_recibido* obtener_proceso_por_id(int pid, t_list* lst_procesos);


t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);
void liberar_memoria_de_instrucciones(t_list* instrucciones);
char* obtener_instruccion_por_indice(int indice_instruccion, t_list* instrucciones);



void cargar_bloques_asignados_en_proceso(int pid, t_list* lista_de_marcos);

#endif /* PROCESO_RECIBIDO_H_ */
