#ifndef PROCESO_H_
#define PROCESO_H_

#include "m_gestor.h"
#include "marcos.h"
#include "servicios_memoria.h"
#include "atender_fs.h"
#include "atender_cpu.h"
#include "atender_kernel.h"

t_proceso* crear_proceso(int pid, int size, char* path_instruc);
void eliminar_proceso(t_proceso* un_proceso);
void eliminar_lista_de_instrucciones(t_list* lista_instrucciones);
void eliminar_tabla_de_paginas(t_proceso* un_proceso);
void crear_paginas_y_asignar_marcos_para_un_proceso_nuevo(t_proceso* un_proceso);
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);
void agregar_proceso_a_listado(t_proceso* un_proceso ,t_list* lst_procesos_recibido);
void sacar_proceso_del_listado(t_proceso* un_proceso ,t_list* lst_procesos_recibido);
t_proceso* obtener_proceso_por_id(int pid);
char* obtener_instruccion_por_indice(t_proceso* un_proceso, int indice_instruccion);
void cargar_bloques_asignados_en_proceso(t_proceso* un_proceso, t_list* lista_de_pos_swap);

t_pagina* pag_obtener_pagina_completa(t_proceso* un_proceso, int nro_pagina);
int pag_obtener_nro_de_marco(t_proceso* un_proceso, int nro_pagina);
int pag_obtener_pos_en_swap(t_proceso* un_proceso, int nro_pagina);
bool pag_obtener_bit_de_PRESENCIA(t_proceso* un_proceso, int nro_pagina);
bool pag_obtener_bit_de_MODIFICADO(t_proceso* un_proceso, int nro_pagina);


#endif /* PROCESO_H_ */
