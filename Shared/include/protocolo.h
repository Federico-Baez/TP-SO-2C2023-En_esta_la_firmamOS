#ifndef INCLUDE_PROTOCOLO_H_
#define INCLUDE_PROTOCOLO_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include"shared.h"
#include"socket.h"

/*
typedef enum {
	INT,
	STRING,
	VOID
}op_tipo_dato;
*/

typedef enum{
	MEMORIA,
	FILESYSTEM,
	CPU,
	KERNEL
}modulo_code;

typedef enum{
	MENSAJE,
	PAQUETE,
	INT,
	STRING,
	VOID,
	SUPER_PAQUETE,
	ADMINISTRAR_PAGINA_MEMORIA,
	ACCESO_USUARIO_MEMORIA,
	CREAR_PAGINA,
	ELIMINAR_PAGINA,
	LECTURA_USUARIO,
	ESCRITURA_USUARIO,
	REEMPLAZO_PAGINA,
	HANDSHAKE,
	//-----
	INICICAR_PROCESO_A,
	FINALIZAR_PROCESO_A,
	DETENER_PLANIFICACION_A,
	INICICAR_PLANIFICACION_A,
	MULTIPROGRAMACION_A,
	PROCESO_ESTADO_A,
	//------- KERNEL - MEMORIA
	INICIAR_ESTRUCTURA_KM,
	LIBERAR_ESTRUCTURA_KM,
	PETICION_PAGE_FAULT_KM,
	//------- MEMORIA - KERNEL
	ESTRUCTURA_INICIADA_MK,
	ESTRUCTURA_LIBERADA_MK,
	RESPUESTA_PAGE_FAULT_MK,
	//------- CPU DISPACHT - KERNEL
	FINALIZAR_PROCESO_CPK,
	ATENDER_INSTRUCCION_CPK,
	DESALOJO_PROCESO_CPK,
	//------- KERNEL - CPU
	EJECUTAR_PROCESO_KC,
	DESALOJO_DEL_PROCESO_KC,
	FORZAR_DESALOJO_KC,
	RESPUESTA_INSTRUCCION_KC,
	//------- KERNEL - FILE SYSTEM
	MANEJAR_F_OPEN_KF,
	MANEJAR_F_CLOSE_KF,
	MANEJAR_F_SEEK_KF,
	MANEJAR_F_TRUNCATE_KF,
	MANEJAR_F_READ_KF,
	MANEJAR_F_WRITE_KF,
	//------- FILE SYSTEM - KERNEL
	RESPUESTA_F_OPEN_FK,
	RESPUESTA_F_CLOSE_FK,
	RESPUESTA_F_SEEK_FK,
	RESPUESTA_F_TRUNCATE_FK,
	RESPUESTA_F_READ_FK,
	RESPUESTA_F_WRITE_FK,
	//------- FILE SYSTEM - MEMORIA
	PETICION_ASIGNACION_BLOQUE_SWAP_FM,
	LIBERAR_PAGINAS_FM,
	PETICION_PAGE_FAULT_FM,
	CARGAR_INFO_DE_LECTURA_FM,
	RPTA_CARGAR_INFO_DE_LECTURA_MF,
	GUARDAR_INFO_FM,
	GUARDAR_MARCO_EN_SWAP_FM,
	//------- CPU - MEMORIA
	PETICION_INFO_RELEVANTE_CM,
	PETICION_DE_INSTRUCCIONES_CM,
	PETICION_DE_EJECUCION_CM,
	CONSULTA_DE_PAGINA_CM,
	LECTURA_BLOQUE_CM,
	ESCRITURA_BLOQUE_CM,
	//-------
	MENSAJES_POR_CONSOLA,
	PRUEBAS,
	IDENTIFICACION,
	CONEXION
}op_code;

//typedef enum{
//	MEMORIA,
//	FILE_SYSTEM,
//	CPU,
//	KERNEL
//}t_module;

typedef enum{
	_INT,
	_STRING,
	_VOID
}t_primitivo;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

//typedef struct {
//	modulo_code emisor;
//	op_code tipo_mensaje;
//	int payload_size;
//} __attribute__((packed)) t_header;
//typedef struct {
//	t_header* header;
//	void* payload;
//} t_msg;

/**/


/**/


/******************TODO: revisar los MENSAJES*************/
void handhsake_modules(int conexion, char* mensaje);
void enviar_mensaje(char* mensaje, int socket_cliente);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* logger, int socket_cliente);
void crear_buffer(t_paquete* paquete);


/******************TODO: revisar los PAQUETES*************/
t_list* recibir_paquete(int);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void* serializar_paquete(t_paquete* paquete, int bytes);
void eliminar_paquete(t_paquete* paquete);

/******************TODO: revisar los EMPAQUETACION*************/


/******************TODO: revisar los SENDS*************/


/******************TODO: revisar los RECVS*************/

/******************TODO: revisar los DESTROY*************/


int* recibir_int(t_log* logger, void* coso);
t_list* recibir_paquete_int(int socket_cliente);

t_paquete* crear_super_paquete(op_code code_op);
void cargar_int_al_super_paquete(t_paquete* paquete, int numero);
void cargar_string_al_super_paquete(t_paquete* paquete, char* string);
void cargar_choclo_al_super_paquete(t_paquete* paquete, void* choclo, int size);
int recibir_int_del_buffer(t_buffer* coso);
char* recibir_string_del_buffer(t_buffer* coso);
void* recibir_choclo_del_buffer(t_buffer* coso);

t_buffer* recibiendo_super_paquete(int conexion);
void enviar_handshake(int conexion);
void gestionar_handshake_como_cliente(int conexion, char* modulo_destino, t_log* logger);
void gestionar_handshake_como_server(int conexion, t_log* logger);
void identificarme_con_memoria(int conexion, modulo_code modulo);

/******************PROCESOS*************/
// void send_path_memoria(int fd_modulo, char* path,int size); // Se usa dependiendo de como inicialicemos las estructuras
void send_enviar_path_memoria(int fd_memoria, char* path, int size, int process_id);


#endif /* INCLUDE_PROTOCOLO_H_ */

