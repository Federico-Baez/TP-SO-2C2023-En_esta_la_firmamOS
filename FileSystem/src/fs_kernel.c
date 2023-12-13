#include "../include/fs_kernel.h"

// =========== ATENDER KERNEL ===============


void atender_f_open_de_kernel(t_buffer* un_buffer){
	char* nombre_archivo = recibir_string_del_buffer(un_buffer);
	char* operacion = recibir_string_del_buffer(un_buffer);

	t_paquete* paquete = crear_super_paquete(RESPUESTA_F_OPEN_FK);
	if(strcmp(operacion , "ABRIR_ARCHIVO") == 0){
		cargar_string_al_super_paquete(paquete, "ABRIR_ARCHIVO");
		cargar_int_al_super_paquete(paquete, 1);
		cargar_string_al_super_paquete(paquete,nombre_archivo);
		cargar_int_al_super_paquete(paquete, 10);
	}else{
		cargar_string_al_super_paquete(paquete, "CERRAR_ARCHIVO");
		cargar_int_al_super_paquete(paquete, -1);
	}

	enviar_rpta_f_open_a_kernel(paquete);
	eliminar_paquete(paquete);

}

void atender_f_truncate_de_kernel(t_buffer* un_buffer){
	int tamanio = recibir_int_del_buffer(un_buffer);
	int pid1 = recibir_int_del_buffer(un_buffer);

	enviar_rpta_f_truncate_a_kernel();

}

void atender_fread_de_kernel(t_buffer* un_buffer){
	//Extraes la info del buffer

	//
}






// =========== ENVIAR A KERNEL ===============

void enviar_rpta_f_open_a_kernel(t_paquete* un_paquete){
	enviar_paquete(un_paquete, fd_kernel);
}

void enviar_rpta_f_truncate_a_kernel(){
	t_paquete* paquete1 = crear_super_paquete(RESPUESTA_F_TRUNCATE_FK);
	cargar_int_al_super_paquete(paquete1, 0);
//	cargar_int_al_super_paquete(paquete1, pid1);
	enviar_paquete(paquete1, fd_kernel);
	eliminar_paquete(paquete1);
}



