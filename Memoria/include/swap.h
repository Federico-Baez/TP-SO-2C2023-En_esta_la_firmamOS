#ifndef SWAP_H_
#define SWAP_H_
#include "m_gestor.h"
#include "proceso_recibido.h"

void solicitar_asignacion_bloques_SWAP_a_FS(int pid, int numero_paginas);
void asignar_posicions_de_SWAP_a_tabla_de_paginas(void* pid_cant_lista);


#endif /* SWAP_H_ */
