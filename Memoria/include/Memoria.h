#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "m_gestor.h"
#include "proceso_recibido.h"
#include "swap.h"
#include "pagina.h"

char* IP_MEMORIA;
char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
char* ALGORITMO_REEMPLAZO;

char* server_name;
int socket_server;
int fd_kernel;
int fd_cpu;
int fd_filesystem;
int server_fd_memoria;
void* espacio_usuario;

int ordenCargaGlobal = 0;
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

void* buscar_tabla(int pid);
/************TODO CARGAR LISTADO DE INSTRUCCIONES DEL PROCESO***************/
//t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);
//void liberar_memoria_de_instrucciones(t_list* instrucciones);
//char* obtener_instruccion_por_indice(int indice_instruccion, t_list* instrucciones);

/******************************FUNCIONES PARA PROCESOS*****************************/
//proceso_recibido* obtener_proceso_por_id(int pid, t_list* lst_procesos);
//void agregar_proceso_a_listado(t_buffer* unBuffer, t_list* lst_procesos_recibido);
//void liberar_proceso(proceso_recibido* proceso);
//void liberar_listado_procesos(t_list* lst_procesos);
/************TODO MANEJO DE INSTRUCCIONES CON CPU***************/
void enviar_instrucciones_a_cpu(int pid_buffer,int ip_buffer);


/************TODO FUNCIONES PARA BLOQUEAR Y DEBLOQUEAR***************/

void bloquear_lista_tablas();
void desbloquear_lista_tablas();
#endif /* MEMORIA_H_ */
