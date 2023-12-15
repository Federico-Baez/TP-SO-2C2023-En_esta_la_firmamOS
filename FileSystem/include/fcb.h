#ifndef FCB_H_
#define FCB_H_

#include "m_gestor.h"

void crear_fcb(char* nombre_archivo);
t_fcb* obtener_fcb(char* nombre_archivo);
void setear_size_de_una_fcb(t_fcb* una_fcb, int nuevo_size);

#endif /* FCB_H_ */
