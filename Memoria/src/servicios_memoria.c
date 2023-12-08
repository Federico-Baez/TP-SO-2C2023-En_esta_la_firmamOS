#include "../include/servicios_memoria.h"

void logg_crear_destruir_tabla_de_paginas(int pid, int cant_paginas){
	//PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño: <%d>", pid, cant_paginas);
}
void logg_destruir_tabla_de_paginas(int pid, int cant_paginas){
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño: <%d>", pid, cant_paginas);
}

void logg_acceso_a_tabla_de_paginas(int pid, int nro_pagina, int nro_marco){
	//PID: <PID> - Pagina: <PAGINA> - Marco: <MARCO>
	log_info(memoria_log_obligatorio, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, nro_pagina, nro_marco);
}

void logg_acceso_a_espacio_de_usuario(int pid, char* accion, int dir_fisica){
	//PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>
	if (strcmp(accion, "leer") == 0){
		log_info(memoria_log_obligatorio, "PID: <%d> - Accion: <%s> - Direccion fisica: <%d>", pid, "LEER", dir_fisica);
	}else if(strcmp(accion, "escribir") == 0){
		log_info(memoria_log_obligatorio, "PID: <%d> - Accion: <%s> - Direccion fisica: <%d>", pid, "ESCRIBIR", dir_fisica);
	}else{
		log_error(memoria_logger, "logg_acceso_a_espacio_de_usuario, con parametro incorrecto, tiene que ser 0-lectura 1-escritura");
		exit(EXIT_FAILURE);
	}
}

void logg_reemplazar_pagina(int nro_marco, int out_pid, int out_nro_pag, int in_pid, int in_nro_pag){
	//REEMPLAZO - Marco: <NRO_MARCO> - Page Out: <PID>-<NRO_PAGINA> - Page In: <PID>-<NRO_PAGINA>
	log_info(memoria_log_obligatorio, "REEMPLAZO - Marco: <%d> - Page Out: <%d>-<%d> - Page In: <%d>-<%d>", nro_marco, out_pid, out_nro_pag, in_pid, in_nro_pag);
}

void logg_lectura_pagina_swap(int pid, int nro_marco, int nro_pagina){
	//SWAP IN -  PID: <PID> - Marco: <MARCO> - Page In: <PID>-<NRO_PAGINA>
	log_info(memoria_log_obligatorio, "SWAP IN - PID: <%d> - Marco: <%d> - Page In: <%d>-<%d>", pid, nro_marco, pid, nro_pagina);
}

void logg_escritura_pagina_en_swap(int pid, int nro_marco, int nro_pagina){
	//SWAP OUT -  PID: <PID> - Marco: <MARCO> - Page Out: <PID>-<NRO_PAGINA>
	log_info(memoria_log_obligatorio, "SWAP OUT - PID: <%d> - Marco: <%d> - Page Out: <%d>-<%d>", pid, nro_marco, pid, nro_pagina);
}
