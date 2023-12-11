#include "../include/espacio_usuario.h"

void* copiar_marco_desde_una_dir_fisica(int pid, int dir_fisica){
	void* pagina_copy = malloc(TAM_PAGINA);
	memcpy(pagina_copy, espacio_usuario + dir_fisica, TAM_PAGINA);

	//Log obligatorio por acceso a espacio de usuario
	logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica);

	return pagina_copy;
}

void escribir_pagina_en_una_dir_fisica_especifica(int pid, int dir_fisica, void* una_pagina){
	memcpy(espacio_usuario + dir_fisica, una_pagina, TAM_PAGINA);

	//Log Obligatorio por lectura de pagina SWAP
	logg_acceso_a_espacio_de_usuario(pid, "escribir", dir_fisica);

}

void eliminar_espacio_de_usuario(){
	free(espacio_usuario);
}

void escribir_data_en_dir_fisica(int pid, int dir_fisica, uint32_t* un_valor){
	memcpy(espacio_usuario + dir_fisica, un_valor, sizeof(uint32_t));
	//Log Obligatorio por lectura de pagina SWAP
	logg_acceso_a_espacio_de_usuario(pid, "escribir", dir_fisica);
}

uint32_t leer_data_de_dir_fisica(int pid, int dir_fisica){
	uint32_t* valor_leido = malloc(sizeof(uint32_t));
	uint32_t dato_retorno;

	memcpy(valor_leido, espacio_usuario + dir_fisica, sizeof(uint32_t));
	dato_retorno = *valor_leido;

	free(valor_leido);

	//Log Obligatorio por lectura de pagina SWAP
	logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica);

	return dato_retorno;
}
