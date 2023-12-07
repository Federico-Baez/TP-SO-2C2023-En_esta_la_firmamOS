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
marco* elegir_vcitima_FIFO();
marco* elegir_victima_LRU();

#endif /* MARCOS_H_ */
