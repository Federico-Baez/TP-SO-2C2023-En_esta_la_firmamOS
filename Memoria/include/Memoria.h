#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "m_gestor.h"
#include "servicios_memoria.h"
#include "marcos.h"
#include "atender_cpu.h"
#include "atender_kernel.h"
#include "atender_fs.h"

char* IP_MEMORIA;
char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
char* ALGORITMO_REEMPLAZO;
int INICIAR_PAGINAS_EN_MEMORIA;

char* server_name;
int socket_server;
int fd_kernel;
int fd_cpu;
int fd_filesystem;
int server_fd_memoria;
void* espacio_usuario;

int ordenCargaGlobal = 1;
/*
 * MEMORIA VIRTUAL
 */
FILE* disco;

/********PAGINA*******/
t_dictionary* tablas;
t_list* lst_marco;

/********LOG Y CONFIGS*******/
t_log* memoria_logger;
t_log* memoria_log_obligatorio;
t_config* memoria_config;

/********RECIBIR PROCESOS Y AGREGAR INSTRUCCIONES*******/
t_list* list_procss_recibidos;
t_list* list_instruciones;

/********SEMAFORO GENERAL PARA LA TABLA*******/
pthread_mutex_t m_tablas;
pthread_mutex_t mutex_lst_marco;
pthread_mutex_t mutex_espacio_usuario;
pthread_mutex_t mutex_ord_carga_global;

/*----------------TODO INIT ------------------------*/
void leer_config();
void finalizar_memoria();



/*----------------COMUNICACION SOCKETS --------*/

void saludar_cliente(void *void_args);
void identificar_modulo(t_buffer* unBuffer, int conexion);
void atender_mensajes_kernel(t_buffer* buffer);
// void recv_inicializar_estructura(t_buffer* buffer);
//static void  procesar_conexion(void *void_args);
void inicializar_memoria();
void iterator(int *value);
void leer_log();
int server_escucha();
void atender_kernel(int cliente_socket);
void atender_cpu(int cliente_socket) ;
void atender_filesystem(int cliente_socket);



#endif /* MEMORIA_H_ */
