PROCESS_NAME=FileSystem
IDIR =./include
SHARED_IDIR = ../Shared/include
CC=gcc
CFLAGS=-I$(IDIR) -I$(SHARED_IDIR) -g -Wall

ODIR=./obj
SRCDIR =./src
LIBS=-lcommons -lpthread -lreadline -lcunit -lrt

SHARED_SRCDIR = ../Shared/src
SHARED_ODIR = ../Shared/obj

DEPS = $(wildcard $(IDIR)/*.h)

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(ODIR)/%.o,$(SRC))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) $(SHARED_DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SHARED_ODIR)/%.o: $(SHARED_SRCDIR)/%.c $(SHARED_DEPS)
	mkdir -p $(SHARED_ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

SHARED_DEPS = $(wildcard $(SHARED_IDIR)/*.h)

SHARED_SRC = $(wildcard $(SHARED_SRCDIR)/*.c)
SHARED_OBJ = $(patsubst $(SHARED_SRCDIR)/%.c,$(SHARED_ODIR)/%.o,$(SHARED_SRC))

$(PROCESS_NAME): $(OBJ) $(SHARED_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o~ core $(INCDIR)/*~ $(PROCESS_NAME) $(SHARED_ODIR)/*.o
## Globales
#CC = gcc
#OUTFILE = CPU.out
#
#### Reglas ###
#
#SRC = $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
#OBJ = $(SRC:%.c=./obj/%.o)
#
#
#SHARED = $(shell find ../Shared/obj/*.o)
#
#build: makefolder makeshared $(OBJ)
#	$(CC) -o $(OUTFILE) cpu.c $(OBJ) $(SHARED)
#
#valgrind: build
#	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./$(OUTFILE)
#
#LINK = -lcommons -lpthread -lsocket -lreadline -lcunit -lrt -lm -lprotocolo -lshared
#CFLAGS = -c
#
#debug: CFLAGS := $(CFLAGS) -g
#debug: makeshared_debug build
#
#./obj/%.o: ./src/%.c
#	$(CC) $(CFLAGS) $< -o $@ $(LINK)
#
#makefolder:
#	if [ ! -d ./obj ]; then mkdir obj; fi
#
#makeshared:
#	make -C ../Shared
#
#makeshared_debug:
#	make debug -C ../Shared
#
## Clean
#
#clean:
#	rm -rf obj
#	rm -rf *.out
#	rm -rf *.log
#	make clean -C ../Shared