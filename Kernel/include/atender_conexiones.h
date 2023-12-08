#ifndef ATENDER_CONEXIONES_H_
#define ATENDER_CONEXIONES_H_

#include "k_gestor.h"
#include "pcb.h"
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"

void iniciar_conexiones();
void atender_memoria();
void atender_filesystem();
void atender_cpu_dispatch();
void atender_cpu_interrupt();

void _gestionar_peticiones_de_memoria();
void _gestionar_peticiones_de_cpu_dispatch();
void _gestionar_interrupt();
void _gestionar_peticiones_de_filesystem();

// Recibir preoceso de CPU
t_pcb* _recibir_proceso_desalojado(t_buffer* un_buffer);
void _atender_motivo_desalojo(char* motivo_desalojo, t_buffer* un_buffer, t_pcb* un_pcb);
void _desalojar_proceso(t_pcb* un_pcb);
void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb);

// PAGE FAULT
void _atender_page_fault(t_page_fault* un_page_fault);

// RESPUESAS DE MEMORIA
void recibir_confirmacion_de_memoria(t_buffer* unBuffer);

void log_blocked_proceso(int pid_process, char* motivo_block);

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
void send_atender_f_open_FS(char* nombre_archivo, char* operacion);

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
void send_atender_F_instruccion_fs(char* nombre_archivo, char* operacion ,int valor_entero, op_code cod);
void send_atender_F_read_write(char* nombre_archivo, int dir_fisica, op_code cod);

void validar_respuesta_F_open(char* operacion, char* mensaje, t_buffer* unBuffer);

t_archivo* obtener_archivo_global(char* nombre_archivo);
t_archivo* crear_archivo(char* nombre_archivo);


#endif /* ATENDER_CONEXIONES_H_ */
