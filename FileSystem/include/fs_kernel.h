#ifndef FS_KERNEL_H_
#define FS_KERNEL_H_

#include "m_gestor.h"


// =========== ATENDER KERNEL ===============
void atender_f_open_de_kernel(t_buffer* un_buffer);
void atender_f_truncate_de_kernel(t_buffer* un_buffer);
void atender_f_read_de_kernel(t_buffer* un_buffer);
void atender_f_write_de_kernel(t_buffer* un_buffer);



// =========== ENVIAR A KERNEL ===============

void enviar_rta_f_open_a_kernel(t_paquete* un_paquete);
// Esta respuesta es para TRUNCATE - READ - WRITE
void enviar_rta_f_generica_a_kernel(int pid_process, char* mensaje_respuesta);

// =========== ENVIAR A MEMORIA ===============
void enviar_contenido_a_memoria(int dir_fisica, void* bloque_a_leer);
void enviar_solicitud_de_escritura_a_memoria(int pid_process, int dir_fisica, uint32_t bloque_a_escribir_fat);


#endif /* FS_KERNEL_H_ */
