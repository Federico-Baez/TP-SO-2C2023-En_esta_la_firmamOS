#ifndef FS_KERNEL_H_
#define FS_KERNEL_H_

#include "m_gestor.h"


// =========== ATENDER KERNEL ===============
void atender_f_open_de_kernel(t_buffer* un_buffer);
void atender_f_truncate_de_kernel(t_buffer* un_buffer);



// =========== ENVIAR A KERNEL ===============

void enviar_rpta_f_open_a_kernel(t_paquete* un_paquete);
void enviar_rpta_f_truncate_a_kernel();


#endif /* FS_KERNEL_H_ */
