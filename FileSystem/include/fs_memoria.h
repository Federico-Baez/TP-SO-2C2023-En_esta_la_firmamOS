#ifndef FS_MEMORIA_H_
#define FS_MEMORIA_H_

#include "m_gestor.h"

// =========== ATENDER MEMORIA ===============
void atender_asignacion_de_bloques_por_creacion_de_proceso(t_buffer* un_buffer);


// =========== ENVIAR A MEMORIA ===============

void  enviar_lista_bloques_a_memoria(int pid, t_list* lista_bloques);


#endif /* FS_MEMORIA_H_ */
