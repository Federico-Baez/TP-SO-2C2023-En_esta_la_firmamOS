#ifndef CPU_H_
#define CPU_H_

#include <protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <shared.h>
#include <pthread.h>
#include <protocolo.h>
#include <socket.h>
#include <stdlib.h>

#define IP_CPU "127.0.0.1"
t_temporal* sada;
t_log* cpu_logger;
t_log* cpu_log_disptach;
t_log* cpu_log_interrupt;
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

void leer_config(t_config* config);
void iterator(char* value);

void atender_cpu_dispatch(void);
void atender_cpu_interrupt(void);
void atender_cpu_memoria(void);


#endif /* CPU_H_ */
