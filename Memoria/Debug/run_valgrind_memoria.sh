#!/bin/bash

export LD_LIBRARY_PATH=/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/
valgrind --leak-check=full ./Memoria ../memoria.config
