#ifndef ATENDER_INSTRUCCIONES_CPU_H_
#define ATENDER_INSTRUCCIONES_CPU_H_


#include "k_gestor.h"
#include "pcb.h"
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"

// ----- Funciones de manejo de INSTRUCCIONES del CPU ------
// ----- SLEEP -----
void atender_sleep(t_pcb* pcb, int seconds_blocked, char* motivo_block);
void manejar_tiempo_sleep(t_sleep* pcb_sleep);

// ----- WAIT  -----
void atender_wait(t_pcb* pcb,char* recurso_solicitado);

// ----- SIGNAL -----
void atender_signal(t_pcb* pcb,char* recurso_a_liberar);

// ----- Funciones para recursos
t_recurso* buscar_recurso(char* recurso_solicitado);
void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recruso, pthread_mutex_t mutex_recruso);

// ----- F_OPEN -----
void atender_F_open(t_pcb* pcb, char* nombre_archivo, char* modo_apertura);

// ----- F_CLOSE -----
void atender_F_close(char* close_archivo, t_pcb* pcb);

// ----- F_SEEK -----
void atender_F_seek(char* nombre_archivo , int puntero_archivo, t_pcb* pcb);

// ----- F_TRUNCATE -----
void atender_F_truncate(char* nombre_archivo , int nuevo_size_archivo, t_pcb* pcb);

// ----- F_READ -----
void atender_F_read(char* nombre_archivo , int dir_fisica, t_pcb* pcb);

// ----- F_WRITE -----
void atender_F_write(char* nombre_archivo , int dir_fisica, t_pcb* pcb);

// ----- Funciones para archivos
void send_atender_f_open(char* nombre_archivo, char* operacion);
void send_atender_f_truncate(char* nombre_archivo,int nuevo_tamanio, int pid_process);
void send_atender_F_read_write(char* nombre_archivo, int puntero_pcb, int dir_fisica, int pid_process, op_code code);

void validar_respuesta_F_open(char* operacion, int confirmacion, t_buffer* unBuffer);

t_archivo* obtener_archivo_global(char* nombre_archivo);
t_archivo* crear_archivo(char* nombre_archivo, int size_archivo);

// ---- EXIT ----
void plp_exit(t_pcb* pcb);


#endif /* ATENDER_INSTRUCCIONES_CPU_H_ */
