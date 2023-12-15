#ifndef CPU_H_
#define CPU_H_

#include "../../Shared/include/shared.h"

#define IP_CPU "127.0.0.1"

t_paquete* mochila;
int tam_pagina;
int marco;
uint32_t* valorMarco;
bool interruptFlag = false;

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

int* interrupt_proceso_id;
int* interrupt_proceso_ticket;
char* interrupt_motivo;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

//int* proceso_pid;
//int* proceso_ticket;
//int* proceso_ip;
//uint32_t* AX;
//uint32_t* BX;
//uint32_t* CX;
//uint32_t* DX;

typedef struct{
	int proceso_pid;
	int proceso_ticket;
	int proceso_ip;
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
}t_contexto;

t_contexto* contexto;

//char* motivo_desalojo;

//Esta varaible sirve para cuando haya que desalojar
//voluntariamente por alguna instruccion
bool hay_que_desalojar;

//Esta varaible sirve para cuando haya que desalojar
//pero kernel ya posee el contexto
bool hay_que_desalojar_sin_mensaje = true;

//Contiene todos los HEADER de las instruccinoes autorizadas
char** opcode_strings;
//Contiene el split de la instruccion de memoria
char** instruccion_split;

char* s_registros[4] = {"AX", "BX", "CX", "DX"};
int* numero_de_marco;

//Semaforos
sem_t sem_control_fetch_decode;
sem_t sem_control_decode_execute;
sem_t sem_control_peticion_marco_a_memoria;
sem_t sem_control_peticion_lectura_a_memoria;
sem_t sem_control_peticion_escritura_a_memoria;
sem_t sem_control_respuesta_kernel;

pthread_mutex_t mutex_interruptFlag;
pthread_mutex_t mutex_manejo_contexto;

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
bool preguntando_si_hay_interrupciones_vigentes();

void ciclo_de_instruccion_execute();
bool validador_de_header(char* header_string);
int MMU(int dir_logica);
int solicitar_valor_memoria(int direccion_logica);
void escribir_valor_memoria(int dir_logica, uint32_t valorAEscribir);
uint32_t* detectar_registro(char* RX);
t_paquete* alistar_paquete_de_desalojo(op_code code_op);
void enviarPaqueteKernel(char* motivo);
void enviarPaqueteKernelConInfoExtra(t_paquete* infoExtra);

void simulador_de_eventos(void); //Esto es solo para las pruebas

#endif /* CPU_H_ */
