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


	uint32_t i = 0;

	while(i<sizeArrayFat && list_size(una_lista)<cant_bloques){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT);
		if(tablaFatEnMemoria[i]==0){
			uint32_t* aux = malloc(sizeof(uint32_t));
			*aux = i;
			list_add(una_lista, aux);
		}
		i++;
	}

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

	//====Fin MMAP====

	//busco el primero
//	while(i<sizeArrayFat){
//		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
//		usleep(RETARDO_ACCESO_FAT);
//		if(i == nro_bloque_inicial){
//			list_add(lista_a_devolver, i);
//			break;
//		}
//		i++;
//	}
//	if (fd_archivoTablaFAT == -1) {
//		log_error(filesystem_logger, "Error al mapear el archivo FAT");
//		exit(1);
//	}

	uint32_t i = nro_bloque_inicial;
	uint32_t* aux = malloc(sizeof(uint32_t));
	*aux = i;
	list_add(lista_a_devolver, aux);

	while(tablaFatEnMemoria[i] != EOF_FS){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT);
		i = tablaFatEnMemoria[i];
		*aux = i;
		list_add(lista_a_devolver, aux);
	}
	log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: EOF", i);
	usleep(RETARDO_ACCESO_FAT);

	// Desmapeo de fd_archivoTablaFAT
	munmap(tablaFatEnMemoria, tamanio_fat);

	return lista_a_devolver;
}


void cargar_secuencia_de_bloques_asignados_a_tabla_fat(t_list* una_lista){
	//Lista [uint32_t*]

	//====Inicio MMAP====while(una_lista)
	int fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	uint32_t* tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (fd_archivoTablaFAT == -1 || tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}

	//====Fin MMAP====


	int i;

	for(i = 0; i< list_size(una_lista); i++){
		if(i == list_size(una_lista)-1){
			tablaFatEnMemoria[*((uint32_t*)list_get(una_lista, i))] = EOF_FS;
		}else{
			tablaFatEnMemoria[*((uint32_t*)list_get(una_lista, i))] =  *((uint32_t*)list_get(una_lista, i+1));
		}
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT);
	}


	// Desmapeo de fd_archivoTablaFAT
	munmap(tablaFatEnMemoria, tamanio_fat);

	close(fd_archivoTablaFAT);
}

uint32_t obtener_el_nro_bloque_segun_el_la_posicion_del_seek(int nro_bloque_inicial, int index_seek){

	//====Inicio MMAP====
	int fd_archivoTablaFAT = open(PATH_FAT, O_RDWR);
	tamanio_fat = (CANT_BLOQUES_TOTAL - CANT_BLOQUES_SWAP) * sizeof(uint32_t);

	uint32_t* tablaFatEnMemoria = mmap(NULL, tamanio_fat, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoTablaFAT, 0);

	if (fd_archivoTablaFAT == -1 || tablaFatEnMemoria == MAP_FAILED) {
		log_error(filesystem_logger, "Error al mapear el archivo FAT");
		exit(1);
	}
	close(fd_archivoTablaFAT);

	//====Fin MMAP====

	uint32_t i = nro_bloque_inicial;
	for(int j=0; j<index_seek; j++){
		log_info(filesystem_log_obligatorio, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, tablaFatEnMemoria[i]);
		usleep(RETARDO_ACCESO_FAT);
		i = tablaFatEnMemoria[i];
	}

	// Desmapeo de fd_archivoTablaFAT
	munmap(tablaFatEnMemoria, tamanio_fat);


	return i;
}

void asignar_mas_nro_de_bloque_a_la_secuencia_de_tabla_fat(int nro_bloque_inicial, int cant_bloques_adicionales){

}

void reducir_nro_de_bloques_de_la_secuencia_de_la_tabla_fat(int nro_bloque_inicial, int cant_bloques_a_reducir){

}

