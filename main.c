/*
 *  main.c
 *  Analizador Sintactico
 *
 *  Created by Alan Rodriguez
 *  Fernando Romero
 *  Jose Jesus Perez Aguinaga
 *
 *
 */

#include "analex.h"
#include "anasin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

void runAndClean(){
	fprintf(stdout, "\n-----Ejectuando Analizador Sintactico-----\n\n");
	// Corre anasin
	anasin();
    
	// Reinicia automatas y limpia inputs
	clear(&pila);
	clearInput();
	freeInp();
	inputSize = 0;
	inputSizeLex = 0;
}

int main (int argc, const char * argv[]) {
	int i;
    /*Stack hola;
    initStack(&hola);
    for (i=0; i<53; i++) {
        push(&hola, i);
        
        if(i%3 == 0){
            pop(&hola);
        }
        
        char ret[BUFSIZ];
        
        memset(ret, '\0', BUFSIZ);
        int pilaprint[BUFSIZ];
        int k;

        despliega(&hola, pilaprint);
        
        for (k=0; k<hola.size; k++) {
            strcat(ret,itoaC(pilaprint[k]));
            strcat(ret," ");
        } 
        fprintf(stdout,"Pila: %s\n", ret);
        fprintf(stdout,"Size: %d\n\n", hola.size);
    }*/
    
    if (argc != 2) {
		fprintf(stderr, "Forma de uso: %s origen\n",argv[0]);
		return -1;
	}
	
	if((fuente=fopen(argv[1],"r")) == NULL){
		fprintf(stderr, "Error al abrir el archivo: %s",argv[1]);
		return -1;
	}
	
	if ((fdLexemas=open("lexemas.txt",O_WRONLY | O_CREAT | O_TRUNC,0666)) < 0) {
		fprintf(stderr, "Error al crear archivo lexemas.txt");
		return -1;
	}
	
	if ((fdTabla=open("tabla_simbolos.txt",O_WRONLY | O_CREAT | O_TRUNC,0666)) < 0) {
		fprintf(stderr, "Error al crear archivo tabla_simbolos.txt");
		return -1;
	}
	
	if ((fdOutput=open("output.txt",O_WRONLY | O_CREAT | O_TRUNC,0666)) < 0) {
		fprintf(stderr, "Error al crear archivo output.txt");
		return -1;
	}
	
	//Crear automatas
	
	automata automatas[cuantosAutomatas];
	automatas[0].funcion = automataComentarios;
	automatas[1].funcion = automataIdentificadores;
	automatas[2].funcion = automataOperadoresLogicos;
	automatas[3].funcion = automataOperadoresComparacion;
	automatas[4].funcion = automataOperadoresAsignacion;
	automatas[5].funcion = automataNumerosReales;
	automatas[6].funcion = automataNumeros;
	automatas[7].funcion = automataPuntuacion;
	automatas[8].funcion = automataCadenaCaracteres;
	automatas[9].funcion = automataOperadoresAgrupacion;
	automatas[10].funcion = automataAritmetico;
	
	reset(automatas);
	resetInp();
	
	//fprintf(stdout, "Dame opcion a ejecutar:");
	//scanf("%d",&eleccion);
    
	fprintf(stdout, "\n\n-----Ejectuando Analizador Lexico-----\n\n");
    
	char c;
    
	while ((c = getc(fuente)) != EOF) {
        
        if (c == '\n' || c == '\t') {
            continue;
        }
        
		// Si el automata regresa 1 es que llega a estado terminal
		
		if (automatas[0].funcion(&automatas[0],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[1].funcion(&automatas[1],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[2].funcion(&automatas[2],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[3].funcion(&automatas[3],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[4].funcion(&automatas[4],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[5].funcion(&automatas[5],c) == 1) {
			reset(automatas);
			continue;
		}
		if (automatas[6].funcion(&automatas[6],c) == 1) {
			reset(automatas);
			continue;
		}
		
		if (automatas[7].funcion(&automatas[7],c) == 1) {
			reset(automatas);
			continue;
		}
        
        if (automatas[8].funcion(&automatas[8],c) == 1) {
			reset(automatas);
			continue;
		}
        
        if (automatas[9].funcion(&automatas[9],c) == 1) {
			reset(automatas);
			continue;
		}
        
        if (automatas[10].funcion(&automatas[10],c) == 1) {
			reset(automatas);
			continue;
		}
			
	}
    
    strcpy(inputLex[inputSizeLex],"$");
    strcpy(inputRealLex[inputSizeLex],"$");
    inputSizeLex++;
    // Igualas los datos del analex con los del anasin
    inputSize = inputSizeLex;
    cuantosTokens = cuantosTokensLex;
    
    for (i=0; i<inputSizeLex; i++) {
        input[i] = inputLex[i];
        inputReal[i] = inputRealLex[i];
    }
    
    // Transferir arreglo de tokens para que el anasin lo use
    for (i=0; i<cuantosTokensLex; i++) {
        tokenTemp[i] = tokenTempLex[i];
    }
    
    // Inicializa Valores
    inicializaGramatica();
    // Inicializa Pila
    initStack(&pila);
    
    // Correr anasin
    fprintf(stdout, "\n-----Ejectuando Analizador Sintactico-----\n\n");
    anasin();
    fprintf(stdout, "\n");
	imprimeTokens();
    reset(automatas);
    clearInput();
    clear(&pila);
    inputSize = inputSizeLex = 0;
    // Si hay error salir
    if (errorSintactico == 1) {
        clear(&pila);
        return EXIT_SUCCESS;
    }

	
	fclose(fuente);
	
    return 0;
}

