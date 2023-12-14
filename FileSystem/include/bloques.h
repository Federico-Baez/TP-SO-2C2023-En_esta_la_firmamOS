#ifndef BLOQUES_H_
#define BLOQUES_H_

#include "m_gestor.h"



//========== SWAP =====================

t_list* swap_obtener_n_cantidad_de_bloques(int cant_bloques);
void* swap_obtener_bloque_pagina_de_pos_swap(int pos_swap);
void swap_actualizar_pagina_bloque_swap(int pos_swap, void* pagina);
void setear_bloque_de_swap_como_libre(int nro_bloque_swap);


//=========== TABLA FAT =============================


void* obtener_bloque_especifico_para_lectura(char* nombre_archivo, int nro_bloque);
void guardar_info_pagina_en_un_bloque_especifico(char* nombre_archivo, int nro_bloque, void* info);


#endif /* BLOQUES_H_ */
