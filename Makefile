CFLAGS=-c -Wall

# chmod a+x MAKEFILE
cparser: analex.h anasin.h main.c
	gcc main.c -o cparser $(CFLAGS)