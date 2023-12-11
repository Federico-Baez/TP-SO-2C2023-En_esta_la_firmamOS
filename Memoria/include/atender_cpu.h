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
void escritura_pagina_bloque_cpu(t_buffer* un_buffer);


//============ENVIOS A CPU=======================

void enviar_una_instruccion_a_cpu(char* instruccion);
void enviar_a_CPU_respuesta_por_consulta_de_pagina(int respuesta_a_cpu);





#endif /* ATENDER_CPU_H_ */
