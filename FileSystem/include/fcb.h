#ifndef FCB_H_
#define FCB_H_

#include "m_gestor.h"

bool verificar_si_existe_la_FCB(char* nombre_del_archivo);
void crear_fcb(char* nombre_archivo, int size, int bloq_inical);
t_fcb* obtener_fcb(char* nombre_archivo);
void setear_size_de_una_fcb(t_fcb* una_fcb, int nuevo_size);

#endif /* FCB_H_ */
