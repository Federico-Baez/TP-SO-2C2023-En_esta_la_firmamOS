#include "../include/marcos.h"

/*Esto ocurre durante la creacion de algun proceso,
 * se le asignan un marco para cada pagina*/
void asignar_marcos_a_cada_pagina(proceso_recibido* un_proceso){
	int cant_paginas = list_size(un_proceso->tabla_paginas);
	for(int i=0; i<cant_paginas; i++){
		Pagina* una_pagina = list_get(un_proceso->tabla_paginas, i);
		marco* un_marco = pedir_un_marco_de_la_lista_de_marcos();
		una_pagina->ptr_marco = un_marco;
		una_pagina->marco = un_marco->pid;
		una_pagina->orden_carga = ordenCargaGlobal; ordenCargaGlobal++;
		una_pagina->presente = true;
		una_pagina->modificado = false;

		if (strcmp(ALGORITMO_REEMPLAZO, "LRU") == 0) {
			una_pagina->ultimo_uso = temporal_create();
		}

	}
}

/*Te devuelve un Marco disponible y si ya no hay llama
 * al algoritmo para elegir una victima y reemplazarla*/
marco* pedir_un_marco_de_la_lista_de_marcos(){
	//
	bool _marco_libre(marco* un_marco){
		return un_marco->libre;
	}
	marco* el_marco = list_find(lst_marco, (void*)_marco_libre);

	//Asumo que si no encuentra ninguno libre, retorna NULL (Lo vi en las commons)
	if(el_marco == NULL){
		//Desplegar el algoritmo de eliminacion
		if(strcmp(ALGORITMO_REEMPLAZO, "FIFO") == 0){
			el_marco = elegir_vcitima_FIFO();
		}
		else {
			el_marco = elegir_victima_LRU();
		}

		el_marco->ptr_pagina->presente = false;

		//Compruebo si fue modificado y guardo los cambios en SWAP
		if(el_marco->ptr_pagina->modificado){
			guardar_marco_en_swap(el_marco);
			el_marco->ptr_pagina->modificado = false;
		}
	}

	el_marco->libre = false;
	return el_marco;

}

void guardar_marco_en_swap(marco* un_marco){
	void* contenido_del_marco = malloc(TAM_PAGINA);
	memcpy(contenido_del_marco, espacio_usuario + un_marco->base, TAM_PAGINA);

	t_paquete* un_paquete = crear_super_paquete(GUARDAR_MARCO_EN_SWAP_FM);
	cargar_int_al_super_paquete(un_paquete, un_marco->ptr_pagina->pos_en_swap);
	cargar_choclo_al_super_paquete(un_paquete, contenido_del_marco, TAM_PAGINA);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}


marco* elegir_vcitima_FIFO(){
	marco* _comparar_orden_carga(marco* marco1, marco* marco2) {
	    if (marco1->ptr_pagina->orden_carga < marco2->ptr_pagina->orden_carga){
	    	return marco1;
	    }
	    else return marco2;
	}
	marco* marco_a_reemplazar = list_get_minimum(lst_marco, (void*)_comparar_orden_carga);
	return marco_a_reemplazar;
}

marco* elegir_victima_LRU(){
	marco* _comparar_acceso_LRU(marco* marco1, marco* marco2) {
		if (temporal_gettime(marco1->ptr_pagina->ultimo_uso) > temporal_gettime(marco2->ptr_pagina->ultimo_uso)){
	    	return marco1;
	    }
	    else return marco2;
	}
	marco* marco_a_reemplazar = list_get_maximum(lst_marco, (void*)_comparar_acceso_LRU);
	return marco_a_reemplazar;
}








