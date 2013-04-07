CFLAGS=-Wall

# chmod a+x MAKEFILE
cparser: main.c
	gcc analex.h anasin.h main.c -o cparser $(CFLAGS)