#ifndef FS_MEMORIA_H_
#define FS_MEMORIA_H_

#include "m_gestor.h"
#include "fcb.h"
#include "tabla_fat.h"
#include "bloques.h"

// =========== ATENDER MEMORIA ===============
void atender_asignacion_de_bloques_por_creacion_de_proceso(t_buffer* un_buffer);
void atender_peticion_de_libarar_bloques_swap(t_buffer* un_buffer);
void atender_peticion_de_guardar_marco_en_swap(t_buffer* un_buffer);
void atender_pedido_de_lectura_de_pag_swap(t_buffer* un_buffer);
void atender_recepcion_de_marco_bloque_de_memoria_por_f_write_de_kernel(t_buffer* un_buffer);
void atender_rpta_de_memoria_a_fs_por_lectura_de_marco_por_dir_fisica(t_buffer* un_buffer);


// =========== ENVIAR A MEMORIA ===============

void  enviar_lista_bloques_a_memoria(int pid, t_list* lista_bloques);
void enviar_bloque_pagina_a_memoria(int pid, int nro_pagina, void* un_bloque);


// =========== ENVIAR A KERNEL ============================

void enviar_rpta_a_kernel_del_f_write(int pid);
void enviar_confirmacion_de_lectura_a_kernel(int pid);

#endif /* FS_MEMORIA_H_ */
