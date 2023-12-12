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

void escritura_pagina_bloque_cpu(t_buffer* un_buffer){
	//[FALTA Hacer la logica]
}


//============ENVIOS A CPU=======================

void enviar_una_instruccion_a_cpu(char* instruccion){
	// M -> CPU : [char* ]
	t_paquete* paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CM);
	cargar_string_al_super_paquete(paquete, instruccion);
	enviar_paquete(paquete, fd_cpu);
	eliminar_paquete(paquete);
}

void enviar_a_CPU_respuesta_por_consulta_de_pagina(int respuesta_a_cpu){
	// M -> CPU : [int nro_bloque_o_(-1)_pagefault]
	t_paquete* un_paquete = crear_super_paquete(CONSULTA_DE_PAGINA_CM);
	cargar_int_al_super_paquete(un_paquete, respuesta_a_cpu);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

