#include "../include/Kernel.h"
#include <termios.h>
#include <ncurses.h>

int main(int argc, char** argv) {

	inicializar_kernel(argv[1]);

	iniciar_conexiones();

	atender_memoria();
	atender_filesystem();
	atender_cpu_dispatch();
	atender_cpu_interrupt();

	leer_consola();

	finalizar_kernel();
	printf("TODO KERNEL SE FINALIZO CORRECTAMENTE...\n");
	return EXIT_SUCCESS;
}
