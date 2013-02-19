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
	
    if (argc != 2) {
		fprintf(stderr, "Forma de uso: %s origen\n",argv[0]);
		return -1;
	}
	
	if((fuente=fopen(argv[1],"r")) == NULL){
		fprintf(stderr, "Error al abrir el archivo: %s",argv[1]);
		return -1;
	}
	
	if((tablaReglasFuente=fopen("TABLA_SLR.txt","r")) == NULL){
		fprintf(stderr, "Error al abrir el archivo TABLA_SLR.txt");
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
	
    inicializaTemp();
    
	char c;
	/*char aux[20], aux1[10], aux2[10], aux3[10], aux4[10];
	char a1[]="S", a2[]="CASE", a3[]="E", a4[]="NOP", a5[]="AOP", a6[]="CLASE", a7[]="F", a8[]="ID";
	char b1[]="num", b2[]=":", b3[]="elemento", b4[]="*", b5[]="n", b6[]=".", b7[]="identificador", b8[]="#", b9[]="|", b10[]="-", b11[]="$";
	int i = 0, j = 0, k = 0, y = 0, n = 0, n1 = 1, n2 = 2, n3 = 3, n4 = 4, n5 = 5, n6 = 6, n7 = 7, n8 = 8, n9 = 9, n10 = 10, n11 = 11, n12 = 12;
	int n13 = 13, n14 = 14, n15 = 15, n16 = 16, n17 = 17, n18 = 18, n19 = 19, n20 = 20, n21 = 21, n22 = 22, n23 = 23, n24 = 24, n25 = 25, n26 = 26, n27 = 27;*/
	
	//Lectura del archivo TABLA_SLR.txt
	/*while ((c = getc(tablaReglasFuente)) != EOF) {
		
		for(y=0; y<10; y=y+1)aux1[y]='\0';
		// Lee entero de no terminales
		while((c != '\n') && (c != EOF)){
			aux1[i] = c;
			c = getc(tablaReglasFuente);
			i++;
		}
		// cast a int
		noTerminales = atoi(aux1);
		c = getc(tablaReglasFuente);
		i = 0;
		
		// Lee entero de terminales
		for(y=0; y<10; y=y+1)aux2[y]='\0';
		while((c != '\n') && (c != EOF)){
			aux2[i] = c;
			c = getc(tablaReglasFuente);
			i++;
		}
		// cast a int
		terminales = atoi(aux2);
		c = getc(tablaReglasFuente);
		i = 0;
		
		for(y=0; y<10; y=y+1)aux3[y]='\0';
		// Lee entero de estados
		while((c != '\n') && (c != EOF)){
			aux3[i] = c;
			c = getc(tablaReglasFuente);
			i++;
		}
		// cast a int
		estados = atoi(aux3);
		c = getc(tablaReglasFuente);
		i = 0;
		
		// Leo entero de derivacionesGramatica
		for(y=0; y<10; y=y+1)aux4[y]='\0';
		while((c != '\n') && (c != EOF)){
			aux4[i] = c;
			c = getc(tablaReglasFuente);
			i++;
		}
		// cast a int
		derivacionesGramatica = atoi(aux4);
		//printf("derivacionesGramatica: %i\n", derivacionesGramatica);
		
		// Arreglo siguientes
		siguientes = malloc(sizeof(siguiente)*terminales);
		// Arreglo gramatica
		gramatica = malloc(sizeof(derivacion)*derivacionesGramatica);
		// Definir el tamaño de la matriz (tabla)
		tablaR = malloc(sizeof(regla)*estados*10);
		int row;
		for (row=0; row<=estados; row++) {
			tablaR[row] = malloc(sizeof(regla) * (terminales + noTerminales + 1));
			//tablaR[row] = malloc(sizeof(regla) * (estados + 1));
		}
		// Inicializa la tabla con valores default
		inicializaTabla(tablaR);
		c = getc(tablaReglasFuente);
		i = 0;
		
		for(y=0; y<20; y=y+1)aux[y]='\0';
		for(y=0; y<8; y=y+1)arregloNoTerminales[y]='\0';
		//Lectura de los elementos No Terminales
		while((c != '\n') && (c != EOF)) {
			if(c != ',') {
				aux[n] = c;
				n++;
				c = getc(tablaReglasFuente);
			} else {
				c = getc(tablaReglasFuente);
				if(strcmp(aux,a1) == 0) {
					arregloNoTerminales[0] = a1;
				} else if(strcmp(aux,a2) == 0) {
					arregloNoTerminales[1] = a2;
				} else if(strcmp(aux,a3) == 0) {
					arregloNoTerminales[2] = a3;
				} else if(strcmp(aux,a4) == 0) {
					arregloNoTerminales[3] = a4;
				} else if(strcmp(aux,a5) == 0) {
					arregloNoTerminales[4] = a5;
				} else if(strcmp(aux,a6) == 0) {
					arregloNoTerminales[5] = a6;
				} else if(strcmp(aux,a7) == 0) {
					arregloNoTerminales[6] = a7;
				} else if(strcmp(aux,a8) == 0) {
					arregloNoTerminales[7] = a8;
				}
				n = 0;
				for(y=0; y<20; y=y+1)aux[y]='\0';
			}
		}
		n = 0;
		c = getc(tablaReglasFuente);
		
		//Lectura de los Siguientes de los elementos de la gramatica
		while(c != '\n') {
			if(c != ',') {
				aux[n] = c;
				n++;
				c = getc(tablaReglasFuente);
			} else {
				c = getc(tablaReglasFuente);
				if(isdigit(aux[0])) {
					if(atoi(aux) == n1) {
						siguientes[j].cuantos = n1;
					} else if(atoi(aux) == n2) {
						siguientes[j].cuantos = n2;
					} else if(atoi(aux) == n3) {
						siguientes[j].cuantos = n3;
					} else if(atoi(aux) == n4) {
						siguientes[j].cuantos = n4;
					} else if(atoi(aux) == n5) {
						siguientes[j].cuantos = n5;
					} else if(atoi(aux) == n6) {
						siguientes[j].cuantos = n6;
					}
					j++;
					k = 0;
				} else {
					if(strcmp(aux,b1) == 0) {
						siguientes[j].elemento[k] = b1;
					} else if(strcmp(aux,b2) == 0) {
						siguientes[j].elemento[k] = b2;
					} else if(strcmp(aux,b3) == 0) {
						siguientes[j].elemento[k] = b3;
					} else if(strcmp(aux,b4) == 0) {
						siguientes[j].elemento[k] = b4;
					} else if(strcmp(aux,b5) == 0) {
						siguientes[j].elemento[k] = b5;
					} else if(strcmp(aux,b6) == 0) {
						siguientes[j].elemento[k] = b6;
					} else if(strcmp(aux,b7) == 0) {
						siguientes[j].elemento[k] = b7;
					} else if(strcmp(aux,b8) == 0) {
						siguientes[j].elemento[k] = b8;
					} else if(strcmp(aux,b9) == 0) {
						siguientes[j].elemento[k] = b9;
					} else if(strcmp(aux,b10) == 0) {
						siguientes[j].elemento[k] = b10;
					} else if(strcmp(aux,b11) == 0) {
						siguientes[j].elemento[k] = b11;
					}
					k++;
				}
				n = 0;
				for(y=0; y<20; y=y+1)aux[y]='\0';
			}
		}
		
		n = 0;
		c = getc(tablaReglasFuente);
		for(y=0; y<10; y=y+1)arregloTerminales[y]='\0';
		
		//Lectura de los elementos Terminales
		while((c != '\n') && (c != EOF)) {
			if(c != ',') {
				aux[n] = c;
				n++;
				c = getc(tablaReglasFuente);
			} else {
				c = getc(tablaReglasFuente);
				if(strcmp(aux,b1) == 0) {
					arregloTerminales[0] = b1;
				} else if(strcmp(aux,b2) == 0) {
					arregloTerminales[1] = b2;
				} else if(strcmp(aux,b3) == 0) {
					arregloTerminales[2] = b3;
				} else if(strcmp(aux,b4) == 0) {
					arregloTerminales[3] = b4;
				} else if(strcmp(aux,b5) == 0) {
					arregloTerminales[4] = b5;
				} else if(strcmp(aux,b6) == 0) {
					arregloTerminales[5] = b6;
				} else if(strcmp(aux,b7) == 0) {
					arregloTerminales[6] = b7;
				} else if(strcmp(aux,b8) == 0) {
					arregloTerminales[7] = b8;
				} else if(strcmp(aux,b9) == 0) {
					arregloTerminales[8] = b9;
				} else if(strcmp(aux,b10) == 0) {
					arregloTerminales[9] = b10;
				} else if(strcmp(aux,b11) == 0) {
					arregloTerminales[10] = b11;
				}
				n = 0;
				for(y=0; y<20; y=y+1)aux[y]='\0';
			}
		}
		
		n = 0;
		j = 0;
		k = 0;
		c = getc(tablaReglasFuente);
		for(y=0; y<20; y=y+1)aux[y]='\0';
		
		//Lectura de la gramatica del lenguaje
		while(c != '\n') {
			if(c != ',') {
				aux[n] = c;
				n++;
				c = getc(tablaReglasFuente);
			} else {
				c = getc(tablaReglasFuente);
				if(isdigit(aux[0])) {
					if(atoi(aux) == n1) {
						gramatica[j].derivaciones = n1;
					} else if(atoi(aux) == n2) {
						gramatica[j].derivaciones = n2;
					} else if(atoi(aux) == n3) {
						gramatica[j].derivaciones = n3;
					} else if(atoi(aux) == n4) {
						gramatica[j].derivaciones = n4;
					} else if(atoi(aux) == n5) {
						gramatica[j].derivaciones = n5;
					} else if(atoi(aux) == n6) {
						gramatica[j].derivaciones = n6;
					}
					//printf("NumG %i\n", gramatica[j].derivaciones);
					j++;
					k = 0;
				} else {
					if(strcmp(aux,a1) == 0) {
						gramatica[j].cadenaDerivacion[k] = a1;
					} else if(strcmp(aux,a2) == 0) {
						gramatica[j].cadenaDerivacion[k] = a2;
					} else if(strcmp(aux,a3) == 0) {
						gramatica[j].cadenaDerivacion[k] = a3;
					} else if(strcmp(aux,a4) == 0) {
						gramatica[j].cadenaDerivacion[k] = a4;
					} else if(strcmp(aux,a5) == 0) {
						gramatica[j].cadenaDerivacion[k] = a5;
					} else if(strcmp(aux,a6) == 0) {
						gramatica[j].cadenaDerivacion[k] = a6;
					} else if(strcmp(aux,a7) == 0) {
						gramatica[j].cadenaDerivacion[k] = a7;
					} else if(strcmp(aux,a8) == 0) {
						gramatica[j].cadenaDerivacion[k] = a8;
					} else if(strcmp(aux,b1) == 0) {
						gramatica[j].cadenaDerivacion[k] = b1;
					} else if(strcmp(aux,b2) == 0) {
						gramatica[j].cadenaDerivacion[k] = b2;
					} else if(strcmp(aux,b3) == 0) {
						gramatica[j].cadenaDerivacion[k] = b3;
					} else if(strcmp(aux,b4) == 0) {
						gramatica[j].cadenaDerivacion[k] = b4;
					} else if(strcmp(aux,b5) == 0) {
						gramatica[j].cadenaDerivacion[k] = b5;
					} else if(strcmp(aux,b6) == 0) {
						gramatica[j].cadenaDerivacion[k] = b6;
					} else if(strcmp(aux,b7) == 0) {
						gramatica[j].cadenaDerivacion[k] = b7;
					} else if(strcmp(aux,b8) == 0) {
						gramatica[j].cadenaDerivacion[k] = b8;
					} else if(strcmp(aux,b9) == 0) {
						gramatica[j].cadenaDerivacion[k] = b9;
					} else if(strcmp(aux,b10) == 0) {
						gramatica[j].cadenaDerivacion[k] = b10;
					} else if(strcmp(aux,b11) == 0) {
						gramatica[j].cadenaDerivacion[k] = b11;
					}
					k++;
				}
				n = 0;
				for(y=0; y<20; y=y+1)aux[y]='\0';
			}
		}
		
		n = 0;
		j = 0;
		k = 0;
		c = getc(tablaReglasFuente);
		for(y=0; y<20; y=y+1)aux[y]='\0';
		
		//Lectura de la TABLA SLR(1)
		while(c != EOF) {
			if(c != ',') {
				aux[n] = c;
				n++;
				c = getc(tablaReglasFuente);
			} else {
				c = getc(tablaReglasFuente);
				if(strcmp(aux, "X") == 0) {
					k++;
				} else {
					if(isdigit(aux[0])) {
						if(atoi(aux) == n1) {
							tablaR[j][k].valor = n1;
						} else if(atoi(aux) == n2) {
							tablaR[j][k].valor = n2;
						} else if(atoi(aux) == n3) {
							tablaR[j][k].valor = n3;
						} else if(atoi(aux) == n4) {
							tablaR[j][k].valor = n4;
						} else if(atoi(aux) == n5) {
							tablaR[j][k].valor = n5;
						} else if(atoi(aux) == n6) {
							tablaR[j][k].valor = n6;
						} else if(atoi(aux) == n7) {
							tablaR[j][k].valor = n7;
						} else if(atoi(aux) == n8) {
							tablaR[j][k].valor = n8;
						} else if(atoi(aux) == n9) {
							tablaR[j][k].valor = n9;
						} else if(atoi(aux) == n10) {
							tablaR[j][k].valor = n10;
						} else if(atoi(aux) == n11) {
							tablaR[j][k].valor = n11;
						} else if(atoi(aux) == n12) {
							tablaR[j][k].valor = n12;
						} else if(atoi(aux) == n13) {
							tablaR[j][k].valor = n13;
						} else if(atoi(aux) == n14) {
							tablaR[j][k].valor = n14;
						} else if(atoi(aux) == n15) {
							tablaR[j][k].valor = n15;
						} else if(atoi(aux) == n16) {
							tablaR[j][k].valor = n16;
						} else if(atoi(aux) == n17) {
							tablaR[j][k].valor = n17;
						} else if(atoi(aux) == n18) {
							tablaR[j][k].valor = n18;
						} else if(atoi(aux) == n19) {
							tablaR[j][k].valor = n19;
						} else if(atoi(aux) == n20) {
							tablaR[j][k].valor = n20;
						} else if(atoi(aux) == n21) {
							tablaR[j][k].valor = n21;
						} else if(atoi(aux) == n22) {
							tablaR[j][k].valor = n22;
						} else if(atoi(aux) == n23) {
							tablaR[j][k].valor = n23;
						} else if(atoi(aux) == n24) {
							tablaR[j][k].valor = n24;
						} else if(atoi(aux) == n25) {
							tablaR[j][k].valor = n25;
						} else if(atoi(aux) == n26) {
							tablaR[j][k].valor = n26;
						} else if(atoi(aux) == n27) {
							tablaR[j][k].valor = n27;
						}
						k++;
					} else {
						if(strcmp(aux, "NUM") == 0) {
							tablaR[j][k].tipo = NUM;
						} else if(strcmp(aux, "D") == 0) {
							tablaR[j][k].tipo = D;
						} else if(strcmp(aux, "R") == 0) {
							tablaR[j][k].tipo = R;
						} else if(strcmp(aux, "ACEPTA") == 0) {
							tablaR[j][k].tipo = ACEPTA;
						} else if(strcmp(aux, "ERR") == 0) {
							tablaR[j][k].valor = ERR;
						}
					}
				}
				if(c == '\n') {
					k = 0;
					j++;
					c = getc(tablaReglasFuente);
				}
				n = 0;
				for(y=0; y<20; y=y+1)aux[y]='\0';
			}
		}
	}*/
    
	while ((c = getc(fuente)) != EOF) {
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
		
		if (c == '\n') {
			strcpy(inputLex[inputSizeLex],"$");
			strcpy(inputRealLex[inputSizeLex],"$");
			inputSizeLex++;
			// Igualas los datos del analex con los del anasin
			inputSize = inputSizeLex;
			int i;
			for (i=0; i<inputSizeLex; i++) {
				input[i] = inputLex[i];
				inputReal[i] = inputRealLex[i];
			}
            
			// Correr anasin
			//runAndClean();
			fprintf(stdout, "\n-----Ejectuando Analizador Sintactico-----\n\n");
			anasin();
			fprintf(stdout, "\n");
			reset(automatas);
			clearInput();
			clear(&pila);
			inputSize = inputSizeLex = 0;
			// Si hay error salir
			if (errorSintactico == 1) {
                clear(&pila);
				return EXIT_SUCCESS;
			}
		}
	}
	
	
	fclose(fuente);
	fclose(tablaReglasFuente);
	
    return 0;
}

