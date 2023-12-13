#include "../include/bloques.h"

//========== SWAP =====================

t_list* swap_obtener_n_cantidad_de_bloques(int cant_bloques){

}

void* swap_obtener_bloque_pagina_de_pos_swap(int pos_swap){
	//LOG Obligatorio
	//Acceso SWAP: <NRO_BLOQUE>
}

void swap_actualizar_pagina_bloque_swap(int pos_swap, void* pagina){
	//LOG Obligatorio
	//Acceso SWAP: <NRO_BLOQUE>
}


//=========== TABLA FAT =============================

void* obtener_bloque_especifico_para_lectura(char* nombre_archivo, int puntero_de_kernel, uint32_t nro_bloque){
	//LA info de nombre de archivo y puntero_de_kernel son para el log obligatorio
	//Acceso Bloque - Archivo: <NOMBRE_ARCHIVO> - Bloque Archivo: <NUMERO_BLOQUE_ARCHIVO> - Bloque FS: <NUMERO_BLOQUE_FS>

	//Para hacer la logica de la funcion solo se necesitaria en nro bloque

}

void guardar_info_pagina_en_un_bloque_especifico(char* nombre_archivo, int puntero_de_kernel, uint32_t nro_bloque, void* info){
	//LA info de nombre de archivo y puntero_de_kernel son para el log obligatorio
	//Acceso Bloque - Archivo: <NOMBRE_ARCHIVO> - Bloque Archivo: <NUMERO_BLOQUE_ARCHIVO> - Bloque FS: <NUMERO_BLOQUE_FS>

	//Para hacer la logica de la funcion solo se necesitaria en nro bloque y el void* info
}

