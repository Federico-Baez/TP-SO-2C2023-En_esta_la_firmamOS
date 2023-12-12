#include "../include/atender_cpu.h"

//============RECIBIDOS DE CPU=======================

void atender_peticion_de_instruccion(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int ip = recibir_int_del_buffer(un_buffer);

	//Buscar proceso por el PID
	t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(un_proceso, ip);

	//Enviar_instruccion a CPU
	enviar_una_instruccion_a_cpu(instruccion);
}

void atender_consulta_de_pagina(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	int nro_pagina = recibir_int_del_buffer(unBuffer);

	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	t_pagina* una_pagina = pag_obtener_pagina_completa(un_proceso, nro_pagina);

	int respuesta_a_cpu;
	if(una_pagina->presente){
		respuesta_a_cpu = una_pagina->nro_marco;
	}else{
		respuesta_a_cpu = -1;
	}
	enviar_a_CPU_respuesta_por_consulta_de_pagina(respuesta_a_cpu);
}

void leer_valor_de_dir_fisica_y_devolver_a_cpu(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);

	//Copiar dato de uint32_t
	uint32_t valor = leer_data_de_dir_fisica(pid, dir_fisica);

	//Enviar valor a CPU
	enviar_valor_a_cpu(valor);
}

void escribir_valor_en_dir_fisica(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	int dir_fisica = recibir_int_del_buffer(un_buffer);

	void* choclito = recibir_choclo_del_buffer(un_buffer);
	uint32_t* valor = (uint32_t*)choclito;

//	uint32_t* valor = (uint32_t*)recibir_choclo_del_buffer(un_buffer);

	escribir_data_en_dir_fisica(pid, dir_fisica, valor);
	enviar_a_CPU_respuesta_por_pedido_de_escritura_en_memoria(pid);

}



//============ENVIOS A CPU=======================

void enviar_una_instruccion_a_cpu(char* instruccion){
	// M -> CPU : [char* ]
	retardo_respuesta_cpu_fs();
	t_paquete* paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);
	cargar_string_al_super_paquete(paquete, instruccion);
	enviar_paquete(paquete, fd_cpu);
	eliminar_paquete(paquete);
}

void enviar_a_CPU_respuesta_por_consulta_de_pagina(int respuesta_a_cpu){
	// M -> CPU : [int nro_bloque_o_(-1)_pagefault]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(CONSULTA_DE_PAGINA_CM);
	cargar_int_al_super_paquete(un_paquete, respuesta_a_cpu);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_valor_a_cpu(uint32_t valor){
	//M -> CPU : [void* valor]
	retardo_respuesta_cpu_fs();
	uint32_t* un_valor = malloc(sizeof(uint32_t));
	*un_valor = valor;
	t_paquete* un_paquete = crear_super_paquete(LECTURA_BLOQUE_CM);
	cargar_choclo_al_super_paquete(un_paquete, un_valor, sizeof(uint32_t));
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
	free(un_valor);
}

void enviar_a_CPU_respuesta_por_pedido_de_escritura_en_memoria(int pid){
	//M -> CPU : [char* OK]
	retardo_respuesta_cpu_fs();
	t_paquete* un_paquete = crear_super_paquete(ESCRITURA_BLOQUE_CM);
//	cargar_int_al_super_paquete(un_paquete, pid);
	cargar_string_al_super_paquete(un_paquete, "OK");
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}





