#include "../include/tabla_fat.h"


t_list* obtener_n_cantidad_de_bloques_libres_de_tabla_fat(int cant_bloques){
	t_list* una_lista = list_create();

	//[FALTA] Hacer la logica

	//MMAP.......



	return una_lista;
}



t_list* obtener_secuencia_de_bloques_de_archivo(int nro_bloque){
	t_list* lista_a_devolver = list_create();

	//


	return lista_a_devolver;
}

uint32_t obtener_el_nro_bloque_segun_el_la_posicion_del_seek(int nro_bloque_inicial, int index_seek){

}

void cargar_secuencia_de_bloques_asignados_a_tabla_fat(t_list* una_lista){
	//Lista [uint32_t*]
}

void asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat(uint32_t bloque_inicial, int cant_bloques_adicionales){

}

void reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat(uint32_t bloque_inicial, int cant_bloques_a_reducir){

}
