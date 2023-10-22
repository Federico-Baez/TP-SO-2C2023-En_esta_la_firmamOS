#!/bin/bash

# Cambia al directorio Debug
cd "$(dirname "$0")/Debug"

# Ejecuta el comando
LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/ valgrind --leak-check=full ./Memoria ../memoria.config
