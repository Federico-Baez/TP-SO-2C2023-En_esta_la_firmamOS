#include "../include/shared.h"


t_list* lista_instrucciones(t_log* logger, char* dir){

	FILE* archivo =  fopen(dir, "r");
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
		cod_instruccion pseudo_codigo = convertir_string_a_instruccion(logger,l_instrucciones[0]);
		if(pseudo_codigo >= 0){
			t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
			pseudo_cod->pseudo_c = pseudo_codigo;
//			pseudo_cod->pseudo_c = l_instrucciones[0]; //set sum mov_in ...
			pseudo_cod->fst_param = l_instrucciones[1] ? strdup(l_instrucciones[1]): NULL;
			pseudo_cod->snd_param = l_instrucciones[2] ? strdup(l_instrucciones[2]): NULL;
			log_info(logger, "Se agrega el numero del codigo del  pseudocodigo: %d y la instruccion: %s  \n",pseudo_cod->pseudo_c,l_instrucciones[0] );
			list_add(list_instrucciones,pseudo_cod );
		}
		free(strings_de_instrucciones);
		string_iterate_lines(l_instrucciones, (void*) free);
		free(l_instrucciones);

	}
	fclose(archivo);

	return list_instrucciones;
}

void liberar_lista_instrucciones(t_list *lista) {
    list_destroy_and_destroy_elements(lista, (void *) lista);
}


cod_instruccion convertir_string_a_instruccion(t_log* logger, const char *str_instruccion) {
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

	log_info(logger, "Se lee la siguiente instruccion: %s \n",str_instruccion);


    return -1;
}

t_pcb* crear_pcb(int process_id, int prioridad){
	t_pcb* new_pcb = malloc(sizeof(t_pcb));

	new_pcb->pid = process_id;
	new_pcb->program_counter = 0;
	new_pcb->prioridad = prioridad;
//	new_pcb->registros->AX = 0;
//	new_pcb->registros->BX = 0; // No se si hay que inicializarlas o modificarlas cuando memoria nos de las instrucciones.
//	new_pcb->registros->CX = 0;
//	new_pcb->registros->DX = 0;
	new_pcb->estado = NEW;
	new_pcb->motivo_vuelta = NULL;

	return new_pcb;
}

void cambiar_estado_pcb(t_pcb* pcb, est_pcb nuevo_estado){
	pcb -> estado = nuevo_estado;
}


