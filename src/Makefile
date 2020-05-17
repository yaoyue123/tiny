#
# makefile for TINY
# Borland C Version
# K. Louden 2/3/98
#

CC = gcc

CFLAGS = 

OBJS = main.o util.o scan.o parse.o symtab.o analyze.o code.o cgen.o

tiny.exe: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o tiny.exe

main.o: main.c globals.h util.h scan.h parse.h analyze.h cgen.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

scan.o: scan.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c scan.c

parse.o: parse.c parse.h scan.h globals.h util.h
	$(CC) $(CFLAGS) -c parse.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c

code.o: code.c code.h globals.h
	$(CC) $(CFLAGS) -c code.c

cgen.o: cgen.c globals.h symtab.h code.h cgen.h
	$(CC) $(CFLAGS) -c cgen.c

clean:
	-del tiny.exe
	-del tm.exe
	-del main.o
	-del util.o
	-del scan.o
	-del parse.o
	-del symtab.o
	-del analyze.o
	-del code.o
	-del cgen.o

tm.exe: tm.c
	$(CC) $(CFLAGS) -o tm.exe tm.c

tiny: tiny.exe

tm: tm.exe

all: tiny tm

.PHONY: clean tiny tm all