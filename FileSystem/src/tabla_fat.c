#include "../include/tabla_fat.h"


t_list* obtener_n_cantidad_de_bloques_libres_de_tabla_fat(int cant_bloques){
	t_list* una_lista = list_create();

	//====Inicio MMAP====
	int fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	uint32_t* tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (fd_archivoTablaFAT == -1 || tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}
	close(fd_archivoTablaFAT);

	size_t sizeArrayFat = tamanio_fat/sizeof(uint32_t);
	//====Fin MMAP====


//	int i = 0;
//	while(i<sizeArrayFat && list_size(una_lista)<cant_bloques){
//		if(tablaFatEnMemoria[i]==0){
//			list_add(una_lista, i);
//		}
//		i++;
//	}

    // Desmapeo de fd_archivoTablaFAT
    munmap(tablaFatEnMemoria, tamanio_fat);


	if(list_size(una_lista)<cant_bloques){
		return NULL;
	}

	return una_lista;
}



t_list* obtener_secuencia_de_bloques_de_archivo(int nro_bloque_inicial){
	t_list* lista_a_devolver = list_create();

	//====Inicio MMAP====
	int fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	uint32_t* tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (fd_archivoTablaFAT == -1 || tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}
	close(fd_archivoTablaFAT);

	size_t sizeArrayFat = tamanio_fat/sizeof(uint32_t);
	//====Fin MMAP====

	int i = 0;
	while(i<sizeArrayFat && tablaFatEnMemoria[i] != ){
		if(tablaFatEnMemoria[i]==0){
			list_add(una_lista, i);
		}
		i++;
	}


	return lista_a_devolver;
}

uint32_t obtener_el_nro_bloque_segun_el_la_posicion_del_seek(int nro_bloque_inicial, int index_seek){

}

void cargar_secuencia_de_bloques_asignados_a_tabla_fat(t_list* una_lista){
	//Lista [uint32_t*]

	//====Inicio MMAP====
	int fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	uint32_t* tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (fd_archivoTablaFAT == -1 || tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}
	close(fd_archivoTablaFAT);

	size_t sizeArrayFat = tamanio_fat/sizeof(uint32_t);
	//====Fin MMAP====
}

void asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat(int nro_bloque_inicial, int cant_bloques_adicionales){

}

void reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat(int nro_bloque_inicial, int cant_bloques_a_reducir){

}
