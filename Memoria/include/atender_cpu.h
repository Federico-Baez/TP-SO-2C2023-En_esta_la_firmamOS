#ifndef ATENDER_CPU_H_
#define ATENDER_CPU_H_

#include "m_gestor.h"
#include "proceso.h"
#include "marcos.h"
#include "espacio_usuario.h"
#include "servicios_memoria.h"

//============RECIBIDOS DE CPU=======================
void atender_peticion_de_instruccion(t_buffer* un_buffer);
void atender_consulta_de_pagina(t_buffer* unBuffer);
void leer_valor_de_dir_fisica_y_devolver_a_cpu(t_buffer* un_buffer);
void escribir_valor_en_dir_fisica(t_buffer* un_buffer);


//============ENVIOS A CPU=======================

void enviar_una_instruccion_a_cpu(char* instruccion);
void enviar_a_CPU_respuesta_por_consulta_de_pagina(int respuesta_a_cpu);
void enviar_valor_a_cpu(uint32_t valor);
void enviar_a_CPU_respuesta_por_pedido_de_escritura_en_memoria(int pid);




#endif /* ATENDER_CPU_H_ */
