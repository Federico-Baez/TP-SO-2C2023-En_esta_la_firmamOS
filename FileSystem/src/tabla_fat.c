#include "../include/tabla_fat.h"


t_list* obtener_n_cantidad_de_bloques_libres_de_tabla_fat(int cant_bloques){
	log_warning(filesystem_logger, "inicio obtener_n_cantidad_de_bloques_libres_de_tabla_fat");
	t_list* una_lista = list_create();

	size_t sizeArrayFat = tamanio_fat/sizeof(uint32_t);

	uint32_t i = 0;

	while(i<sizeArrayFat && list_size(una_lista)<cant_bloques){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT*1000);
		if(tablaFatEnMemoria[i]==0){
			uint32_t* aux = malloc(sizeof(uint32_t));
			*aux = i;
			list_add(una_lista, aux);
		}
		i++;
	}

	if(list_size(una_lista)<cant_bloques){
		log_warning(filesystem_logger, "fin obtener_n_cantidad_de_bloques_libres_de_tabla_fat");
		return NULL;
	}
	log_warning(filesystem_logger, "fin obtener_n_cantidad_de_bloques_libres_de_tabla_fat");
	return una_lista;
}

t_list* obtener_secuencia_de_bloques_de_archivo(int nro_bloque_inicial){
	log_warning(filesystem_logger, "inicio obtener_secuencia_de_bloques_de_archivo");
	t_list* lista_a_devolver = list_create();

	uint32_t i = nro_bloque_inicial;
	uint32_t* aux = malloc(sizeof(uint32_t));
	*aux = i;
	list_add(lista_a_devolver, aux);

	while(tablaFatEnMemoria[i] != EOF_FS){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT*1000);
		i = tablaFatEnMemoria[i];
		*aux = i;
		list_add(lista_a_devolver, aux);
	}
	log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: EOF", i);
	usleep(RETARDO_ACCESO_FAT*1000);

	log_warning(filesystem_logger, "fin obtener_secuencia_de_bloques_de_archivo");
	return lista_a_devolver;
}


void cargar_secuencia_de_bloques_asignados_a_tabla_fat(t_list* una_lista){
	//Lista [uint32_t*]
	log_warning(filesystem_logger, "inicio cargar_secuencia_de_bloques_asignados_a_tabla_fat");
	int i;

	for(i = 0; i< list_size(una_lista); i++){
		if(i == list_size(una_lista)-1){
			tablaFatEnMemoria[*((uint32_t*)list_get(una_lista, i))] = EOF_FS;
		}else{
			tablaFatEnMemoria[*((uint32_t*)list_get(una_lista, i))] =  *((uint32_t*)list_get(una_lista, i+1));
		}
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT*1000);
	}
	log_warning(filesystem_logger, "fin cargar_secuencia_de_bloques_asignados_a_tabla_fat");
}

uint32_t obtener_el_nro_bloque_segun_el_la_posicion_del_seek(int nro_bloque_inicial, int index_seek){
	log_warning(filesystem_logger, "inicio obtener_el_nro_bloque_segun_el_la_posicion_del_seek");
	uint32_t bloque_desplazado = index_seek/TAM_BLOQUE;
	uint32_t i = nro_bloque_inicial;

	for(int j=0; j<bloque_desplazado; j++){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT*1000);
		i = tablaFatEnMemoria[i];
	}
	log_warning(filesystem_logger, "fin obtener_el_nro_bloque_segun_el_la_posicion_del_seek");
	return i;
}

void asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat(int nro_bloque_inicial, int cant_bloques_adicionales){
	log_warning(filesystem_logger, "inicio asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat");
	t_list* bloques_de_archivo = obtener_secuencia_de_bloques_de_archivo(nro_bloque_inicial);
	t_list* bloques_a_agregar = obtener_n_cantidad_de_bloques_libres_de_tabla_fat(cant_bloques_adicionales);

	uint32_t bloque_final = *((uint32_t*)list_get(bloques_de_archivo, list_size(bloques_de_archivo)-1));
	tablaFatEnMemoria[bloque_final] = *((uint32_t*)list_get(bloques_a_agregar, 0));

	for(int i=0; i < list_size(bloques_a_agregar); i++){

		if(i == list_size(bloques_a_agregar)-1){
			tablaFatEnMemoria[*((uint32_t*)list_get(bloques_a_agregar, i))] = EOF_FS;
		}else{
			tablaFatEnMemoria[*((uint32_t*)list_get(bloques_a_agregar, i))] =  *((uint32_t*)list_get(bloques_a_agregar, i+1));
		}

		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", bloque_final, tablaFatEnMemoria[bloque_final]);
		usleep(RETARDO_ACCESO_FAT*1000);
	}
	log_warning(filesystem_logger, "fin asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat");
}

void reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat(int nro_bloque_inicial, int cant_bloques_a_reducir){
	log_warning(filesystem_logger, "inicio reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat");
	t_list* bloques_de_archivo = obtener_secuencia_de_bloques_de_archivo(nro_bloque_inicial);

	int posicion = list_size(bloques_de_archivo)-1;
	uint32_t bloque_final = *((uint32_t*)list_get(bloques_de_archivo, posicion));

	for(int i = cant_bloques_a_reducir; i > 0 ; i--){
		tablaFatEnMemoria[bloque_final] = 0;
		bloque_final = *((uint32_t*)list_get(bloques_de_archivo, posicion-1));
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", bloque_final, tablaFatEnMemoria[bloque_final]);
		usleep(RETARDO_ACCESO_FAT*1000);
	}

	tablaFatEnMemoria[bloque_final] = EOF_FS;
	log_warning(filesystem_logger, "fin reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat");
}

