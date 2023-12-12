#ifndef ATENDER_KERNEL_H_
#define ATENDER_KERNEL_H_

#include "m_gestor.h"
#include "proceso.h"
#include "marcos.h"
#include "espacio_usuario.h"
#include "servicios_memoria.h"
#include "atender_fs.h"
#include "atender_cpu.h"


//============RECIBIDOS DE KERNEL=======================

void iniciar_estructura_para_un_proceso_nuevo(t_buffer* un_Buffer);
void eliminar_proceso_y_liberar_estructuras(t_buffer* unBuffer);
void atender_pagefault_kernel(t_buffer* un_buffer);

//============ENVIOS A KERNEL=======================

void responder_a_kernel_confirmacion_del_proceso_creado();
void enviar_a_kernel_rpta_del_pedido_de_carga_de_pagina(int pid);

#endif /* ATENDER_KERNEL_H_ */
