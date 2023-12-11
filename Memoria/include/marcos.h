/*
 * marcos.h
 *
 *  Created on: Dec 7, 2023
 *      Author: utnso
 */

#ifndef MARCOS_H_
#define MARCOS_H_

#include "m_gestor.h"
#include "servicios_memoria.h"
#include "atender_fs.h"
#include "atender_cpu.h"
#include "atender_kernel.h"

t_marco* crear_marco(int base, bool libre, int index);
void liberar_marco(t_marco* un_marco);
t_marco* obtener_marco_por_nro_marco(int nro_marco);
void destruir_list_marcos_y_todos_los_marcos();
t_marco* obtener_un_marco_de_la_lista_de_marcos(int* tipo_de_marco);
t_marco* elegir_victima_segun_algoritmo();
void evaluar_si_esta_o_no_el_bit_modificado_del_marco(t_marco* un_marco);
void user_escribir_pagina_en_swap(t_marco* un_marco);


//---------------------


#endif /* MARCOS_H_ */
