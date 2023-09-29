#include "../include/shared.h"


t_list* lista_instrucciones(t_log* logger, char* dir){

	FILE* archivo =  fopen(dir, "rt");
	char* strings_de_instrucciones;
	t_list* list_instrucciones = list_create();

	if(archivo==NULL){
		log_error(logger, "No se encontro el archivo");
		return list_instrucciones;
	}
	char linea[256];
	while(fgets(linea,sizeof(linea),archivo)){
		log_info(logger, "Leyendo archivo de instrucciones");
		strings_de_instrucciones = string_from_format("%s", linea);
		char** l_instrucciones = string_split(strings_de_instrucciones, " ");
		cod_instruccion pseudo_codigo = convertir_string_a_instruccion(l_instrucciones[0]);
		if(pseudo_codigo != 0){
			t_instruccion_codigo* pseudo_cod = malloc(sizeof(cod_instruccion));
			pseudo_cod->pseudo_c = pseudo_codigo;
			pseudo_cod->fst_param = l_instrucciones[1] ? strdup(l_instrucciones[1]): NULL;
			pseudo_cod->snd_param = l_instrucciones[2] ? strdup(l_instrucciones[2]): NULL;
			log_info(logger, "Primera fila de speudocodigo: %d, primer parametro: %s y segundo parametro: %s",pseudo_cod->pseudo_c,pseudo_cod->fst_param, pseudo_cod->snd_param );
			list_add(list_instrucciones,pseudo_cod );
		}
		free(strings_de_instrucciones);
		string_iterate_lines(l_instrucciones, (void*) free);
		free(l_instrucciones);

	}
	fclose(archivo);

	return list_instrucciones;
}



cod_instruccion convertir_string_a_instruccion(const char *str_instruccion) {
    if (strcmp(str_instruccion, "SET") == 0) return SET;
    if (strcmp(str_instruccion, "SUM") == 0) return SUM;
    if (strcmp(str_instruccion, "SUB") == 0) return SUB;
    if (strcmp(str_instruccion, "MOV_IN") == 0) return MOV_IN;
    if (strcmp(str_instruccion, "MOV_OUT") == 0) return MOV_OUT;
    if (strcmp(str_instruccion, "SLEEP") == 0) return SLEEP;
    if (strcmp(str_instruccion, "JNZ") == 0) return JNZ;
    if (strcmp(str_instruccion, "WAIT") == 0) return WAIT;
    if (strcmp(str_instruccion, "SIGNAL") == 0) return SIGNAL;
    if (strcmp(str_instruccion, "F_OPEN") == 0) return F_OPEN;
    if (strcmp(str_instruccion, "F_TRUNCATE") == 0) return F_TRUNCATE;
    if (strcmp(str_instruccion, "F_SEEK") == 0) return F_SEEK;
    if (strcmp(str_instruccion, "F_WRITE") == 0) return F_WRITE;
    if (strcmp(str_instruccion, "F_READ") == 0) return F_READ;
    if (strcmp(str_instruccion, "F_CLOSE") == 0) return F_CLOSE;
    if (strcmp(str_instruccion, "EXIT") == 0) return EXIT_P;
    return 0;
}


