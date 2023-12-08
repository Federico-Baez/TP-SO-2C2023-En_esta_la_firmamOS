/*
 * marcos.h
 *
 *  Created on: Dec 7, 2023
 *      Author: utnso
 */

#ifndef MARCOS_H_
#define MARCOS_H_

#include "m_gestor.h"
#include "pagina.h"

void asignar_marcos_a_cada_pagina(proceso_recibido* un_proceso);
marco* pedir_un_marco_de_la_lista_de_marcos();
void guardar_marco_en_swap(marco* un_marco);
marco* elegir_victima_FIFO();
marco* elegir_victima_LRU();


void leer_archivo_de_FS_y_cargarlo_en_memoria(void* un_buffer);
void leer_todo_el_marco_de_la_dir_fisica_y_enviarlo_a_FS(void* un_buffer);

marco* buscar_marco_por_direccion_fisica(int dir_fisica);

#endif /* MARCOS_H_ */
