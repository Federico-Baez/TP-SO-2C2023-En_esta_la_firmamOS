# TP Sistemas Operativos 2023 2do Cuatrimestre
Grupo: En_esta_la_firmamOS

Integrantes: 
+ Aiello Luca
+ Alvarez Damian
+ Baez Federico
+ Davinson Frank
+ Lizarraga Ever

Con mucho esfuerzo de parte de todo el equipo se logró. Alabado sea el sol.

[Enunciado TP MaPPA](https://docs.google.com/document/d/1g6DEcbjilpX2XUBADuF6dPnrLv5lzoTZSVYCPgpHM_Q/edit#heading=h.ta7htg1j6jba)

[Documento de pruebas](https://docs.google.com/document/d/1DkLI9mhsAWXD-r2YHdnLBWHChp0JFpVMHtYBoqfezhg/edit)

[Guía de primeros pasos para SO (muy util)](https://docs.utnso.com.ar/primeros-pasos/)

# Guia de despliegue en VM

## Levantar la VM
+ Seguir [esta guía](https://docs.utnso.com.ar/recursos/vms) para bajarse la VM de la cátedra

## Correr el tp
+ Clonar el repo en la VM
+ Abrir cuatro consolas (con abrir una y splitearla mediante las opciones del click derecho alcanza)
+ Posicionarte en el repo desde las cuatro consolas
+ En cada una abrir el modulo correspondiente(kernel, memoria, filesystem, cpu)
+ Correr el comando `make` en todos los módulos
+ Correr los siguientes comandos en el siguiente orden de módulos (el config puede ser el predeterminado o alguno de una prueba)
   + ./Memoria memoria.config
   + ./FileSystem filesystem.config
   + ./CPU cpu.config
   + ./Kernel kernel.config
+ Desde este punto todo se maneja desde kernel, donde se encontrara corriendo la consola, con HELP pueden ver todos los comandos disponibles
+ Correr alguna de las pruebas para testear el funcionamiento, en el siguiente item pueden ver un ejemplo de inicio de proceso

## Iniciar proceso desde kernel
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/<nombre_> <tamaño_> <prioridad_>
   + <nombre_> ver en el directorio mappa-pruebas los archivos disponibles (o directamente el docs de pruebas)
   + <tamaño_> ver docs de pruebas
   + <prioridad_> solo importante si el algoritmo de planificación corto plazo esta en prioridades, de lo contrario no importa el valor

## Configs
Configs para las diferentes pruebas
+ ./Kernel ./configs/base.config
+ ./Kernel ./configs/recursos.config
+ ./Kernel ./configs/memoria.config
+ ./Kernel ./configs/fs.config
+ ./Kernel ./configs/integral.config
+ ./Kernel ./configs/estres.config

+ ./CPU ./configs/base.config
+ ./CPU ./configs/recursos.config
+ ./CPU ./configs/memoria.config
+ ./CPU ./configs/fs.config
+ ./CPU ./configs/integral.config
+ ./CPU ./configs/estres.config

+ ./Memoria ./configs/base.config
+ ./Memoria ./configs/recursos.config
+ ./Memoria ./configs/memoria.config
+ ./Memoria ./configs/fs.config
+ ./Memoria ./configs/integral.config
+ ./Memoria ./configs/estres.config

+ ./FileSystem ./configs/base.config
+ ./FileSystem ./configs/recursos.config
+ ./FileSystem ./configs/memoria.config
+ ./FileSystem ./configs/fs.config
+ ./FileSystem ./configs/integral.config
+ ./FileSystem ./configs/estres.config

## Secuencia de comandos consola para las pruebas
### ---- PRUEBA BASE ----
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_1 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_2 64 3
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/PLANI_3 64 2
+ INICIAR_PLANIFICACION
+ Cambiar a RR y volver a ejecutar
+ Cambiar a PRIORIDADES y volver a ejecutar

### ---- PRUEBA RECURSOS ----
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_A 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_B 64 3
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_C 64 2
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/DEADLOCK_D 64 2
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_1 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_2 64 3
+ INICIAR_PLANIFICACION
+ Cuando estén todos bloqueados, finalizar DEADLOCK_A (FINALIZAR_PROCESO 1)

### ---- PRUEBA MEMORIA ----
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_1 128 1
+ INICIAR_PLANIFICACION
+ Cuando finaliza, ejecutar
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_2 64 1 (ejecutar 4 veces)
+ INICIAR_PLANIFICACION
+  Esperar al thrashing
+ DETENER_PLANIFICACION
+ FINALIZAR_PROCESO 2
+ FINALIZAR_PROCESO 3
+ FINALIZAR_PROCESO 4
+ FINALIZAR_PROCESO 5
+ MULTIPROGRAMACION 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/MEMORIA_2 64 1 (ejecutar 4 veces)
+ INICIAR_PLANIFICACION

### ---- PRUEBA FS ----
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_A 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_B 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_C 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_D 16 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/FS_E 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ERROR_3 64 1 
+ INICIAR_PLANIFICACION

### ---- PRUEBA INTEGRAL ----
+ INICIAR_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_A 128 10
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_B 128 5
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/INTEGRAL_C 64 1
+ Esperar que solo quede ejecutando INTEGRAL_A
+ FINALIZAR_PROCESO 1

### ---- PRUEBA ESTRÉS ----
+ DETENER_PLANIFICACION
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_1 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_2 64 1
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_3 64 1 (ejecutarlo 4 veces)
+ INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/mappa-pruebas/ESTRES_4 256 1
+ INICIAR_PLANIFICACION


