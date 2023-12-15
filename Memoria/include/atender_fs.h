#ifndef ATENDER_FS_H_
#define ATENDER_FS_H_

#include "m_gestor.h"
#include "proceso.h"
#include "marcos.h"
#include "espacio_usuario.h"
#include "servicios_memoria.h"
#include "atender_kernel.h"
#include "atender_cpu.h"


//============RECIBIDOS DE FAILESYSTEM=======================

void asignar_posicions_de_SWAP_a_tabla_de_paginas_de_un_proceso(t_buffer* un_buffer);
void atender_lectura_de_pagina_de_swap_a_memoria(t_buffer* un_buffer);
void atender_bloque_de_memoria_y_llevarlos_a_fylesystem(t_buffer* un_buffer);
void atender_bloque_de_fs_a_memoria(t_buffer* un_buffer);


//============ENVIOS A FILEsYSTEM=======================

void enviar_a_fs_peticion_de_asignacion_de_bloques(int pid, int cantidad_de_paginas);
void enviar_a_fs_orden_de_liberacion_de_posiciones_swap(t_proceso* un_proceso);
void evniar_pagina_a_fs_area_swap(int pos_swap, void* coso_marco);
void pedir_lectura_de_pag_swap_a_fs(int pid, int nro_pagina, int pos_en_swap);
void enviar_marco_a_fs(int pid, int nro_bloque, void* un_marco, char* nombre_archivo);
void enviar_rpta_por_pedido_de_escritura_en_memoria(int pid);


#endif /* ATENDER_FS_H_ */
