/*
 * pagina.h
 *
 *  Created on: Oct 28, 2023
 *      Author: utnso
 */

#ifndef PAGINA_H_
#define PAGINA_H_
#include "m_gestor.h"
#include "proceso_recibido.h"
#include "servicios_memoria.h"



/************TODO INICIAR LA PAGINAS***************/
tabla_paginas* crear_tabla_paginas(int pid);
void destruir_tabla_paginas(tabla_paginas* tabla);
int liberar_pagina(Pagina* una_pagina, int offset, int faltante);
void liberar_paginas(tabla_paginas* una_tabla, int  dirLogica, int tamanio, int pid);
void destruir_pagina(Pagina* pagina);
void eliminar_pagina(tabla_paginas* tabla, int num);
void acceder_pagina(tabla_paginas* tabla, int numero_pagina);
void cargar_pagina_en_memoria(tabla_paginas* tabla, Pagina* pagina);
Pagina* comparar_acceso_LRU(Pagina* pagina1, Pagina* pagina2);
Pagina* comparar_orden_carga(Pagina* pagina1, Pagina* pagina2);
Pagina* victima_pagina_LRU(tabla_paginas* tabla);
Pagina* victima_pagina_FIFO(tabla_paginas* tabla);
/************************COMUNICACION*CON*CPU*******************************/
void devolver_marco_o_pagefault_cpu(int pid, int nro_pagina);
void pagefault_respuesta_cpu();
void lectura_pagina_bloque_cpu(int pid, int dir_fisica);
void escritura_pagina_bloque_cpu(int pid, int dir_fisica, uint32_t valor_uint32);

/************************COMUNICACION*CON*KERNEL*******************************/
void atender_pagefault_kernel(int pid, int nro_pagina);
void responder_pagefault_a_kernel(int pid);
/************************COMUNICACION*CON*FILESYSTEM*******************************/
void pedir_a_FS_la_pagina(Pagina* una_pagina, int pid, int nro_pagina);
void recibir_la_pagina_desde_FS(int pid, void* pagina_swap, int nro_pagina);


/************TODO INICIAR LOS MARCOS***************/
marco* crear_marco(int base, bool presente, int index);
Pagina* obtener_pagina_por_marco(marco* un_marco);


#endif /* PAGINA_H_ */
