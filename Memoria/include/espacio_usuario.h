#ifndef ESPACIO_USUARIO_H_
#define ESPACIO_USUARIO_H_

#include "m_gestor.h"
#include "servicios_memoria.h"

void* copiar_marco_desde_una_dir_fisica(int pid, int dir_fisica);
void escribir_pagina_en_una_dir_fisica_especifica(int pid, int dir_fisica, void* una_pagina);
void eliminar_espacio_de_usuario();
void escribir_data_en_dir_fisica(int pid, int dir_fisica, uint32_t* un_valor);
uint32_t leer_data_de_dir_fisica(int pid, int dir_fisica);



#endif /* ESPACIO_USUARIO_H_ */
