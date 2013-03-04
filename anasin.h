/*
 *  anasin.h
 *  Analizador Sintactico
 *
 *  Created by Alan Rodriguez
 *  Fernando Romero
 *  Jose Jesus Perez Aguinaga
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "stack.c"
#define eq(str1, str2) (strcmp(str1, str2) == 0)
#define ERR -1
#define D 0
#define R 1
#define NUM 2
#define ACEPTA 3


/* Estructura regla, contiene una regla de la tabla
 * tipo  = el tipo de regla
 * valor = el valor o estado al que apunta
 **/
typedef struct regla {
	// tipo de regla ERR, D, NUM, ACEPTA
	int tipo;
	// valor siempre es un numero que apunta a un estado
	int valor;
}regla;

/* Estructura derivacion, contiene una derivación de la tabla
 * cadenaDerivacion = cada posicion de este arreglo es un elemento de la derivacion
 * derivaciones = cuantas derivaciones
 **/
typedef struct derivacion {
	// Contiene la cadena de derivacion (arreglo de strings)
	char *cadenaDerivacion[20];
	//cantidad de derivaciones
	int derivaciones;
}derivacion;

/* Estructura siguientes, contiene una los valores siguientes de un no terminal
 * elemento = cada posicion de este arreglo es un elemento
 * cuantos = cuantos elementos
 **/
typedef struct siguiente {
	// Contiene los elemento
	char *elemento[10];
	//cantidad de elementos
	int cuantos;
}siguiente;

char* itoa(int i){
    int n = snprintf(NULL, 0, "%d", i) + 1;
    char *s = (char *)malloc(n);
    
    if (s != NULL)
        snprintf(s, n, "%d", i);
    return s;
}

FILE * tablaReglasFuente;
// Globales de cantidades
int noTerminales;
int terminales;
int estados;
int derivacionesGramatica; // Cuantas derivaciones en la gramatica
// Arreglo con valores de terminales y no terminales en string
char *arregloTerminales[34];
char *arregloNoTerminales[27];
int arregloEstados[30];
char buf[BUFSIZ];
extern char escritura[BUFSIZ];
char *input[BUFSIZ]; // La traduccion, si es un no terminal o un terminal, por ejemplo VAR_TYPE
char *inputReal[BUFSIZ]; // El buffer tal cual (los valores)
int inputSize;
int errorSintactico = 0;
// Tabla de analisis SLR1
regla **tablaR;
derivacion *gramatica;
siguiente *siguientes;
Stack pila;

Stack anidados;
int fdOutput,eleccion;


/**
 * Checa si el terminal dado existe.
 *
 * @param *char
 * @return int
 **/
int existeTerminal(char *term){
	int i;
	for (i=0; i<terminales; i++) {
		if (strcmp(arregloTerminales[i],term) == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Checa si el no terminal dado existe.
 *
 * @param *char
 * @return int
 **/
int existeNoTerminal(char *noterm){
	int i;
	for (i=0; i<noTerminales; i++) {
		if (strcmp(arregloNoTerminales[i],noterm) == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Convierte el numero llave dado a su representacion en string
 * Puede ser tanto terminal como no terminal
 * El numero comienza desde 0
 *
 * @param int
 * @return *char
 **/
char* convierteAString(int num){
	if (num < 200) {
		return itoa(num - 100);
	} else if (num  < 300) {
		return arregloNoTerminales[num - 200];
	} else if (num < 400) {
		return arregloTerminales[num - 300];
	}
	return "";
}

/**
 * Convierte el string dado en su representacion numerica
 * Los numeros (valores numericos de la pila) comienzan en 100 y pueden ir hasta 199
 * Los no terminales van de 200 hasta 299
 * Los terminales van de 300 hasta 399
 *
 * @param *char
 * @return int
 **/
int convierteAInt(char *str){
	int i;
	for (i=0; i<noTerminales; i++) {
		if (eq(arregloNoTerminales[i], str)) {
			return 200 + i;
		}
	}
	for (i=0; i<terminales; i++) {
		if (eq(arregloTerminales[i], str)) {
			return 300 + i;
		}
	}
	
	return 100 + atoi(str);
}

/**
 * Convierte el string dado a su valor en la matriz
 *
 * @param *char
 * @return int
 **/
int convierteAMat(char *str){
	int i;
	for (i=0; i<noTerminales; i++) {
		if (eq(arregloNoTerminales[i], str)) {
			return terminales + i;
		}
	}
	for (i=0; i<terminales; i++) {
		if (eq(arregloTerminales[i], str)) {
			return i;
		}
	}
	
	return atoi(str);
}

/**
 * Imprime la pila
 *
 * @param void
 * @return void
 **/
char* imprimePila(char* ret){
	memset(ret, '\0', BUFSIZ);
	int pilaprint[BUFSIZ];
	int k;
	//char s[BUFSIZ];
	despliega(&pila, pilaprint);
	
	for (k=0; k<pila.size; k++) {
		strcat(ret,convierteAString(pilaprint[k]));
		strcat(ret," ");
	}
	return ret;
}

/**
 * Imprime la tabla de SLR1
 *
 * @param void
 * @return void
 **/
void imprimeTabla(){
	int i,j;
	for (i=0; i<estados; i++) {
		fprintf(stdout, "\nE%d\n",i);
		for (j=0; j<(noTerminales+terminales); j++) {
			fprintf(stdout, " %d|%d ",tablaR[i][j].tipo,tablaR[i][j].valor);
		}
	}
}

/**
 * Imprime el numero de gramatica dado
 *
 * @param void
 * @return void
 **/
void imprimeGramatica(int num){
	int i;
	fprintf(stdout, "%s->",gramatica[num].cadenaDerivacion[0]);
	for (i = 1; i<gramatica[num].derivaciones; i++) {
		fprintf(stdout, "%s",gramatica[num].cadenaDerivacion[i]);
	}
}

/**
 * Imprime los no terminales
 *
 * @param void
 * @return void
 **/
void imprimeTerNoTer(){
	int i;
	fprintf(stdout, "\nTerminales(%d):\n",terminales);
	for (i = 0; i<terminales; i++) {
		fprintf(stdout, "%i:%s ",i,arregloTerminales[i]);
	}
	fprintf(stdout, "\n\nNo Terminales(%d):\n",noTerminales);
	for (i = 0; i<noTerminales; i++) {
		fprintf(stdout, "%i:%s ",i,arregloNoTerminales[i]);
	}
}

/**
 * Imprime el valor de entrada desde la localidad dada
 *
 * @param void
 * @return void
 **/
char* imprimeInput(char* ret, int num){
	memset(ret, '\0', BUFSIZ);
	int k;
	
	for (k=num; k<inputSize; k++) {
		strcat(ret,input[k]);
		strcat(ret," ");
	}
	return ret;
}

/**
 * Imprime el valor de entrada desde la localidad dada
 *
 * @param void
 * @return void
 **/
char* imprimeInputReal(char* ret, int num){
	memset(ret, '\0', BUFSIZ);
	int k;
	
	for (k=num; k<inputSize; k++) {
		strcat(ret,inputReal[k]);
		strcat(ret," ");
	}
	return ret;
}


/**
 * Imprime los formatos en pantalla dependiendo del tipo especificado
 *
 * @param int tipo de impresion
 * @param int valor de i
 * @param int
 * @param int
 * @return void
 **/
void imprimeFormato(int tipo,int i,int valor){
	char ret[BUFSIZ];
	char ret1[BUFSIZ];
	//int k;
	switch (tipo) {
			// Header
		case 0:
			fprintf(stdout, "%-90s\t%-45s\tACCION\n","PILA","ENTRADA");
			break;
			// Derivacion normal D#
		case 1:
			fprintf(stdout,"%-90s\t%-45s\t", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "D%d",valor);
			fprintf(stdout, "\n");
			break;
			// Reduccion R#
		case 2:
			fprintf(stdout,"%-90s\t%-45s\t", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "R%d: ",valor+1);
			imprimeGramatica(valor);
			fprintf(stdout, "\n");
			break;
			// Acepta
		case 3:
			fprintf(stdout, "%-90s\t%-45s\tCadena Aceptada\n", imprimePila(ret), imprimeInput(ret1,i));
			break;
			// Error Sintactico
		case 4:
			fprintf(stdout, "%-90s\t%-45s\tError Sintactico.", imprimePila(ret), imprimeInputReal(ret1,i));
			fprintf(stdout, "\n");
			errorSintactico = 1;
			break;
		default:
			break;
	}
	
}

/**
 * Metodo que hace las corridas del analizador lexico
 *
 * @param void
 * @return void
 **/
int anasin(){
    
	push(&pila, convierteAInt("$"));
	push(&pila, convierteAInt("0"));
	int i = 0, t, yapaso = 0;
	char *aux, *uno, *cero, *dos, *p;
	// Para escribir menos
	regla actual;
	
	// Imprime Header
	imprimeFormato(0,-1,-1);
    
	while (1) {
		// Toma primer elemento
		aux = convierteAString(top(&pila));
        //fprintf(stdout, "(%d,%s)%d", atoi(aux),input[i],tablaR[atoi(aux)][convierteAMat(input[i])].tipo);
		// Guardas lo que hay en la tabla en una variable
		actual = tablaR[atoi(aux)][convierteAMat(input[i])];
		
		// Si es D#
		if (actual.tipo == ACEPTA) {
			imprimeFormato(3, i, -1);
			
			return 0;
		} else if (actual.tipo == D) {
			// Imprime
			imprimeFormato(1, i, actual.valor);
			// Mete el de accion
			push(&pila, convierteAInt(input[i]));
			// Mete el valor de D
			push(&pila, convierteAInt(itoa(actual.valor)));
			//fprintf(stdout, "\nAcastoy, Meto %s %d\n",input[i],actual.valor);
			// Incrementa Valor
			i++;
		} else if (actual.tipo == R) {
			// Pop hasta encontrar en pila el primer valor de derivacion
			cero	= gramatica[actual.valor - 1].cadenaDerivacion[0];
			uno		= gramatica[actual.valor - 1].cadenaDerivacion[1];
			if(gramatica[actual.valor ].derivaciones > 2){
                dos		= gramatica[actual.valor - 1].cadenaDerivacion[2];
            }
			fprintf(stdout, "cero:%s Uno:%s\n",cero,uno);
			
			// Si la derivacion no es a epsilon se hace pop
			if (!eq(uno,"epsilon")) {
				
				while ((p = convierteAString(pop(&pila))),uno) {
					fprintf(stdout, "Pop:%s\n",p);
					if (eq(p,"$")) {
						imprimeFormato(4, i, -1);
						fprintf(stdout, "No encontre %s, salir.\n",uno);
						return -1;
					}
					if (eq(p,uno)) {
						// Checa con el siguiente valor para el caso S->CC
						/*if(gramatica[actual.valor - 1].derivaciones > 2) {
                            fprintf(stdout, "Dos:%s\n",dos);
                            if (eq(uno,dos) && yapaso == 0) {
                                fprintf(stdout, "Uno igual a dos:%s\n",dos);
                                yapaso = 1;
                                continue;
                            }
                            break;
                        }*/
                        break;
					}
				}
			}
			imprimeFormato(2, i, actual.valor - 1);
			// t siempre va a ser un numero (el renglon de la tabla)
			t = top(&pila);
			// Agrega el derivado a la pila
			push(&pila,		convierteAInt(cero));
			push(&pila,		convierteAInt(itoa(tablaR[atoi(convierteAString(t))][convierteAMat(cero)].valor)));
			
			// Imprime
			//imprimeFormato(2, i, actual.valor - 1);
		} else if (actual.tipo == ERR) {
			imprimeFormato(4, i, -1);
			return -1;
		}
		//fprintf(stdout, "pila:%s\n",imprimePila(ret));
	}
}

/**
 * Metodo que inicializa los tipod y los valores de
 * las reglas en -1, esto significa que por default todas
 * las localidades de la matriz son de ERROR hasta que se
 * agregue algo en ellas
 *
 * @param void
 * @return void
 **/
void inicializaTabla(regla **tablaR){
	int i,j;
	for (i=0; i<estados; i++) {
		for (j=0; j<(terminales+noTerminales); j++) {
			tablaR[i][j].tipo  = ERR;
			tablaR[i][j].valor = ERR;
		}
	}
}


/**
 * Limpia el arreglo de strings input
 *
 * @param void
 * @return void
 **/
void clearInput(){
	memset(input, 0, BUFSIZ);
	memset(inputReal, 0, BUFSIZ);
	inputSize = 0;
}

/**
 * Este es un ejemplo de como se puede llenar la tabla en caso de que no exista lectura desde archivos
 *
 * @param void
 * @return void
 **/
void inicializaTemp(){
	// Inicializa cantidades
	noTerminales = 18;
	terminales	 = 19;
	estados		 = 68;
	derivacionesGramatica = 32;
    siguientes = 0;
	
	// Crea arreglo de no terminales
    arregloNoTerminales[0] = "PROGRAM";
    arregloNoTerminales[1] = "GLOBAL_DECLARATIONS";
    arregloNoTerminales[2] = "VAR_LIST";
    arregloNoTerminales[3] = "VAR_ITEM";
    arregloNoTerminales[4] = "MAIN_DEF";
    arregloNoTerminales[5] = "FUNCTION_BODY";
    arregloNoTerminales[6] = "DECLARATION";
    arregloNoTerminales[7] = "INTERNAL_DECLARATIONS";
    arregloNoTerminales[8] = "STATEMENT_LIST";
    arregloNoTerminales[9] = "WHILE_STATEMENT";
    arregloNoTerminales[10] = "IF_STATEMENT";
    arregloNoTerminales[11] = "STATEMENT";
    arregloNoTerminales[12] = "RETURN_STATEMENT";
    arregloNoTerminales[13] = "EXPRESSION";
    arregloNoTerminales[14] = "ASSIGN_EXP";
    arregloNoTerminales[15] = "BINARY_EXP";
    arregloNoTerminales[16] = "BINARY_OP";
    arregloNoTerminales[17] = "PRIMARY_EXPR";
	
    // Crea arreglo de terminales
    arregloTerminales[0] = "$";
    arregloTerminales[1] = "right_parenthesis";
    arregloTerminales[2] = "left_parenthesis";
    arregloTerminales[3] = "constant";
    arregloTerminales[4] = "var_name";
    arregloTerminales[5] = "arith_op";
    arregloTerminales[6] = "rel_op";
    arregloTerminales[7] = "boolean_op";
    arregloTerminales[8] = "equal";
    arregloTerminales[9] = "semi_colon";
    arregloTerminales[10] = "return";
    arregloTerminales[11] = "right_curly_bracket";
    arregloTerminales[12] = "left_curly_bracket";
    arregloTerminales[13] = "else";
    arregloTerminales[14] = "if";
    arregloTerminales[15] = "while";
    arregloTerminales[16] = "main";
    arregloTerminales[17] = "var_type";
    arregloTerminales[18] = "comma";
	
	// Definir el tamaño de la matriz (tabla)
	tablaR = (regla**)malloc(sizeof(regla)*estados);
	int row;
	for (row=0; row<estados; row++) {
		tablaR[row] = (regla*)malloc(sizeof(regla) * (terminales + noTerminales + 1));
	}
	
    inicializaTabla(tablaR);
    
	// Arreglo gramatica
	gramatica = (derivacion *)malloc(sizeof(derivacion)*derivacionesGramatica);
	
    // PROGRAM -> MAIN_DEF
    gramatica[0].cadenaDerivacion[0] = "PROGRAM";
    gramatica[0].cadenaDerivacion[1] = "MAIN_DEF";
    gramatica[0].derivaciones		 = 2;
    
    // DECLARATION -> var_type VAR_LIST semi_colon
    gramatica[1].cadenaDerivacion[0] = "DECLARATION";
    gramatica[1].cadenaDerivacion[1] = "var_type";
    gramatica[1].cadenaDerivacion[2] = "VAR_LIST";
    gramatica[1].cadenaDerivacion[3] = "semi_colon";
    gramatica[1].derivaciones		 = 4;
    
    // VAR_LIST -> VAR_LIST comma VAR_ITEM
    gramatica[2].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[2].cadenaDerivacion[1] = "VAR_LIST";
    gramatica[2].cadenaDerivacion[2] = "comma";
    gramatica[2].cadenaDerivacion[3] = "VAR_ITEM";
    gramatica[2].derivaciones = 4;
    
    // VAR_LIST -> VAR_ITEM
    gramatica[3].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[3].cadenaDerivacion[1] = "VAR_ITEM";
    gramatica[3].derivaciones = 2;
    
    
    // VAR_ITEM -> var_name
    gramatica[4].cadenaDerivacion[0] = "VAR_ITEM";
    gramatica[4].cadenaDerivacion[1] = "var_name";
    gramatica[4].derivaciones = 2;
    
    // MAIN_DEF -> var_type main left_parenthesis right_parenthesis left_curly_bracket FUNCTION_BODY right_curly_bracket.
    gramatica[5].cadenaDerivacion[0] = "MAIN_DEF";
    gramatica[5].cadenaDerivacion[1] = "var_type";
    gramatica[5].cadenaDerivacion[2] = "main";
    gramatica[5].cadenaDerivacion[3] = "left_parenthesis";
    gramatica[5].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[5].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[5].cadenaDerivacion[6] = "FUNCTION_BODY";
    gramatica[5].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[5].derivaciones = 8;
    
    // FUNCTION_BODY -> INTERNAL_DECLARATIONS STATEMENT_LIST.
    gramatica[6].cadenaDerivacion[0] = "FUNCTION_BODY";
    gramatica[6].cadenaDerivacion[1] = "INTERNAL_DECLARATIONS";
    gramatica[6].cadenaDerivacion[2] = "STATEMENT_LIST";
    gramatica[6].derivaciones = 3;
    
    // INTERNAL_DECLARATIONS -> DECLARATION INTERNAL_DECLARATIONS
    gramatica[7].cadenaDerivacion[0] = "INTERNAL_DECLARATIONS";
    gramatica[7].cadenaDerivacion[1] = "DECLARATION";
    gramatica[7].cadenaDerivacion[2] = "INTERNAL_DECLARATIONS";
    gramatica[7].derivaciones = 3;
    
    // INTERNAL_DECLARATIONS -> <epsilon>
    gramatica[8].cadenaDerivacion[0] = "INTERNAL_DECLARATIONS";
    gramatica[8].cadenaDerivacion[1] = "epsilon";
    gramatica[8].derivaciones = 2;
    
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    gramatica[9].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[9].cadenaDerivacion[1] = "STATEMENT";
    gramatica[9].cadenaDerivacion[2] = "STATEMENT_LIST";
    gramatica[9].derivaciones = 3;
    
    // STATEMENT_LIST -> <epsilon>
    gramatica[10].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[10].cadenaDerivacion[1] = "epsilon";
    gramatica[10].derivaciones = 2;
    
    // STATEMENT -> IF_STATEMENT
    gramatica[11].cadenaDerivacion[0] = "STATEMENT";
    gramatica[11].cadenaDerivacion[1] = "IF_STATEMENT";
    gramatica[11].derivaciones = 2;
    
    // STATEMENT -> EXPRESSION semi_colon
    gramatica[12].cadenaDerivacion[0] = "STATEMENT";
    gramatica[12].cadenaDerivacion[1] = "EXPRESSION_STATEMENT";
    gramatica[12].cadenaDerivacion[2] = "semi_colon";
    gramatica[12].derivaciones = 3;
    
    // STATEMENT -> WHILE_STATEMENT
    gramatica[13].cadenaDerivacion[0] = "STATEMENT";
    gramatica[13].cadenaDerivacion[1] = "WHILE_STATEMENT";
    gramatica[13].derivaciones = 2;
    
    // STATEMENT -> RETURN_STATEMENT
    gramatica[14].cadenaDerivacion[0] = "STATEMENT";
    gramatica[14].cadenaDerivacion[1] = "RETURN_STATEMENT";
    gramatica[14].derivaciones = 2;
    
    // STATEMENT -> semi_colon
    gramatica[15].cadenaDerivacion[0] = "STATEMENT";
    gramatica[15].cadenaDerivacion[1] = "semi_colon";
    gramatica[15].derivaciones = 2;
    
    // WHILE_STATEMENT -> while left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket
    gramatica[16].cadenaDerivacion[0] = "WHILE_STATEMENT";
    gramatica[16].cadenaDerivacion[1] = "while";
    gramatica[16].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[16].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[16].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[16].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[16].cadenaDerivacion[6] = "STATEMENT";
    gramatica[16].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[16].derivaciones = 8;
    
    // IF_STATEMENT -> if left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket
    gramatica[17].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[17].cadenaDerivacion[1] = "if";
    gramatica[17].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[17].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[17].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[17].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[17].cadenaDerivacion[6] = "STATEMENT";
    gramatica[17].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[17].derivaciones = 8;
    
    
    // IF_STATEMENT -> if left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket else left_curly_bracket STATEMENT right_curly_bracket
    gramatica[18].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[18].cadenaDerivacion[1] = "if";
    gramatica[18].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[18].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[18].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[18].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[18].cadenaDerivacion[6] = "STATEMENT";
    gramatica[18].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[18].cadenaDerivacion[8] = "else";
    gramatica[18].cadenaDerivacion[9] = "left_curly_bracket";
    gramatica[18].cadenaDerivacion[10] = "STATEMENT";
    gramatica[18].cadenaDerivacion[11] = "right_curly_bracket";
    gramatica[18].derivaciones = 12;
    
    // RETURN_STATEMENT -> return EXPRESSION semi_colon
    gramatica[19].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[19].cadenaDerivacion[1] = "return";
    gramatica[19].cadenaDerivacion[2] = "EXPRESSION";
    gramatica[19].cadenaDerivacion[3] = "semi_colon";
    gramatica[19].derivaciones = 4;
    
    // RETURN_STATEMENT -> return semi_colon
    gramatica[20].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[20].cadenaDerivacion[1] = "return";
    gramatica[20].cadenaDerivacion[2] = "semi_colon";
    gramatica[20].derivaciones = 3;
    
    // EXPRESSION -> ASSIGN_EXP
    gramatica[21].cadenaDerivacion[0] = "EXPRESSION";
    gramatica[21].cadenaDerivacion[1] = "ASSIGN_EXP";
    gramatica[21].derivaciones = 2;
    
    // ASSIGN_EXP -> var_name equal BINARY_EXP
    gramatica[22].cadenaDerivacion[0] = "var_name";
    gramatica[22].cadenaDerivacion[1] = "equal";
    gramatica[22].cadenaDerivacion[2] = "BINARY_EXP";
    gramatica[22].derivaciones = 3;
    
    // ASSIGN_EXP -> BINARY_EXP
    gramatica[23].cadenaDerivacion[0] = "ASSIGN_EXP";
    gramatica[23].cadenaDerivacion[1] = "BINARY_EXP";
    gramatica[23].derivaciones = 2;
    
    // BINARY_EXP -> BINARY_EXP BINARY_OP PRIMARY_EXPR
    gramatica[24].cadenaDerivacion[0] = "BINARY_EXP";
    gramatica[24].cadenaDerivacion[1] = "BINARY_EXP";
    gramatica[24].cadenaDerivacion[2] = "BINARY_OP";
    gramatica[24].cadenaDerivacion[3] = "PRIMARY_EXPR";
    gramatica[24].derivaciones = 4;
    
    // BINARY_EXP -> PRIMARY_EXPR
    gramatica[25].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[25].cadenaDerivacion[1] = "PRIMARY_EXPR";
    gramatica[25].derivaciones = 2;
    
    // BINARY_OP -> boolean_op
    gramatica[26].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[26].cadenaDerivacion[1] = "boolean_op";
    gramatica[26].derivaciones = 2;
    
    // BINARY_OP -> rel_op
    gramatica[27].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[27].cadenaDerivacion[1] = "rel_op";
    gramatica[27].derivaciones = 2;
    
    // BINARY_OP -> arith_op
    gramatica[28].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[28].cadenaDerivacion[1] = "arith_op";
    gramatica[28].derivaciones = 2;
    
    // PRIMARY_EXPR -> var_name
    gramatica[29].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[29].cadenaDerivacion[1] = "var_name";
    gramatica[29].derivaciones = 2;
    
    // PRIMARY_EXPR -> constant
    gramatica[30].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[30].cadenaDerivacion[1] = "constant";
    gramatica[30].derivaciones = 2;
    
    // PRIMARY_EXPR -> left_parenthesis PRIMARY_EXPR right_parenthesis
    gramatica[31].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[31].cadenaDerivacion[1] = "left_parenthesis";
    gramatica[31].cadenaDerivacion[2] = "PRIMARY_EXPR";
    gramatica[31].cadenaDerivacion[3] = "right_parenthesis";
    gramatica[31].derivaciones = 4;
    
	// Inicializa la tabla con valores default
	inicializaTabla(tablaR);
    
	// Tabla
    tablaR[0][17].tipo = D;
    tablaR[0][17].valor = 4;
    
    tablaR[0][19].tipo = D;
    tablaR[0][19].valor = 3;
    
    tablaR[0][20].tipo = D;
    tablaR[0][20].valor = 2;
    
    tablaR[0][23].tipo = D;
    tablaR[0][23].valor = 1;
    
    tablaR[1][0].tipo = R;
    tablaR[1][0].valor = 1;
    
    tablaR[2][0].tipo = R;
    tablaR[2][0].valor = 0;
    
    tablaR[3][0].tipo = ACEPTA;
    tablaR[3][0].valor = ERR;
    
    tablaR[4][16].tipo = D;
    tablaR[4][16].valor = 5;
    
    tablaR[5][2].tipo = D;
    tablaR[5][2].valor = 6;
    
    tablaR[6][1].tipo = D;
    tablaR[6][1].valor = 7;
    
    tablaR[7][12].tipo = D;
    tablaR[7][12].valor = 8;
    
    tablaR[8][2].tipo = R;
    tablaR[8][2].valor = 9;
    
    tablaR[8][3].tipo = R;
    tablaR[8][3].valor = 9;
    
    tablaR[8][4].tipo = R;
    tablaR[8][4].valor = 9;
    
    tablaR[8][9].tipo = R;
    tablaR[8][9].valor = 9;
    
    tablaR[8][10].tipo = R;
    tablaR[8][10].valor = 9;
    
    tablaR[8][11].tipo = R;
    tablaR[8][11].valor = 9;
    
    tablaR[8][14].tipo = R;
    tablaR[8][14].valor = 9;
    
    tablaR[8][15].tipo = R;
    tablaR[8][15].valor = 9;
    
    tablaR[8][17].tipo = D;
    tablaR[8][17].valor = 12;
    
    tablaR[8][24].tipo = D;
    tablaR[8][24].valor = 11;
    
    tablaR[8][25].tipo = D;
    tablaR[8][25].valor = 10;
    
    tablaR[8][26].tipo = D;
    tablaR[8][26].valor = 9;
    
    tablaR[9][2].tipo = D;
    tablaR[9][2].valor = 33;
    
    tablaR[9][3].tipo = D;
    tablaR[9][3].valor = 32;
    
    tablaR[9][4].tipo = D;
    tablaR[9][4].valor = 31;
    
    tablaR[9][9].tipo = D;
    tablaR[9][9].valor = 30;
    
    tablaR[9][10].tipo = D;
    tablaR[9][10].valor = 29;
    
    tablaR[9][11].tipo = R;
    tablaR[9][11].valor = 11;
    
    tablaR[9][14].tipo = D;
    tablaR[9][14].valor = 28;
    
    tablaR[9][15].tipo = D;
    tablaR[9][15].valor = 27;
    
    tablaR[9][27].tipo = D;
    tablaR[9][27].valor = 26;
    
    tablaR[9][28].tipo = D;
    tablaR[9][28].valor = 25;
    
    tablaR[9][29].tipo = D;
    tablaR[9][29].valor = 24;
    
    tablaR[9][30].tipo = D;
    tablaR[9][30].valor = 23;
    
    tablaR[9][31].tipo = D;
    tablaR[9][31].valor = 22;
    
    tablaR[9][32].tipo = D;
    tablaR[9][32].valor = 21;
    
    tablaR[9][33].tipo = D;
    tablaR[9][33].valor = 20;
    
    tablaR[9][34].tipo = D;
    tablaR[9][34].valor = 19;
    
    tablaR[9][36].tipo = D;
    tablaR[9][36].valor = 18;
    
    tablaR[10][2].tipo = R;
    tablaR[10][2].valor = 9;
    
    tablaR[10][3].tipo = R;
    tablaR[10][3].valor = 8;
    
    tablaR[10][4].tipo = R;
    tablaR[10][4].valor = 9;
    
    tablaR[10][9].tipo = R;
    tablaR[10][9].valor = 9;
    
    tablaR[10][10].tipo = R;
    tablaR[10][10].valor = 9;
    
    tablaR[10][11].tipo = R;
    tablaR[10][11].valor = 9;
    
    tablaR[10][14].tipo = R;
    tablaR[10][14].valor = 9;
    
    tablaR[10][15].tipo = R;
    tablaR[10][15].valor = 9;
    
    tablaR[10][17].tipo = D;
    tablaR[10][17].valor = 12;
    
    tablaR[10][25].tipo = D;
    tablaR[10][25].valor = 10;
    
    tablaR[10][26].tipo = D;
    tablaR[10][26].valor = 17;
    
    tablaR[11][11].tipo = D;
    tablaR[11][11].valor = 16;
    
    tablaR[12][4].tipo = D;
    tablaR[12][4].valor = 15;
    
    tablaR[12][21].tipo = D;
    tablaR[12][21].valor = 14;
    
    tablaR[12][22].tipo = D;
    tablaR[12][22].valor = 13;
    
    tablaR[13][9].tipo = R;
    tablaR[13][9].valor = 4;
    
    tablaR[13][18].tipo = R;
    tablaR[13][18].valor = 4;
    
    tablaR[14][9].tipo = D;
    tablaR[14][9].valor = 48;
    
    tablaR[14][18].tipo = D;
    tablaR[14][18].valor = 47;
    
    tablaR[15][9].tipo = R;
    tablaR[15][9].valor = 5;
    
    tablaR[15][18].tipo = R;
    tablaR[15][18].valor = 5;
    
    tablaR[16][0].tipo = R;
    tablaR[16][0].valor = 6;
    
    tablaR[17][2].tipo = R;
    tablaR[17][2].valor = 8;
    
    tablaR[17][3].tipo = R;
    tablaR[17][3].valor = 8;
    
    tablaR[17][4].tipo = R;
    tablaR[17][4].valor = 8;
    
    tablaR[17][9].tipo = R;
    tablaR[17][9].valor = 8;
    
    tablaR[17][10].tipo = R;
    tablaR[17][10].valor = 8;
    
    tablaR[17][11].tipo = R;
    tablaR[17][11].valor = 8;
    
    tablaR[17][14].tipo = R;
    tablaR[17][14].valor = 8;
    
    tablaR[17][15].tipo = R;
    tablaR[17][15].valor = 8;
    
    tablaR[18][1].tipo = R;
    tablaR[18][1].valor = 26;
    
    tablaR[18][5].tipo = R;
    tablaR[18][5].valor = 26;
    
    tablaR[18][6].tipo = R;
    tablaR[18][6].valor = 26;
    
    tablaR[18][7].tipo = R;
    tablaR[18][7].valor = 26;
    
    tablaR[18][9].tipo = R;
    tablaR[18][9].valor = 26;
    
    tablaR[19][1].tipo = R;
    tablaR[19][1].valor = 24;
    
    tablaR[19][5].tipo = D;
    tablaR[19][5].valor = 46;
    
    tablaR[19][6].tipo = D;
    tablaR[19][6].valor = 45;
    
    tablaR[19][7].tipo = D;
    tablaR[19][7].valor = 44;
    
    tablaR[19][9].tipo = R;
    tablaR[19][9].valor = 24;
    
    tablaR[19][35].tipo = D;
    tablaR[19][35].valor = 43;
    
    tablaR[20][1].tipo = R;
    tablaR[20][1].valor = 22;
    
    tablaR[20][9].tipo = R;
    tablaR[20][9].valor = 22;
    
    tablaR[21][9].tipo = D;
    tablaR[21][9].valor = 42;
    
    tablaR[22][2].tipo = R;
    tablaR[22][2].valor = 15;
    
    tablaR[22][3].tipo = R;
    tablaR[22][3].valor = 15;
    
    tablaR[22][4].tipo = R;
    tablaR[22][4].valor = 15;
    
    tablaR[22][9].tipo = R;
    tablaR[22][9].valor = 15;
    
    tablaR[22][10].tipo = R;
    tablaR[22][10].valor = 15;
    
    tablaR[22][11].tipo = R;
    tablaR[22][11].valor = 15;
    
    tablaR[22][14].tipo = R;
    tablaR[22][14].valor = 15;
    
    tablaR[22][15].tipo = R;
    tablaR[22][15].valor = 15;
    
    tablaR[23][2].tipo = D;
    tablaR[23][2].valor = 33;
    
    tablaR[23][3].tipo = D;
    tablaR[23][3].valor = 32;
    
    tablaR[23][4].tipo = D;
    tablaR[23][4].valor = 31;
    
    tablaR[23][9].tipo = D;
    tablaR[23][9].valor = 30;
    
    tablaR[23][10].tipo = D;
    tablaR[23][10].valor = 29;
    
    tablaR[23][11].tipo = R;
    tablaR[23][11].valor = 11;
    
    tablaR[23][14].tipo = D;
    tablaR[23][14].valor = 28;
    
    tablaR[23][15].tipo = D;
    tablaR[23][15].valor = 27;
    
    tablaR[23][27].tipo = D;
    tablaR[23][27].valor = 41;
    
    tablaR[23][28].tipo = D;
    tablaR[23][28].valor = 25;
    
    tablaR[23][29].tipo = D;
    tablaR[23][29].valor = 24;
    
    tablaR[23][30].tipo = D;
    tablaR[23][30].valor = 23;
    
    tablaR[23][31].tipo = D;
    tablaR[23][31].valor = 22;
    
    tablaR[23][32].tipo = D;
    tablaR[23][32].valor = 21;
    
    tablaR[23][33].tipo = D;
    tablaR[23][33].valor = 20;
    
    tablaR[23][34].tipo = D;
    tablaR[23][34].valor = 19;
    
    tablaR[23][36].tipo = D;
    tablaR[23][36].valor = 18;
    
    tablaR[24][2].tipo = R;
    tablaR[24][2].valor = 12;
    
    tablaR[24][3].tipo = R;
    tablaR[24][3].valor = 12;
    
    tablaR[24][4].tipo = R;
    tablaR[24][4].valor = 12;
    
    tablaR[24][9].tipo = R;
    tablaR[24][9].valor = 12;
    
    tablaR[24][10].tipo = R;
    tablaR[24][10].valor = 12;
    
    tablaR[24][11].tipo = R;
    tablaR[24][11].valor = 12;
    
    tablaR[24][14].tipo = R;
    tablaR[24][14].valor = 12;
    
    tablaR[24][15].tipo = R;
    tablaR[24][15].valor = 12;
    
    tablaR[25][2].tipo = R;
    tablaR[25][2].valor = 14;
    
    tablaR[25][3].tipo = R;
    tablaR[25][3].valor = 14;
    
    tablaR[25][4].tipo = R;
    tablaR[25][4].valor = 14;
    
    tablaR[25][9].tipo = R;
    tablaR[25][9].valor = 14;
    
    tablaR[25][10].tipo = R;
    tablaR[25][10].valor = 14;
    
    tablaR[25][11].tipo = R;
    tablaR[25][11].valor = 14;
    
    tablaR[25][14].tipo = R;
    tablaR[25][14].valor = 14;
    
    tablaR[25][15].tipo = R;
    tablaR[25][15].valor = 14;
    
    tablaR[26][11].tipo = R;
    tablaR[26][11].valor = 7;
    
    tablaR[27][2].tipo = D;
    tablaR[27][2].valor = 40;
    
    tablaR[28][2].tipo = D;
    tablaR[28][2].valor = 39;
    
    tablaR[29][2].tipo = D;
    tablaR[29][2].valor = 33;
    
    tablaR[29][3].tipo = D;
    tablaR[29][3].valor = 32;
    
    tablaR[29][4].tipo = D;
    tablaR[29][4].valor = 31;
    
    tablaR[29][9].tipo = D;
    tablaR[29][9].valor = 38;
    
    tablaR[29][32].tipo = D;
    tablaR[29][32].valor = 37;
    
    tablaR[29][33].tipo = D;
    tablaR[29][33].valor = 20;
    
    tablaR[29][34].tipo = D;
    tablaR[29][34].valor = 19;
    
    tablaR[29][36].tipo = D;
    tablaR[29][36].valor = 18;
    
    tablaR[30][2].tipo = R;
    tablaR[30][2].valor = 16;
    
    tablaR[30][3].tipo = R;
    tablaR[30][3].valor = 16;
    
    tablaR[30][4].tipo = R;
    tablaR[30][4].valor = 16;
    
    tablaR[30][9].tipo = R;
    tablaR[30][9].valor = 16;
    
    tablaR[30][10].tipo = R;
    tablaR[30][10].valor = 16;
    
    tablaR[30][11].tipo = R;
    tablaR[30][11].valor = 16;
    
    tablaR[30][14].tipo = R;
    tablaR[30][14].valor = 16;
    
    tablaR[30][15].tipo = R;
    tablaR[30][15].valor = 16;
    
    tablaR[31][1].tipo = R;
    tablaR[31][1].valor = 30;
    
    tablaR[31][5].tipo = R;
    tablaR[31][5].valor = 30;
    
    tablaR[31][6].tipo = R;
    tablaR[31][6].valor = 30;
    
    tablaR[31][7].tipo = R;
    tablaR[31][7].valor = 30;
    
    tablaR[31][8].tipo = D;
    tablaR[31][8].valor = 36;
    
    tablaR[31][9].tipo = R;
    tablaR[31][9].valor = 30;
    
    tablaR[32][1].tipo = R;
    tablaR[32][1].valor = 31;
    
    tablaR[32][5].tipo = R;
    tablaR[32][5].valor = 31;
    
    tablaR[32][6].tipo = R;
    tablaR[32][6].valor = 31;
    
    tablaR[32][7].tipo = R;
    tablaR[32][7].valor = 31;
    
    tablaR[32][9].tipo = R;
    tablaR[32][9].valor = 31;
    
    tablaR[33][2].tipo = D;
    tablaR[33][2].valor = 33;
    
    tablaR[33][3].tipo = D;
    tablaR[33][3].valor = 32;
    
    tablaR[33][4].tipo = D;
    tablaR[33][4].valor = 35;
    
    tablaR[33][36].tipo = D;
    tablaR[33][36].valor = 34;
    
    tablaR[34][1].tipo = D;
    tablaR[34][1].valor = 55;
    
    tablaR[35][1].tipo = R;
    tablaR[35][1].valor = 30;
    
    tablaR[35][5].tipo = R;
    tablaR[35][5].valor = 30;
    
    tablaR[35][6].tipo = R;
    tablaR[35][6].valor = 30;
    
    tablaR[35][7].tipo = R;
    tablaR[35][7].valor = 30;
    
    tablaR[36][2].tipo = D;
    tablaR[36][2].valor = 33;
    
    tablaR[36][3].tipo = D;
    tablaR[36][3].valor = 32;
    
    tablaR[36][4].tipo = D;
    tablaR[36][4].valor = 35;
    
    tablaR[36][34].tipo = D;
    tablaR[36][34].valor = 54;
    
    tablaR[36][36].tipo = D;
    tablaR[36][36].valor = 18;
    
    tablaR[37][9].tipo = D;
    tablaR[37][9].valor = 53;
    
    tablaR[38][2].tipo = R;
    tablaR[38][2].valor = 21;
    
    tablaR[38][3].tipo = R;
    tablaR[38][3].valor = 21;
    
    tablaR[38][4].tipo = R;
    tablaR[38][4].valor = 21;
    
    tablaR[38][9].tipo = R;
    tablaR[38][9].valor = 21;
    
    tablaR[38][10].tipo = R;
    tablaR[38][10].valor = 21;
    
    tablaR[38][11].tipo = R;
    tablaR[38][11].valor = 21;
    
    tablaR[38][14].tipo = R;
    tablaR[38][14].valor = 21;
    
    tablaR[38][15].tipo = R;
    tablaR[38][15].valor = 21;
    
    tablaR[39][2].tipo = D;
    tablaR[39][2].valor = 33;
    
    tablaR[39][3].tipo = D;
    tablaR[39][3].valor = 32;
    
    tablaR[39][4].tipo = D;
    tablaR[39][4].valor = 31;
    
    tablaR[39][32].tipo = D;
    tablaR[39][32].valor = 52;
    
    tablaR[39][33].tipo = D;
    tablaR[39][33].valor = 20;
    
    tablaR[39][34].tipo = D;
    tablaR[39][34].valor = 19;
    
    tablaR[39][36].tipo = D;
    tablaR[39][36].valor = 18;
    
    tablaR[40][2].tipo = D;
    tablaR[40][2].valor = 33;
    
    tablaR[40][3].tipo = D;
    tablaR[40][3].valor = 32;
    
    tablaR[40][4].tipo = D;
    tablaR[40][4].valor = 31;
    
    tablaR[40][32].tipo = D;
    tablaR[40][32].valor = 51;
    
    tablaR[40][33].tipo = D;
    tablaR[40][33].valor = 20;
    
    tablaR[40][34].tipo = D;
    tablaR[40][34].valor = 19;
    
    tablaR[40][36].tipo = D;
    tablaR[40][36].valor = 18;
    
    tablaR[41][11].tipo = R;
    tablaR[41][11].valor = 10;
    
    tablaR[42][2].tipo = R;
    tablaR[42][2].valor = 13;
    
    tablaR[42][3].tipo = R;
    tablaR[42][3].valor = 13;
    
    tablaR[42][4].tipo = R;
    tablaR[42][4].valor = 13;
    
    tablaR[42][9].tipo = R;
    tablaR[42][9].valor = 13;
    
    tablaR[42][10].tipo = R;
    tablaR[42][10].valor = 13;
    
    tablaR[42][11].tipo = R;
    tablaR[42][11].valor = 13;
    
    tablaR[42][14].tipo = R;
    tablaR[42][14].valor = 13;
    
    tablaR[42][15].tipo = R;
    tablaR[42][15].valor = 13;
    
    tablaR[43][2].tipo = D;
    tablaR[43][2].valor = 33;
    
    tablaR[43][3].tipo = D;
    tablaR[43][3].valor = 32;
    
    tablaR[43][4].tipo = D;
    tablaR[43][4].valor = 35;
    
    tablaR[43][36].tipo = D;
    tablaR[43][36].valor = 50;
    
    tablaR[44][2].tipo = R;
    tablaR[44][2].valor = 27;
    
    tablaR[44][3].tipo = R;
    tablaR[44][3].valor = 27;
    
    tablaR[44][4].tipo = R;
    tablaR[44][4].valor = 27;
    
    tablaR[45][2].tipo = R;
    tablaR[45][2].valor = 28;
    
    tablaR[45][3].tipo = R;
    tablaR[45][3].valor = 28;
    
    tablaR[45][4].tipo = R;
    tablaR[45][4].valor = 28;
    
    tablaR[46][2].tipo = R;
    tablaR[46][2].valor = 29;
    
    tablaR[46][3].tipo = R;
    tablaR[46][3].valor = 29;
    
    tablaR[46][4].tipo = R;
    tablaR[46][4].valor = 29;
    
    tablaR[47][4].tipo = D;
    tablaR[47][4].valor = 15;
    
    tablaR[47][22].tipo = D;
    tablaR[47][22].valor = 49;
    
    tablaR[48][2].tipo = R;
    tablaR[48][2].valor = 2;
    
    tablaR[48][3].tipo = R;
    tablaR[48][3].valor = 2;
    
    tablaR[48][4].tipo = R;
    tablaR[48][4].valor = 2;
    
    tablaR[48][9].tipo = R;
    tablaR[48][9].valor = 2;
    
    tablaR[48][10].tipo = R;
    tablaR[48][10].valor = 2;
    
    tablaR[48][11].tipo = R;
    tablaR[48][11].valor = 2;
    
    tablaR[48][14].tipo = R;
    tablaR[48][14].valor = 2;
    
    tablaR[48][15].tipo = R;
    tablaR[48][15].valor = 2;
    
    tablaR[48][17].tipo = R;
    tablaR[48][17].valor = 2;
    
    tablaR[49][9].tipo = R;
    tablaR[49][9].valor = 3;
    
    tablaR[49][18].tipo = R;
    tablaR[49][18].valor = 3;
    
    tablaR[50][1].tipo = R;
    tablaR[50][1].valor = 25;
    
    tablaR[50][5].tipo = R;
    tablaR[50][5].valor = 25;
    
    tablaR[50][6].tipo = R;
    tablaR[50][6].valor = 25;
    
    tablaR[50][7].tipo = R;
    tablaR[50][7].valor = 25;
    
    tablaR[50][9].tipo = R;
    tablaR[50][9].valor = 25;
    
    tablaR[51][1].tipo = D;
    tablaR[51][1].valor = 57;
    
    tablaR[52][1].tipo = D;
    tablaR[52][1].valor = 56;
    
    tablaR[53][2].tipo = R;
    tablaR[53][2].valor = 20;
    
    tablaR[53][3].tipo = R;
    tablaR[53][3].valor = 20;
    
    tablaR[53][4].tipo = R;
    tablaR[53][4].valor = 20;
    
    tablaR[53][9].tipo = R;
    tablaR[53][9].valor = 20;
    
    tablaR[53][10].tipo = R;
    tablaR[53][10].valor = 20;
    
    tablaR[53][11].tipo = R;
    tablaR[53][11].valor = 20;
    
    tablaR[53][14].tipo = R;
    tablaR[53][14].valor = 20;
    
    tablaR[53][15].tipo = R;
    tablaR[53][15].valor = 20;
    
    tablaR[54][1].tipo = R;
    tablaR[54][1].valor = 23;
    
    tablaR[54][5].tipo = D;
    tablaR[54][5].valor = 46;
    
    tablaR[54][6].tipo = D;
    tablaR[54][6].valor = 45;
    
    tablaR[54][7].tipo = D;
    tablaR[54][7].valor = 44;
    
    tablaR[54][9].tipo = R;
    tablaR[54][9].valor = 23;
    
    tablaR[54][35].tipo = D;
    tablaR[54][35].valor = 43;
    
    tablaR[55][1].tipo = R;
    tablaR[55][1].valor = 32;
    
    tablaR[55][5].tipo = R;
    tablaR[55][5].valor = 32;
    
    tablaR[55][6].tipo = R;
    tablaR[55][6].valor = 32;
    
    tablaR[55][7].tipo = R;
    tablaR[55][7].valor = 32;
    
    tablaR[55][9].tipo = R;
    tablaR[55][9].valor = 32;
    
    tablaR[56][12].tipo = D;
    tablaR[56][12].valor = 59;
    
    tablaR[57][12].tipo = D;
    tablaR[57][12].valor = 58;
    
    tablaR[58][2].tipo = D;
    tablaR[58][2].valor = 33;
    
    tablaR[58][3].tipo = D;
    tablaR[58][3].valor = 32;
    
    tablaR[58][4].tipo = D;
    tablaR[58][4].valor = 31;
    
    tablaR[58][9].tipo = D;
    tablaR[58][9].valor = 30;
    
    tablaR[58][10].tipo = D;
    tablaR[58][10].valor = 29;
    
    tablaR[58][14].tipo = D;
    tablaR[58][14].valor = 28;
    
    tablaR[58][15].tipo = D;
    tablaR[58][15].valor = 27;
    
    tablaR[58][28].tipo = D;
    tablaR[58][28].valor = 25;
    
    tablaR[58][29].tipo = D;
    tablaR[58][29].valor = 24;
    
    tablaR[58][30].tipo = D;
    tablaR[58][30].valor = 61;
    
    tablaR[58][31].tipo = D;
    tablaR[58][31].valor = 22;
    
    tablaR[58][32].tipo = D;
    tablaR[58][32].valor = 21;
    
    tablaR[58][33].tipo = D;
    tablaR[58][33].valor = 20;
    
    tablaR[58][34].tipo = D;
    tablaR[58][34].valor = 19;
    
    tablaR[58][36].tipo = D;
    tablaR[58][36].valor = 18;
    
    tablaR[59][2].tipo = D;
    tablaR[59][2].valor = 33;
    
    tablaR[59][3].tipo = D;
    tablaR[59][3].valor = 32;
    
    tablaR[59][4].tipo = D;
    tablaR[59][4].valor = 31;
    
    tablaR[59][9].tipo = D;
    tablaR[59][9].valor = 30;
    
    tablaR[59][10].tipo = D;
    tablaR[59][10].valor = 29;
    
    tablaR[59][14].tipo = D;
    tablaR[59][14].valor = 28;
    
    tablaR[59][15].tipo = D;
    tablaR[59][15].valor = 27;
    
    tablaR[59][28].tipo = D;
    tablaR[59][28].valor = 25;
    
    tablaR[59][29].tipo = D;
    tablaR[59][29].valor = 24;
    
    tablaR[59][30].tipo = D;
    tablaR[59][30].valor = 60;
    
    tablaR[59][31].tipo = D;
    tablaR[59][31].valor = 22;
    
    tablaR[59][32].tipo = D;
    tablaR[59][32].valor = 21;
    
    tablaR[59][33].tipo = D;
    tablaR[59][33].valor = 20;
    
    tablaR[59][34].tipo = D;
    tablaR[59][34].valor = 19;
    
    tablaR[59][36].tipo = D;
    tablaR[59][36].valor = 18;
    
    tablaR[60][11].tipo = D;
    tablaR[60][11].valor = 63;
    
    tablaR[61][11].tipo = D;
    tablaR[61][11].valor = 62;
    
    tablaR[62][2].tipo = R;
    tablaR[62][2].valor = 17;
    
    tablaR[62][3].tipo = R;
    tablaR[62][3].valor = 17;
    
    tablaR[62][4].tipo = R;
    tablaR[62][4].valor = 17;
    
    tablaR[62][9].tipo = R;
    tablaR[62][9].valor = 17;
    
    tablaR[62][10].tipo = R;
    tablaR[62][10].valor = 17;
    
    tablaR[62][11].tipo = R;
    tablaR[62][11].valor = 17;
    
    tablaR[62][14].tipo = R;
    tablaR[62][14].valor = 17;
    
    tablaR[62][15].tipo = R;
    tablaR[62][15].valor = 17;
    
    tablaR[63][2].tipo = R;
    tablaR[63][2].valor = 18;
    
    tablaR[63][3].tipo = R;
    tablaR[63][3].valor = 18;
    
    tablaR[63][4].tipo = R;
    tablaR[63][4].valor = 18;
    
    tablaR[63][9].tipo = R;
    tablaR[63][9].valor = 18;
    
    tablaR[63][10].tipo = R;
    tablaR[63][10].valor = 18;
    
    tablaR[63][11].tipo = R;
    tablaR[63][11].valor = 18;
    
    tablaR[63][13].tipo = D;
    tablaR[63][13].valor = 64;
    
    tablaR[63][14].tipo = R;
    tablaR[63][14].valor = 18;
    
    tablaR[63][15].tipo = R;
    tablaR[63][15].valor = 18;
    
    tablaR[64][12].tipo = D;
    tablaR[64][12].valor = 65;
    
    tablaR[65][2].tipo = D;
    tablaR[65][2].valor = 33;
    
    tablaR[65][3].tipo = D;
    tablaR[65][3].valor = 32;
    
    tablaR[65][4].tipo = D;
    tablaR[65][4].valor = 31;
    
    tablaR[65][9].tipo = D;
    tablaR[65][9].valor = 30;
    
    tablaR[65][10].tipo = D;
    tablaR[65][10].valor = 29;
    
    tablaR[65][14].tipo = D;
    tablaR[65][14].valor = 28;
    
    tablaR[65][15].tipo = D;
    tablaR[65][15].valor = 27;
    
    tablaR[65][28].tipo = D;
    tablaR[65][28].valor = 25;
    
    tablaR[65][29].tipo = D;
    tablaR[65][29].valor = 24;
    
    tablaR[65][30].tipo = D;
    tablaR[65][30].valor = 66;
    
    tablaR[65][31].tipo = D;
    tablaR[65][31].valor = 22;
    
    tablaR[65][32].tipo = D;
    tablaR[65][32].valor = 21;
    
    tablaR[65][33].tipo = D;
    tablaR[65][33].valor = 20;
    
    tablaR[65][34].tipo = D;
    tablaR[65][34].valor = 19;
    
    tablaR[65][36].tipo = D;
    tablaR[65][36].valor = 18;
    
    tablaR[66][11].tipo = D;
    tablaR[66][11].valor = 67;
    
    tablaR[67][2].tipo = R;
    tablaR[67][2].valor = 19;
    
    tablaR[67][3].tipo = R;
    tablaR[67][3].valor = 19;
    
    tablaR[67][4].tipo = R;
    tablaR[67][4].valor = 19;
    
    tablaR[67][9].tipo = R;
    tablaR[67][9].valor = 19;
    
    tablaR[67][10].tipo = R;
    tablaR[67][10].valor = 19;
    
    tablaR[67][11].tipo = R;
    tablaR[67][11].valor = 19;
    
    tablaR[67][14].tipo = R;
    tablaR[67][14].valor = 19;
    
    tablaR[67][15].tipo = R;
    tablaR[67][15].valor = 19;
}
