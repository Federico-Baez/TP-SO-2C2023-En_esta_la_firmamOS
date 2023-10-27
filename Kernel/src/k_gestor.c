#include "../include/k_gestor.h"

//void pausador(){
//	pthread_mutex_lock(&mutex_pausa);
//	if(var_pausa == 1){
//		log_warning(kernel_logger, "Planificadores Pausados");
//		sem_wait(&sem_pausa);
//	}
//	pthread_mutex_unlock(&mutex_pausa);
//}

void public_imprimir_procesos_por_estado_v1(){

	int* __encontrando_el_maximo(int* valor_1, int* valor_2){
		if(*valor_1 >= *valor_2)return valor_1;
		else return valor_2;
	}

	int* mayor_size;
	int size_new = list_size(lista_new);
	int size_ready = list_size(lista_ready);
	int size_execute = list_size(lista_execute);
	int size_blocked = list_size(lista_blocked);
	int size_exit = list_size(lista_exit);

	t_list* una_lista_12345 = list_create();
	list_add(una_lista_12345, &size_new);
	list_add(una_lista_12345, &size_ready);
	list_add(una_lista_12345, &size_execute);
	list_add(una_lista_12345, &size_blocked);
	list_add(una_lista_12345, &size_exit);
	mayor_size = (int*)list_get_maximum(una_lista_12345, (void*)__encontrando_el_maximo);

	int ancho_columna = 16;
	char* borde_completo = string_repeat('-', ancho_columna*5);

	char* header = string_new();
	string_append(&header, " |===== NEW ====");
	string_append(&header, "=|==== READY ===");
	string_append(&header, "=|=== EXECUTE ==");
	string_append(&header, "=|=== BLOCKED ==");
	string_append(&header, "=|==== EXIT ====");
		 char* relleno_1 = " |              ";
//		 char* relleno_2 = "=|==============";

	char* relleno_caracteres = string_repeat('-', 69);
	printf(" [GMMP: %d] %s\n", GRADO_MULTIPROGRAMACION_INI, relleno_caracteres);
	printf("%s\n", header);

//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) \n");

//----------------------------------------------------------
	char* __traer_string_con_data(t_list* una_lista, int elemento, int size_r){
		char* respuesta_char = string_new();
		char* aux_1 = string_new();
		char* aux_2;
		char* ultimos_3;
		string_append(&respuesta_char, " | PID: ");
//		t_pcb* una_pcb = list_get(una_lista, elemento); //<
		t_pcb* una_pcb = list_get(una_lista, size_r - (elemento + 1)); //<
		aux_2 = string_itoa(una_pcb->pid);
		string_append(&aux_1, "00");
		string_append(&aux_1, aux_2); //[001|0032|00076|000981]
		int size = strlen(aux_1);
		if(size >= 3){
			ultimos_3 = malloc(4*sizeof(char));
			memcpy(ultimos_3, aux_1 + size - 3, 4*sizeof(char));
		}else{
			log_error(kernel_logger, "El size deberia se >= 3");
			exit(EXIT_FAILURE);
		}
		string_append(&respuesta_char, ultimos_3);
		string_append(&respuesta_char, " (");
		char* otro_char = string_itoa(una_pcb->estado);
//		char* otro_char = " ";
		string_append(&respuesta_char, otro_char);
		string_append(&respuesta_char, ") ");

		free(aux_2);//[12]
		free(aux_1);//[00] -> [0012]
		free(ultimos_3);//[012]
		free(otro_char);
		return respuesta_char;
	}
//----------------------------------------------------------
	char* __imprimir_linea_completa(int nro_linea){
		char* linea = string_new();
		char* char_aux;

		//Evaluando lista_new
		if(size_new > nro_linea){
			char_aux = __traer_string_con_data(lista_new, nro_linea, size_new);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_ready
		if(size_ready > nro_linea){
			char_aux = __traer_string_con_data(lista_ready, nro_linea, size_ready);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_execute
		if(size_execute > nro_linea){
			char_aux = __traer_string_con_data(lista_execute, nro_linea, size_execute);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_blocked
		if(size_blocked > nro_linea){
			char_aux = __traer_string_con_data(lista_blocked, nro_linea, size_blocked);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_exit
		if(size_exit > nro_linea){
			char_aux = __traer_string_con_data(lista_exit, nro_linea, size_exit);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}


		return linea;
	}
//----------------------------------------------------------

	int contador_linea = 0;

	while(*mayor_size > contador_linea){
		char* linea = __imprimir_linea_completa(contador_linea);
		printf("%s\n", linea);
		contador_linea++;
		free(linea);
	}


	printf("%s\n", borde_completo);

//	char* prueba_1;
//	char* prueba_2;

	free(relleno_caracteres);
	free(borde_completo);
	free(header);
	list_destroy(una_lista_12345);

}

void public_imprimir_procesos_por_estado_v0(){
	void __list_aux(t_pcb* una_pcb){
		printf("PID: %d\n", una_pcb->pid);
	}

	printf("===========================\n");
	printf("------- ESTADO NEW --------\n");
	pthread_mutex_lock(&mutex_lista_new);
	if(!list_is_empty(lista_new)){
		list_iterate(lista_new, (void*)__list_aux);
	}
	pthread_mutex_unlock(&mutex_lista_new);

	printf("===========================\n");
	printf("------- ESTADO READY ------\n");
	pthread_mutex_lock(&mutex_lista_ready);
	if(!list_is_empty(lista_ready)){
		list_iterate(lista_ready, (void*)__list_aux);
	}
	pthread_mutex_unlock(&mutex_lista_ready);

	printf("===========================\n");
	printf("------ ESTADO EXECUTE -----\n");
	pthread_mutex_lock(&mutex_lista_exec);
	if(!list_is_empty(lista_execute)){
		list_iterate(lista_execute, (void*)__list_aux);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	printf("===========================\n");
	printf("------ ESTADO BLOCKED -----\n");
	pthread_mutex_lock(&mutex_lista_blocked);
	if(!list_is_empty(lista_blocked)){
		list_iterate(lista_blocked, (void*)__list_aux);
	}
	pthread_mutex_unlock(&mutex_lista_blocked);

	printf("===========================\n");
	printf("------- ESTADO EXIT -------\n");
	pthread_mutex_lock(&mutex_lista_exit);
	if(!list_is_empty(lista_exit)){
		list_iterate(lista_exit, (void*)__list_aux);
	}
	pthread_mutex_unlock(&mutex_lista_exit);
	printf(".................................\n");
}






