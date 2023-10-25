#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <protocolo.h>
#include <socket.h>
#include <shared.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <protocolo.h>
#include <stdlib.h>

#include <unistd.h>


typedef struct{
	bool modificado;
	bool presente;
	int marco;
	int pos_en_swap;
}tabla_pagina;
t_list* lst_marco;

typedef struct{
	int pid;
	int size;
	char* pathInstrucciones;
	t_list* instrucciones;
}proceso_recibido;

t_list* list_procss_recibidos;

typedef struct{
	int tamamo_usado;
	bool en_memoria;
	int ultimo_uso;//Para el algoritmo LRU
	tabla_pagina* puntero_marco;

}Pagina;
typedef struct{
	Pagina* page;
}tabla_paginas;


t_list* list_pagina;
t_list* list_instruciones;

typedef enum{
	FIFO,
	LRU
}t_algoritmo;


/*
 *
 * MEMORIA VIRTUAL
 */

FILE* disco;

//config
char* IP_MEMORIA;
char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
char* ALGORITMO_REEMPLAZO;
//¿no debería ser un string para leerlo en el config?

t_log* memoria_logger;
t_log* memoria_log_obligatorio;
t_config* memoria_config;
char* server_name;
int socket_server;
int fd_kernel;
int fd_cpu;
int fd_filesystem;
int server_fd_memoria;
void* espacio_usuario;

t_dictionary* tablas;
t_list* instrucciones_para_cpu;


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


/************TODO CARGAR LISTADO DE INSTRUCCIONES DEL PROCESO***************/
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);
void liberar_memoria_de_instrucciones(t_list* instrucciones);
char* obtener_instruccion_por_indice(int indice_instruccion, t_list* instrucciones);

/******************************FUNCIONES PARA PROCESOS*****************************/
proceso_recibido* obtener_proceso_por_id(int pid, t_list* lst_procesos);
void agregar_proceso_a_listado(t_buffer* unBuffer, t_list* lst_procesos_recibido);
void liberar_proceso(proceso_recibido* proceso);
void liberar_listado_procesos(t_list* lst_procesos);
/************TODO MANEJO DE INSTRUCCIONES CON CPU***************/
void enviar_instrucciones_a_cpu(int pid_buffer,int ip_buffer);
/************TODO INICIAR LA TABLA DE PAGINAS***************/
tabla_paginas* crear_tabla_paginas(int pid);
tabla_pagina* crear_marco(int base, bool presente);

#endif /* MEMORIA_H_ */
