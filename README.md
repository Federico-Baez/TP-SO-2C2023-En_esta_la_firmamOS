# tp-2023-2c-En_esta_la_firmamOS

#Esta posicionado en el Debug de cada modulo para correr por consola

LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./Kernel ../kernel.config
LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./CPU ../cpu.config
LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./FIleSystem ../filesystem.config
LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ ./Memoria ../memoria.config

#Valgrind de memoria
LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ valgrind --leak-check=full ./Memoria ../memoria.config

#INICIAR PROCESO EN KERNEL 
INICIAR_PROCESO /home/utnso/tp-2023-2c-En_esta_la_firmamOS/prueba_instrucciones/psudocodigos.txt 12 1

#Para ejecutar con el archivo .sh
kernel:
cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Kernel/ 
./run_kernel.sh
cpu:
cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/CPU/ 
./run_cpu.sh
filesystem:
cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/FileSystem/
./run_filesystem.sh
memoria:
cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Memoria/
./run_memoria.sh
