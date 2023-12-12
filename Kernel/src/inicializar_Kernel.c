#include "../include/inicializar_Kernel.h"

#include <termios.h>
#include <ncurses.h>

int main(int argc, char** argv) {

	inicializar_kernel(argv[1]);

	iniciar_conexiones();

	atender_memoria();
	atender_filesystem();
	atender_cpu_dispatch();
	atender_cpu_interrupt();
	ejecutar_en_un_hilo_nuevo_detach(deteccion_deadlock, NULL);


	leer_consola();

	finalizar_kernel();
	printf("TODO KERNEL SE FINALIZO CORRECTAMENTE...\n");
	return EXIT_SUCCESS;
}
