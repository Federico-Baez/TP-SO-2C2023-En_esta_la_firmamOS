#ifndef SERVICIOS_MEMORIA_H_
#define SERVICIOS_MEMORIA_H_

#include "m_gestor.h"

void logg_crear_destruir_tabla_de_paginas(int pid, int cant_paginas);
void logg_acceso_a_tabla_de_paginas(int pid, int nro_pagina, int nro_marco);
/* * 0 - Leer
 *   1 - Escribir*/
void logg_acceso_a_espacio_de_usuario(int pid, int leer_0_y_escribir_1, int dir_fisica);
void logg_reemplazar_pagina(int nro_marco, int out_pid, int out_nro_pag, int in_pid, int in_nro_pag);
void logg_lectura_pagina_swap(int pid, int nro_marco, int nro_pagina);
void logg_escritura_pagina_en_swap(int pid, int nro_marco, int nro_pagina);

#endif /* SERVICIOS_MEMORIA_H_ */
