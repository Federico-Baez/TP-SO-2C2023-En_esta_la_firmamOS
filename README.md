# tp-2023-2c-En_esta_la_firmamOS
## Esta posicionado en el Debug de cada modulo para correr por consola

- LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./Kernel ../kernel.config
- LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./CPU ../cpu.config
- LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./FIleSystem ../filesystem.config
- LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./Memoria ../memoria.config

## Valgrind de memoria
- >LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ valgrind --leak-check=full ./Memoria ../memoria.config

## INICIAR PROCESO EN KERNEL 
- INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/prueba_instrucciones/psudocodigos.txt 12 1

## Para ejecutar con el archivo .sh
- kernel (cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Kernel/ ):
 ./run_kernel.sh
- cpu (/home/utnso/tp-2023-2c-En_esta_la_firmamOS/CPU/):
./run_cpu.sh
- filesystem (cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/FileSystem/):
./run_filesystem.sh
- memoria (cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Memoria/)
./run_memoria.sh

##===================================

## CONFIGS
./Kernel ./configs/base.config
./Kernel ./configs/recursos.config
./Kernel ./configs/memoria.config
./Kernel ./configs/fs.config
./Kernel ./configs/integral.config
./Kernel ./configs/estres.config

./CPU ./configs/base.config
./CPU ./configs/recursos.config
./CPU ./configs/memoria.config
./CPU ./configs/fs.config
./CPU ./configs/integral.config
./CPU ./configs/estres.config

./Memoria ./configs/base.config
./Memoria ./configs/recursos.config
./Memoria ./configs/memoria.config
./Memoria ./configs/fs.config
./Memoria ./configs/integral.config
./Memoria ./configs/estres.config

./FileSystem ./configs/base.config
./FileSystem ./configs/recursos.config
./FileSystem ./configs/memoria.config
./FileSystem ./configs/fs.config
./FileSystem ./configs/integral.config
./FileSystem ./configs/estres.config



+ FUNCIONA CON FIFO, RR Y PRIORIDADES
---- PRUEBA BASE ----
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_1 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_2 64 3
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_3 64 2
INICIAR_PLANIFICACION
# Cambiar a RR y volver a ejecutar
# Cambiar a PRIORIDADES y volver a ejecutar

+ FUNCIONA
---- PRUEBA RECURSOS ----
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_A 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_B 64 3
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_C 64 2
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_D 64 2
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_1 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_2 64 3
INICIAR_PLANIFICACION
# Cuando estén todos bloqueados, finalizar DEADLOCK_A
FINALIZAR_PROCESO 1

+ FUNCIONA CON FIFO Y LRU
---- PRUEBA MEMORIA ----
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_1 128 1
INICIAR_PLANIFICACION
# Cuando finaliza, ejecutar
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_2 64 1
(ejecutar 4 veces)
INICIAR_PLANIFICACION
# Esperar al thrashing
DETENER_PLANIFICACION
FINALIZAR_PROCESO 2
FINALIZAR_PROCESO 3
FINALIZAR_PROCESO 4
FINALIZAR_PROCESO 5
MULTIPROGRAMACION 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_2 64 1
(ejecutar 4 veces)
INICIAR_PLANIFICACION



- TESTEANDO
---- PRUEBA FS ----
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_A 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_B 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_C 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_D 16 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_E 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_3 64 1 
INICIAR_PLANIFICACION

- TESTEANDO
---- PRUEBA INTEGRAL ----
INICIAR_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_A 128 10
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_B 128 5
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_C 64 1
# Esperar que solo quede ejecutando INTEGRAL_A
FINALIZAR_PROCESO 1

+ FUNCIONA
---- PRUEBA ESTRÉS ----
DETENER_PLANIFICACION
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_1 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_2 64 1
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_3 64 1
(ejecutarlo 4 veces)
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_4 256 1
INICIAR_PLANIFICACION


