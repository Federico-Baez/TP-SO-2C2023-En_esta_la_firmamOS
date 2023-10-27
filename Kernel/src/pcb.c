#include "../include/pcb.h"

/*Convierte el valor del estado a texto
 * (Retorna NULL si no detecta algun estado conocido)*/
static char* _convertir_estado_pcb_a_texto(int un_valor){
	char* nombre_del_estado;
	switch (un_valor) {
		case NEW:
			nombre_del_estado = "NEW";
			break;
		case READY:
			nombre_del_estado = "READY";
			break;
		case EXEC:
			nombre_del_estado = "EXECUTE";
			break;
		case BLOCKED:
			nombre_del_estado = "BLOCKED";
			break;
		case EXIT:
			nombre_del_estado = "EXIT";
			break;
		default:
			nombre_del_estado = NULL;
			log_error(kernel_logger, "No se reconocio el nombre del estado");
			break;
	}
	if(nombre_del_estado != NULL){
		return string_duplicate(nombre_del_estado);
	}else{
		return nombre_del_estado;
	}
}

t_pcb* crear_pcb(char* path, char* size, char* prioridad){
	t_pcb* nueva_PCB = malloc(sizeof(t_pcb));
	pthread_mutex_lock(&mutex_process_id);
	process_id++;
	nueva_PCB->pid = process_id;
	pthread_mutex_unlock(&mutex_process_id);
	nueva_PCB->program_counter = 0;

	nueva_PCB->size = atoi(size);
	nueva_PCB->prioridad = atoi(prioridad);
	nueva_PCB->path = string_duplicate(path);
//	nueva_PCB->path = malloc(sizeof(char)*(strlen(path) + 1));
//	memcpy(nueva_PCB->path, path, sizeof(char)*(strlen(path) + 1));

	nueva_PCB->lista_recursos_pcb = list_create();

	nueva_PCB->registros_CPU = malloc(sizeof(t_registros_CPU));
	nueva_PCB->registros_CPU->AX = 0;
	nueva_PCB->registros_CPU->BX = 0;
	nueva_PCB->registros_CPU->CX = 0;
	nueva_PCB->registros_CPU->DX = 0;

	return nueva_PCB;
}


void destruir_pcb(t_pcb* una_PCB){
	free(una_PCB->path);
	free(una_PCB->registros_CPU);
	list_destroy(una_PCB->lista_recursos_pcb);
	free(una_PCB);
}

void imprimir_pcb(t_pcb* una_PCB){
	log_info(kernel_logger, "<PCB_%d> [%s][%d][%d]",
							una_PCB->pid,
							una_PCB->path,
							una_PCB->size,
							una_PCB->prioridad);

}

void imprimir_pcb_v2(t_pcb* una_pcb){
	char* string_estado = _convertir_estado_pcb_a_texto(una_pcb->estado);
	log_info(kernel_logger, "<PCB>[PID:%d][%s][]",
							una_pcb->pid,
							string_estado);
	free(string_estado);
}

/*Busca la PCB por el PID pasado por parametro
 * (Devuleve NULL si no encusntra el PID en ninguna lista)*/
t_pcb* buscar_pcb_por_pid(int un_pid){
	t_pcb* una_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_new);
		if(list_any_satisfy(lista_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			una_pcb = list_find(lista_new, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_new);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_ready);
		if(list_any_satisfy(lista_ready, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			una_pcb = list_find(lista_ready, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_ready);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exec);
		if(list_any_satisfy(lista_execute, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			una_pcb = list_find(lista_execute, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exec);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exit);
		if(list_any_satisfy(lista_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			una_pcb = list_find(lista_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exit);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_blocked);
		if(list_any_satisfy(lista_blocked, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			una_pcb = list_find(lista_blocked, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_blocked);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		una_pcb = NULL;
		log_error(kernel_logger, "PID no encontrada en ninguna lista");
	}

	return una_pcb;
}

/*Para llamar a esta  funcion necesitas protegerla con MUTEX segun la lista*/
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* una_pcb){
	int var_aux = 0;
	void __buscar_pcb_exacta(t_pcb* void_pcb){
		if(void_pcb == una_pcb){
			var_aux++;
		}
	}

	list_iterate(una_lista, (void*)__buscar_pcb_exacta);
	if(var_aux != 0){
		return true;
	}else{
		return false;
	}


}




