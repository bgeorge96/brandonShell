# Makefile for shell.

CC=gcc
MPI=mpicc
TAR=tar
CFLAGS=-Wall

.PHONY: all clean

clean:
	$(RM) shell

shell: shell.c
	$(CC) $(CFLAGS) -o $@ $^

all: shell
