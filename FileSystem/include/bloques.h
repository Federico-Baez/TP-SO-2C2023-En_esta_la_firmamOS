#ifndef BLOQUES_H_
#define BLOQUES_H_

#include "m_gestor.h"



//========== SWAP =====================

t_list* swap_obtener_n_cantidad_de_bloques(int cant_bloques);
void setear_bloque_de_swap_como_libre(uint32_t nro_bloque_swap);


//=========== BLOQUES =============================


void* obtener_bloque(char* nombre_archivo, int nro_bloque);
void modificar_bloque(char* nombre_archivo, int nro_bloque, void* info);


#endif /* BLOQUES_H_ */
