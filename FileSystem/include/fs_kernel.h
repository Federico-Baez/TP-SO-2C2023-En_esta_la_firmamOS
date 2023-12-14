#ifndef FS_KERNEL_H_
#define FS_KERNEL_H_

#include "m_gestor.h"
#include "fcb.h"
#include "tabla_fat.h"
#include "bloques.h"


// =========== ATENDER KERNEL ===============
void atender_f_open_de_kernel(t_buffer* un_buffer);
void atender_f_truncate_de_kernel(t_buffer* un_buffer);
void atender_f_read_de_kernel(t_buffer* un_buffer);
void atender_f_write_de_kernel(t_buffer* un_buffer);



// =========== ENVIAR A KERNEL ===============

void enviar_rta_f_open_a_kernel(t_paquete* un_paquete);
void enviar_rta_f_truncate_a_kernel(int pid_process);

// =========== ENVIAR A MEMORIA ===============
void enviar_contenido_a_memoria(int pid_process ,int dir_fisica, void* contenido_leido);
void enviar_solicitud_de_escritura_a_memoria(int pid_process, int dir_fisica, uint32_t bloque_a_escribir_fat);


#endif /* FS_KERNEL_H_ */
