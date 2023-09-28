#include "../include/Kernel.h"

int main(int argc, char** argv) {
	kernel_logger = log_create("kernel.log", "[Kernel]", 1, LOG_LEVEL_INFO);
	kernel_log_obligatorio = log_create("kernel_log_obligatorio.log", "[Kernel - Log obligatorio]", 1, LOG_LEVEL_INFO);

	kernel_config = config_create(argv[1]); //Esto quiza lo descomentemos para las pruebas
	//kernel_config = config_create("kernel.config");

	if(kernel_config == NULL){
		log_error(kernel_logger, "No se encontro el path del config\n");
		config_destroy(kernel_config);
		log_destroy(kernel_logger);
		log_destroy(kernel_log_obligatorio);
		exit(2);
	}

	leer_config(kernel_config);


	//pruebo con conectarme a cpu
	fd_cpu_dispatcher = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* paquete = crear_paquete();
	int numero = 7321;
	agregar_a_paquete(paquete, &numero, sizeof(int));
	enviar_paquete(paquete, fd_memoria);
	eliminar_paquete(paquete);

	//Pruebas de nuevas funcionalidades practicas de serializacion
	op_code nuermo_de_operacion = PRUEBAS;
	t_paquete* paquete2 = crear_super_paquete(nuermo_de_operacion);

	int valor1 = 25;
	int valor2 = 32;
	char* myString = "Ever Lizarraga";
	char* unchoclo = "Esto es un choclo";

	log_info(kernel_logger, "int: %d | char*: %s | char*: %s | int: %d ", valor1, myString, unchoclo, valor2);

	cargar_int_al_super_paquete(paquete2, valor1);
	cargar_string_al_super_paquete(paquete2, myString);
	cargar_choclo_al_super_paquete(paquete2, unchoclo, sizeof(char)*(strlen(unchoclo)+1));
	cargar_int_al_super_paquete(paquete2, valor2);
	enviar_paquete(paquete2, fd_filesystem);
	eliminar_paquete(paquete2);

	finalizar_kernel();

	return EXIT_SUCCESS;
}


void leer_config(t_config* config){
	IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	IP_FILESYSTEM = config_get_string_value(config,"IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(config,"PUERTO_FILESYSTEM");
	IP_CPU = config_get_string_value(config,"IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
	char* algoritmo_planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	asignar_planificador_cp(algoritmo_planificacion);
	QUANTUM = config_get_int_value(config,"QUANTUM");
	RECURSOS = config_get_array_value(config, "RECURSOS");
	INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS"); //TODO: tratar de convertirlo en un array de ints
	GRADO_MULTIPROGRAMACION_INI = config_get_int_value(config,"GRADO_MULTIPROGRAMACION_INI");

}

void finalizar_kernel(){
	log_destroy(kernel_logger);
	log_destroy(kernel_log_obligatorio);
	config_destroy(kernel_config);
	liberar_conexion(fd_cpu_dispatcher);
	liberar_conexion(fd_cpu_interrupt);
	liberar_conexion(fd_filesystem);
	liberar_conexion(fd_memoria);
	free(INSTANCIAS_RECURSOS);
	free(RECURSOS);
}

void asignar_planificador_cp(char* algoritmo_planificacion){
	if (strcmp(algoritmo_planificacion, "FIFO") == 0) {
			ALGORITMO_PLANIFICACION = FIFO;
		} else if (strcmp(algoritmo_planificacion, "RR") == 0) {
			ALGORITMO_PLANIFICACION = ROUNDROBIN;
		} else if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
			ALGORITMO_PLANIFICACION = PRIORIDADES;
		} else {
			log_error(kernel_logger, "No se encontro el algoritmo de planificacion de corto plazo");
		}
}


//TODO: Tira error al compilar
/*bool conectarse_a_modulos(){
	pthread_t conexion_fs;
	pthread_t conexion_cpu_dispatcher;
	pthread_t conexion_cpu_interrupt;
	pthread_t conexion_memoria;

	fd_cpu_dispatcher = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	//TODO: EN EL CASO DE TENER UNA INTERRUPCION SE DEBE DE CREAR OTRA CONEXION?
	pthread_create(&conexion_cpu_dispatcher, NULL, (void*)procesar_conexion, (void*) &fd_cpu_dispatcher);
	pthread_create(&conexion_cpu_interrupt, NULL, (void*)procesar_conexion, (void*) &fd_cpu_interrupt);

	pthread_detach(conexion_cpu_dispatcher);
	pthread_detach(conexion_cpu_interrupt);


	fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	pthread_create(&conexion_fs, NULL, (void*)procesar_conexion, (void*) &fd_filesystem);
	pthread_detach(conexion_fs);


	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	return fd_cpu_dispatcher != -1 && fd_cpu_interrupt != -1 && fd_filesystem != -1 && fd_memoria != -1;

}

void procesar_conexion(){}*/





