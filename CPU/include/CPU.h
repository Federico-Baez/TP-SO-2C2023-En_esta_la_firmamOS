#ifndef CPU_H_
#define CPU_H_

#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <shared.h>
#include <pthread.h>
#include <protocolo.h>
#include <socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include <math.h>

#define IP_CPU "127.0.0.1"

t_log* cpu_logger;
//t_log* cpu_log_disptach;
//t_log* cpu_log_interrupt;
t_log* cpu_log_obligatorio; //TODO: ¿un solo log obligatorio para cpu o con uno para cada canal esta bien?
t_config* cpu_config;

int server_fd_cpu_dispatch;
int server_fd_cpu_interrupt;
int fd_memoria;
int fd_kernel_dispatch;
int fd_kernel_interrupt;


char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

int* proceso_pid;
int* proceso_ip;
uint32_t* AX;
uint32_t* BX;
uint32_t* CX;
uint32_t* DX;

char** opcode_strings; //Contiene todos los HEADER de las instruccinoes autorizadas
char** instruccion_split;//Contiene el split de la instruccion de memoria

char* s_registros[4] = {"AX", "BX", "CX", "DX"};
int* numero_de_marco;

//Semaforos
sem_t sem_control_fetch_decode;
sem_t sem_control_decode_execute;
sem_t sem_control_peticion_marco_a_memoria;

/*==================================*/

void leer_config(t_config* config);
void iterator(char* value);
void finalizar_CPU();
void iniciar_estructuras(void);
void iniciar_semaforos(void);
void destruir_semaforos(void);

void atender_cpu_dispatch(void);
void atender_cpu_interrupt(void);

//Funciones para KERNEL
void atender_proceso_del_kernel(t_buffer* unBuffer);

//Funciones para MEMORIA
void atender_cpu_memoria(void);
void atender_recepcion_de_instruccion(t_buffer* unBuffer);

//---------------------------------
void iniciar_estructuras_para_atender_al_proceso(t_buffer* unBuffer);
void destruir_estructuras_del_contexto_acttual(void);
void print_proceso(void);

void iniciar_ciclo_de_instruccion();
void ciclo_de_instruccion_fetch();
void ciclo_de_instruccion_decode();
void ciclo_de_instruccion_execute();
bool validador_de_header(char* header_string);
int MMU(int dir_logica, int* dir_fisica);
uint32_t* detectar_registro(char* RX);
t_paquete* alistar_paquete_de_desalojo(op_code code_op);

void simulador_de_eventos(void); //Esto es solo para las pruebas

#endif /* CPU_H_ */
