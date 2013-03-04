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
	char *cadenaDerivacion[10];
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
char *input[BUFSIZ];
char *inputReal[BUFSIZ];
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
 * Los numeros comienzan en 100 y pueden ir hasta 199
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
			fprintf(stdout, "%-70s\t%-45s\tACCION\n","PILA","ENTRADA");
			break;
			// Derivacion normal D#
		case 1:
			fprintf(stdout,"%-70s\t%-45s\t", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "D%d",valor);
			fprintf(stdout, "\n");
			break;
			// Reduccion R#
		case 2:
			fprintf(stdout,"%-70s\t%-45s\t", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "R%d: ",valor+1);
			imprimeGramatica(valor);
			fprintf(stdout, "\n");
			break;
			// Acepta
		case 3:
			fprintf(stdout, "%-70s\t%-45s\tCadena Aceptada\n", imprimePila(ret), imprimeInput(ret1,i));
			break;
			// Error Sintactico
		case 4:
			fprintf(stdout, "%-70s\t%-45s\tError Sintactico.", imprimePila(ret), imprimeInputReal(ret1,i));
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
			
			// Incrementa Valor
			i++;
		} else if (actual.tipo == R) {
			
			// Pop hasta encontrar en pila el primer valor de derivacion
			
			cero	= gramatica[actual.valor].cadenaDerivacion[0];
			uno		= gramatica[actual.valor].cadenaDerivacion[1];
			if(gramatica[actual.valor].derivaciones > 2)
                dos		= gramatica[actual.valor].cadenaDerivacion[2];
			fprintf(stdout, "cero:%s Uno:%s\n",cero,uno);
			
			// Si la derivacion no es a epsilon se hace pop
			if (!eq(uno,"epsilon")) {
				
				while ((p = convierteAString(pop(&pila))),uno) {
					
					//fprintf(stdout, "Pop:%s\n",p);
					if (eq(p,"$")) {
						imprimeFormato(4, i, -1);
						//fprintf(stdout, "No encontre %s, salir.\n",uno);
						return -1;
					}
					if (eq(p,uno)) {
						// Checa con el siguiente valor para el caso S->CC
						if(gramatica[actual.valor].derivaciones > 2)
                            if (eq(uno,dos) && yapaso == 0) {
                                yapaso = 1;
                                continue;
                            }
						break;
					}
				}
			}
			imprimeFormato(2, i, actual.valor);
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
	noTerminales = 26;
	terminales	 = 34;
	estados		 = 93;
	derivacionesGramatica = 56;
    siguientes = 0;
	
	// Crea arreglo de no terminales
    arregloNoTerminales[0] = "PROGRAM";
    arregloNoTerminales[1] = "GLOBAL_DECLARATIONS";
    arregloNoTerminales[2] = "VAR_TYPE";
    arregloNoTerminales[3] = "VAR_LIST";
    arregloNoTerminales[4] = "VAR_ITEM";
    arregloNoTerminales[5] = "ARRAY_VAR";
    arregloNoTerminales[6] = "SCALAR_VAR";
    arregloNoTerminales[7] = "MAIN_DEF";
    arregloNoTerminales[8] = "FUNCTION_BODY";
    arregloNoTerminales[9] = "DECLARATION";
    arregloNoTerminales[10] = "INTERNAL_DECLARATIONS";
    arregloNoTerminales[11] = "STATEMENT_LIST";
    arregloNoTerminales[12] = "WHILE_STATEMENT";
    arregloNoTerminales[13] = "IF_STATEMENT";
    arregloNoTerminales[14] = "STATEMENT";
    arregloNoTerminales[15] = "RETURN_STATEMENT";
    arregloNoTerminales[16] = "EXPRESSION_STATEMENT";
    arregloNoTerminales[17] = "ASSIGNMENT_EXPRESSION";
    arregloNoTerminales[18] = "BINARY_EXPR";
    arregloNoTerminales[19] = "BINARY_OP";
    arregloNoTerminales[20] = "BOOLEAN_OP";
    arregloNoTerminales[21] = "REL_OP";
    arregloNoTerminales[22] = "ARITH_OP";
    arregloNoTerminales[23] = "PRIMARY_EXPR";
    arregloNoTerminales[24] = "EXPRESSION";
    arregloNoTerminales[25] = "CONSTANT";
	
    // Crea arreglo de terminales
    arregloTerminales[0] = "$";
    arregloTerminales[1] = "string";
    arregloTerminales[2] = "character";
    arregloTerminales[3] = "float_number";
    arregloTerminales[4] = "integer_number";
    arregloTerminales[5] = "right_parenthesis";
    arregloTerminales[6] = "left_parenthesis";
    arregloTerminales[7] = "var_name";
    arregloTerminales[8] = "mod_sign";
    arregloTerminales[9] = "divide_sign";
    arregloTerminales[10] = "multiply_sign";
    arregloTerminales[11] = "minus_sign";
    arregloTerminales[12] = "plus_sign";
    arregloTerminales[13] = "equal";
    arregloTerminales[14] = "minus_than";
    arregloTerminales[15] = "more_than";
    arregloTerminales[16] = "not";
    arregloTerminales[17] = "or_symbol";
    arregloTerminales[18] = "ampersan";
    arregloTerminales[19] = ";";
    arregloTerminales[20] = "return";
    arregloTerminales[21] = "else";
    arregloTerminales[22] = "if";
    arregloTerminales[23] = "while";
    arregloTerminales[24] = "right_curly_bracket";
    arregloTerminales[25] = "left_curly_bracket";
    arregloTerminales[26] = "main";
    arregloTerminales[27] = "int";
    arregloTerminales[28] = "right_bracket";
    arregloTerminales[29] = "left_bracket";
    arregloTerminales[30] = "comma";
    arregloTerminales[31] = "char";
    arregloTerminales[32] = "float";
    arregloTerminales[33] = "void";
    
	
	// Definir el tamaño de la matriz (tabla)
	tablaR = (regla**)malloc(sizeof(regla)*estados);
	int row;
	for (row=0; row<estados; row++) {
		tablaR[row] = (regla*)malloc(sizeof(regla) * (terminales + noTerminales + 1));
	}
	
    inicializaTabla(tablaR);
    
	// Arreglo gramatica
	gramatica = (derivacion *)malloc(sizeof(derivacion)*derivacionesGramatica);
	
    // PROGRAM -> GLOBAL_DECLARATIONS.
    gramatica[0].cadenaDerivacion[0] = "PROGRAM";
	gramatica[0].cadenaDerivacion[1] = "GLOBAL_DECLARATIONS";
	gramatica[0].derivaciones		 = 2;
    
    //GLOBAL_DECLARATIONS -> DECLARATION GLOBAL_DECLARATIONS | MAIN_DEF.
    gramatica[1].cadenaDerivacion[0] = "GLOBAL_DECLARATIONS";
	gramatica[1].cadenaDerivacion[1] = "DECLARATION";
	gramatica[1].cadenaDerivacion[2] = "GLOBAL_DECLARATIONS";
	gramatica[1].derivaciones		 = 3;
    
    gramatica[2].cadenaDerivacion[0] = "GLOBAL_DECLARATIONS";
	gramatica[2].cadenaDerivacion[1] = "MAIN_DEF";
	gramatica[2].derivaciones		 = 2;
    
    //DECLARATION -> VAR_TYPE VAR_LIST ;.
    gramatica[3].cadenaDerivacion[0] = "DECLARATION";
	gramatica[3].cadenaDerivacion[1] = "VAR_TYPE";
    gramatica[3].cadenaDerivacion[2] = "VAR_LIST";
	gramatica[3].derivaciones		 = 3;
    
    //VAR_TYPE -> void | int | float | char.
    gramatica[4].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[4].cadenaDerivacion[1] = "void";
    gramatica[4].derivaciones = 2;
    
    gramatica[5].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[5].cadenaDerivacion[1] = "int";
    gramatica[5].derivaciones = 2;
    
    gramatica[6].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[6].cadenaDerivacion[1] = "float";
    gramatica[6].derivaciones = 2;
    
    gramatica[7].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[7].cadenaDerivacion[1] = "char";
    gramatica[7].derivaciones = 2;
    
    // VAR_LIST -> VAR_LIST comma VAR_ITEM | VAR_ITEM
    gramatica[8].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[8].cadenaDerivacion[1] = "VAR_LIST";
    gramatica[8].cadenaDerivacion[2] = "comma";
    gramatica[8].cadenaDerivacion[3] = "VAR_ITEM";
    gramatica[8].derivaciones = 4;
    
    gramatica[9].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[9].cadenaDerivacion[1] = "VAR_ITEM";
    gramatica[9].derivaciones = 2;
    
    // VAR_ITEM -> ARRAY_VAR | SCALAR_VAR.
    gramatica[10].cadenaDerivacion[0] = "VAR_ITEM";
    gramatica[10].cadenaDerivacion[1] = "ARRAY_VAR";
    gramatica[10].derivaciones = 2;
    
    gramatica[11].cadenaDerivacion[0] = "VAR_ITEM";
    gramatica[11].cadenaDerivacion[1] = "SCALAR_VAR";
    gramatica[11].derivaciones = 2;
    
    // ARRAY_VAR -> var_name left_bracket integer_number right_bracket.
    gramatica[12].cadenaDerivacion[0] = "ARRAY_VAR";
    gramatica[12].cadenaDerivacion[1] = "var_name";
    gramatica[12].cadenaDerivacion[2] = "left_bracket";
    gramatica[12].cadenaDerivacion[3] = "integer_number";
    gramatica[12].cadenaDerivacion[4] = "right_bracket";
    gramatica[12].derivaciones = 5;
    
    // SCALAR_VAR -> var_name.
    gramatica[13].cadenaDerivacion[0] = "SCALAR_VAR";
    gramatica[13].cadenaDerivacion[1] = "var_name";
    gramatica[13].derivaciones = 2;
    
    
    // MAIN_DEF -> int main left_parenthesis right_parenthesis left_curly_bracket FUNCTION_BODY right_curly_bracket.
    gramatica[14].cadenaDerivacion[0] = "MAIN_DEF";
    gramatica[14].cadenaDerivacion[1] = "int";
    gramatica[14].cadenaDerivacion[2] = "main";
    gramatica[14].cadenaDerivacion[3] = "left_parenthesis";
    gramatica[14].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[14].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[14].cadenaDerivacion[6] = "FUNCTION_BODY";
    gramatica[14].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[14].derivaciones = 8;
    
    // FUNCTION_BODY -> INTERNAL_DECLARATIONS STATEMENT_LIST.
    gramatica[15].cadenaDerivacion[0] = "FUNCTION_BODY";
    gramatica[15].cadenaDerivacion[1] = "INTERNAL_DECLARATIONS";
    gramatica[15].cadenaDerivacion[2] = "STATEMENT_LIST";
    gramatica[15].derivaciones = 3;
    
    // INTERNAL_DECLARATIONS -> DECLARATION INTERNAL_DECLARATIONS | .
    gramatica[16].cadenaDerivacion[0] = "INTERNAL_DECLARATIONS";
    gramatica[16].cadenaDerivacion[1] = "DECLARATION";
    gramatica[16].cadenaDerivacion[2] = "INTERNAL_DECLARATIONS";
    gramatica[16].derivaciones = 3;
    
    // PENDIENTE
    
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST | .
    gramatica[17].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[17].cadenaDerivacion[1] = "STATEMENT_LIST";
    gramatica[17].derivaciones = 2;
    
    gramatica[18].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[18].cadenaDerivacion[1] = "epsilon";
    gramatica[18].derivaciones = 2;
    
    // STATEMENT -> IF_STATEMENT | EXPRESSION_STATEMENT | WHILE_STATEMENT | RETURN_STATEMENT | ;.
    gramatica[19].cadenaDerivacion[0] = "STATEMENT";
    gramatica[19].cadenaDerivacion[1] = "IF_STATEMENT";
    gramatica[19].derivaciones = 2;
    
    gramatica[20].cadenaDerivacion[0] = "STATEMENT";
    gramatica[20].cadenaDerivacion[1] = "EXPRESSION_STATEMENT";
    gramatica[20].derivaciones = 2;
    
    gramatica[21].cadenaDerivacion[0] = "STATEMENT";
    gramatica[21].cadenaDerivacion[1] = "WHILE_STATEMENT";
    gramatica[21].derivaciones = 2;
    
    gramatica[22].cadenaDerivacion[0] = "STATEMENT";
    gramatica[22].cadenaDerivacion[1] = "WHILE_STATEMENT";
    gramatica[22].derivaciones = 2;
    
    gramatica[23].cadenaDerivacion[0] = "STATEMENT";
    gramatica[23].cadenaDerivacion[1] = "RETURN_STATEMENT";
    gramatica[23].derivaciones = 2;
    
	gramatica[24].cadenaDerivacion[0] = "STATEMENT";
    gramatica[24].cadenaDerivacion[1] = ";";
    gramatica[24].derivaciones = 2;
    
    // WHILE_STATEMENT -> while left_parenthesis EXPRESSION right_parenthesis STATEMENT.
    gramatica[25].cadenaDerivacion[0] = "WHILE_STATEMENT";
    gramatica[25].cadenaDerivacion[1] = "while";
    gramatica[25].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[25].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[25].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[25].cadenaDerivacion[5] = "STATEMENT";
    gramatica[25].derivaciones = 6;
    
    // IF_STATEMENT -> if left_parenthesis EXPRESSION right_parenthesis STATEMENT | if left_parenthesis EXPRESSION right_parenthesis STATEMENT else STATEMENT.
    gramatica[26].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[26].cadenaDerivacion[1] = "if";
    gramatica[26].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[26].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[26].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[26].cadenaDerivacion[5] = "STATEMENT";
    gramatica[26].derivaciones = 6;
    
    gramatica[27].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[27].cadenaDerivacion[1] = "if";
    gramatica[27].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[27].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[27].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[27].cadenaDerivacion[5] = "STATEMENT";
    gramatica[27].cadenaDerivacion[6] = "else";
    gramatica[27].cadenaDerivacion[7] = "STATEMENT";
    gramatica[27].derivaciones = 8;
    
    // RETURN_STATEMENT -> return EXPRESSION ; | return ;.
    gramatica[28].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[28].cadenaDerivacion[1] = "return";
    gramatica[28].cadenaDerivacion[2] = "EXPRESSION";
    gramatica[28].cadenaDerivacion[3] = ";";
    gramatica[28].derivaciones = 4;
    
    gramatica[29].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[29].cadenaDerivacion[1] = "return";
    gramatica[29].cadenaDerivacion[2] = ";";
    gramatica[29].derivaciones = 3;
    
    // EXPRESSION_STATEMENT -> EXPRESSION ;.
    gramatica[30].cadenaDerivacion[0] = "EXPRESSION_STATEMENT";
    gramatica[30].cadenaDerivacion[1] = "EXPRESSION";
    gramatica[30].cadenaDerivacion[2] = ";";
    gramatica[30].derivaciones = 3;
    
    
    // EXPRESSION -> ASSIGNMENT_EXPRESSION.
    gramatica[31].cadenaDerivacion[0] = "EXPRESSION";
    gramatica[31].cadenaDerivacion[1] = "ASSIGNMENT_EXPRESSION";
    gramatica[31].derivaciones = 2;
    
    // ASSIGNMENT_EXPRESSION -> BINARY_EXPR.
    gramatica[32].cadenaDerivacion[0] = "ASSIGNMENT_EXPRESSION";
    gramatica[32].cadenaDerivacion[1] = "BINARY_EXPR";
    gramatica[32].derivaciones = 2;
    
    // BINARY_EXPR -> BINARY_EXPR BINARY_OP PRIMARY_EXPR | PRIMARY_EXPR.
    gramatica[33].cadenaDerivacion[0] = "BINARY_EXPR";
    gramatica[33].cadenaDerivacion[1] = "BINARY_EXPR";
    gramatica[33].cadenaDerivacion[2] = "BINARY_OP";
    gramatica[33].cadenaDerivacion[3] = "PRIMARY_EXPR";
    gramatica[33].derivaciones = 4;
    
    gramatica[34].cadenaDerivacion[0] = "BINARY_EXPR";
    gramatica[34].cadenaDerivacion[1] = "PRIMARY_EXPR";
    gramatica[34].derivaciones = 2;
    
    // BINARY_OP -> BOOLEAN_OP | REL_OP | ARITH_OP.
    gramatica[35].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[35].cadenaDerivacion[1] = "BOOLEAN_OP";
    gramatica[35].derivaciones = 2;
    
    gramatica[36].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[36].cadenaDerivacion[1] = "REL_OP";
    gramatica[36].derivaciones = 2;
    
    gramatica[37].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[37].cadenaDerivacion[1] = "ARITH_OP";
    gramatica[37].derivaciones = 2;
    
    // BOOLEAN_OP -> ampersan ampersan | or_symbol or_symbol.
    gramatica[36].cadenaDerivacion[0] = "BOOLEAN_OP";
    gramatica[36].cadenaDerivacion[1] = "ampersan";
    gramatica[36].cadenaDerivacion[2] = "ampersan";
    gramatica[36].derivaciones = 3;
    
    gramatica[38].cadenaDerivacion[0] = "BOOLEAN_OP";
    gramatica[38].cadenaDerivacion[1] = "or_symbol";
    gramatica[38].cadenaDerivacion[2] = "or_symbol";
    gramatica[38].derivaciones = 3;
    
    // REL_OP -> equal equal | not equal | minus_than | more_than equal | minus_than | minus_than equal.
    gramatica[39].cadenaDerivacion[0] = "REL_OP";
    gramatica[39].cadenaDerivacion[1] = "equal";
    gramatica[39].cadenaDerivacion[2] = "equal";
    gramatica[39].derivaciones = 3;
    
    gramatica[40].cadenaDerivacion[0] = "REL_OP";
    gramatica[40].cadenaDerivacion[1] = "not";
    gramatica[40].cadenaDerivacion[2] = "equal";
    gramatica[40].derivaciones = 3;
    
    gramatica[41].cadenaDerivacion[0] = "REL_OP";
    gramatica[41].cadenaDerivacion[1] = "minus_than";
    gramatica[41].derivaciones = 2;
    
    gramatica[42].cadenaDerivacion[0] = "REL_OP";
    gramatica[42].cadenaDerivacion[1] = "more_than";
    gramatica[42].cadenaDerivacion[2] = "equal";
    gramatica[42].derivaciones = 3;
    
    gramatica[43].cadenaDerivacion[0] = "REL_OP";
    gramatica[43].cadenaDerivacion[1] = "minus_than";
    gramatica[43].derivaciones = 2;
    
    gramatica[44].cadenaDerivacion[0] = "REL_OP";
    gramatica[44].cadenaDerivacion[1] = "minus_than";
    gramatica[44].cadenaDerivacion[2] = "equal";
    gramatica[44].derivaciones = 3;
    
    
    // ARITH_OP -> plus_sign | minus_sign | multiply_sign | divide_sign | mod_sign.
    gramatica[45].cadenaDerivacion[0] = "ARITH_OP";
    gramatica[45].cadenaDerivacion[1] = "plus_sign";
    gramatica[45].derivaciones = 2;
    
    gramatica[46].cadenaDerivacion[0] = "ARITH_OP";
    gramatica[46].cadenaDerivacion[1] = "minus_sign";
    gramatica[46].derivaciones = 2;
    
    gramatica[47].cadenaDerivacion[0] = "ARITH_OP";
    gramatica[47].cadenaDerivacion[1] = "multiply_sign";
    gramatica[47].derivaciones = 2;
    
    gramatica[48].cadenaDerivacion[0] = "ARITH_OP";
    gramatica[48].cadenaDerivacion[1] = "divide_sign";
    gramatica[48].derivaciones = 2;
    
    gramatica[49].cadenaDerivacion[0] = "ARITH_OP";
    gramatica[49].cadenaDerivacion[1] = "mod_sign";
    gramatica[49].derivaciones = 2;
    
    // PRIMARY_EXPR -> var_name  |  CONSTANT  |  left_parenthesis EXPRESSION right_parenthesis.
    gramatica[50].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[50].cadenaDerivacion[1] = "var_name";
    gramatica[50].derivaciones = 2;
    
    gramatica[51].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[51].cadenaDerivacion[1] = "CONSTANT";
    gramatica[51].derivaciones = 2;
    
    gramatica[52].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[52].cadenaDerivacion[1] = "left_parenthesis";
    gramatica[52].cadenaDerivacion[2] = "EXPRESSION";
    gramatica[52].cadenaDerivacion[3] = "right_parenthesis";
    gramatica[52].derivaciones = 4;
    
    // CONSTANT -> integer_number |  float_number |  character | string.
    gramatica[53].cadenaDerivacion[0] = "CONSTANT";
    gramatica[53].cadenaDerivacion[1] = "integer_number";
    gramatica[53].derivaciones = 2;
    
    gramatica[53].cadenaDerivacion[0] = "CONSTANT";
    gramatica[53].cadenaDerivacion[1] = "float_number";
    gramatica[53].derivaciones = 2;
    
    gramatica[54].cadenaDerivacion[0] = "CONSTANT";
    gramatica[54].cadenaDerivacion[1] = "character";
    gramatica[54].derivaciones = 2;
    
    gramatica[55].cadenaDerivacion[0] = "CONSTANT";
    gramatica[55].cadenaDerivacion[1] = "string";
    gramatica[55].derivaciones = 2;
    
	// Inicializa la tabla con valores default
	inicializaTabla(tablaR);
    
	// Tabla
    tablaR[0][27].tipo = D;
    tablaR[0][27].valor = 9;
    
    tablaR[0][31].tipo = D;
    tablaR[0][31].valor = 8;
    
    tablaR[0][32].tipo = D;
    tablaR[0][32].valor = 7;
    
    tablaR[0][33].tipo = D;
    tablaR[0][33].valor = 6;
    
    tablaR[0][34].tipo = D;
    tablaR[0][34].valor = 5;
    
    tablaR[0][35].tipo = D;
    tablaR[0][35].valor = 4;
    
    tablaR[0][36].tipo = D;
    tablaR[0][36].valor = 3;
    
    tablaR[0][41].tipo = D;
    tablaR[0][41].valor = 2;
    
    tablaR[0][43].tipo = D;
    tablaR[0][43].valor = 1;
    
    tablaR[1][27].tipo = D;
    tablaR[1][27].valor = 9;
    
    tablaR[1][31].tipo = D;
    tablaR[1][31].valor = 8;
    
    tablaR[1][32].tipo = D;
    tablaR[1][32].valor = 7;
    
    tablaR[1][33].tipo = D;
    tablaR[1][33].valor = 6;
    
    tablaR[1][35].tipo = D;
    tablaR[1][35].valor = 16;
    
    tablaR[1][36].tipo = D;
    tablaR[1][36].valor = 3;
    
    tablaR[1][41].tipo = D;
    tablaR[1][41].valor = 2;
    
    tablaR[1][43].tipo = D;
    tablaR[1][43].valor = 1;
    
    tablaR[2][0].tipo = R;
    tablaR[2][0].valor = 2;
    
    tablaR[3][7].tipo = D;
    tablaR[3][7].valor = 15;
    
    tablaR[3][37].tipo = D;
    tablaR[3][37].valor = 14;
    
    tablaR[3][38].tipo = D;
    tablaR[3][38].valor = 13;
    
    tablaR[3][39].tipo = D;
    tablaR[3][39].valor = 12;
    
    tablaR[3][40].tipo = D;
    tablaR[3][40].valor = 11;
    
    tablaR[4][0].tipo = R;
    tablaR[4][0].valor = 0;
    
    tablaR[5][0].tipo = ACEPTA;
    tablaR[5][0].valor = ERR;
    
    tablaR[6][7].tipo = R;
    tablaR[6][7].valor = 4;
    
    tablaR[7][7].tipo = R;
    tablaR[7][7].valor = 6;
    
    tablaR[8][7].tipo = R;
    tablaR[8][7].valor = 7;
    
    tablaR[9][7].tipo = R;
    tablaR[9][7].valor = 5;
    
    tablaR[9][26].tipo = D;
    tablaR[9][26].valor = 10;
    
    tablaR[10][6].tipo = D;
    tablaR[10][6].valor = 20;
    
    tablaR[11][19].tipo = R;
    tablaR[11][19].valor = 11;
    
    tablaR[11][30].tipo = R;
    tablaR[11][30].valor = 11;
    
    tablaR[12][19].tipo = R;
    tablaR[12][19].valor = 10;
    
    tablaR[12][30].tipo = R;
    tablaR[12][30].valor = 10;
    
    tablaR[13][19].tipo = R;
    tablaR[13][19].valor = 9;
    
    tablaR[13][30].tipo = R;
    tablaR[13][30].valor = 9;
    
    tablaR[14][19].tipo = D;
    tablaR[14][19].valor = 19;
    
    tablaR[14][30].tipo = D;
    tablaR[14][30].valor = 18;
    
    tablaR[15][19].tipo = R;
    tablaR[15][19].valor = 13;
    
    tablaR[15][29].tipo = D;
    tablaR[15][29].valor = 17;
    
    tablaR[15][30].tipo = R;
    tablaR[15][30].valor = 13;
    
    tablaR[16][0].tipo = R;
    tablaR[16][0].valor = 1;
    
    tablaR[17][4].tipo = D;
    tablaR[17][4].valor = 23;
    
    tablaR[18][7].tipo = D;
    tablaR[18][7].valor = 15;
    
    tablaR[18][38].tipo = D;
    tablaR[18][38].valor = 22;
    
    tablaR[18][39].tipo = D;
    tablaR[18][39].valor = 12;
    
    tablaR[18][40].tipo = D;
    tablaR[18][40].valor = 11;
    
    tablaR[19][1].tipo = R;
    tablaR[19][1].valor = 3;
    
    tablaR[19][2].tipo = R;
    tablaR[19][2].valor = 3;
    
    tablaR[19][3].tipo = R;
    tablaR[19][3].valor = 3;
    
    tablaR[19][4].tipo = R;
    tablaR[19][4].valor = 3;
    
    tablaR[19][6].tipo = R;
    tablaR[19][6].valor = 3;
    
    tablaR[19][7].tipo = R;
    tablaR[19][7].valor = 3;
    
    tablaR[19][19].tipo = R;
    tablaR[19][19].valor = 3;
    
    tablaR[19][20].tipo = R;
    tablaR[19][20].valor = 3;
    
    tablaR[19][22].tipo = R;
    tablaR[19][22].valor = 3;
    
    tablaR[19][23].tipo = R;
    tablaR[19][23].valor = 3;
    
    tablaR[19][24].tipo = R;
    tablaR[19][24].valor = 3;
    
    tablaR[19][27].tipo = R;
    tablaR[19][27].valor = 3;
    
    tablaR[19][31].tipo = R;
    tablaR[19][31].valor = 3;
    
    tablaR[19][32].tipo = R;
    tablaR[19][32].valor = 3;
    
    tablaR[19][33].tipo = R;
    tablaR[19][33].valor = 3;
    
    tablaR[20][5].tipo = D;
    tablaR[20][5].valor = 21;
    
    tablaR[21][25].tipo = D;
    tablaR[21][25].valor = 25;
    
    tablaR[22][19].tipo = R;
    tablaR[22][19].valor = 8;
    
    tablaR[22][30].tipo = R;
    tablaR[22][30].valor = 8;
    
    tablaR[23][28].tipo = D;
    tablaR[23][28].valor = 24;
    
    tablaR[24][19].tipo = R;
    tablaR[24][19].valor = 12;
    
    tablaR[24][30].tipo = R;
    tablaR[24][30].valor = 12;
    
    tablaR[25][1].tipo = R;
    tablaR[25][1].valor = 17;
    
    tablaR[25][2].tipo = R;
    tablaR[25][2].valor = 17;
    
    tablaR[25][3].tipo = R;
    tablaR[25][3].valor = 17;
    
    tablaR[25][4].tipo = R;
    tablaR[25][4].valor = 17;
    
    tablaR[25][6].tipo = R;
    tablaR[25][6].valor = 17;
    
    tablaR[25][7].tipo = R;
    tablaR[25][7].valor = 17;
    
    tablaR[25][19].tipo = R;
    tablaR[25][19].valor = 17;
    
    tablaR[25][20].tipo = R;
    tablaR[25][20].valor = 17;
    
    tablaR[25][22].tipo = R;
    tablaR[25][22].valor = 17;
    
    tablaR[25][23].tipo = R;
    tablaR[25][23].valor = 17;
    
    tablaR[25][24].tipo = R;
    tablaR[25][24].valor = 17;
    
    tablaR[25][27].tipo = D;
    tablaR[25][27].valor = 29;
    
    tablaR[25][31].tipo = D;
    tablaR[25][31].valor = 8;
    
    tablaR[25][32].tipo = D;
    tablaR[25][32].valor = 7;
    
    tablaR[25][33].tipo = D;
    tablaR[25][33].valor = 6;
    
    tablaR[25][36].tipo = D;
    tablaR[25][36].valor = 3;
    
    tablaR[25][42].tipo = D;
    tablaR[25][42].valor = 28;
    
    tablaR[25][43].tipo = D;
    tablaR[25][43].valor = 27;
    
    tablaR[25][44].tipo = D;
    tablaR[25][44].valor = 26;
    
    tablaR[26][1].tipo = D;
    tablaR[26][1].valor = 52;
    
    tablaR[26][2].tipo = D;
    tablaR[26][2].valor = 51;
    
    tablaR[26][3].tipo = D;
    tablaR[26][3].valor = 50;
    
    tablaR[26][4].tipo = D;
    tablaR[26][4].valor = 49;
    
    tablaR[26][6].tipo = D;
    tablaR[26][6].valor = 48;
    
    tablaR[26][7].tipo = D;
    tablaR[26][7].valor = 47;
    
    tablaR[26][19].tipo = D;
    tablaR[26][19].valor = 46;
    
    tablaR[26][20].tipo = D;
    tablaR[26][20].valor = 45;
    
    tablaR[26][22].tipo = D;
    tablaR[26][22].valor = 44;
    
    tablaR[26][23].tipo = D;
    tablaR[26][23].valor = 43;
    
    tablaR[26][24].tipo = R;
    tablaR[26][24].valor = 19;
    
    tablaR[26][45].tipo = D;
    tablaR[26][45].valor = 42;
    
    tablaR[26][46].tipo = D;
    tablaR[26][46].valor = 41;
    
    tablaR[26][47].tipo = D;
    tablaR[26][47].valor = 40;
    
    tablaR[26][48].tipo = D;
    tablaR[26][48].valor = 39;
    
    tablaR[26][49].tipo = D;
    tablaR[26][49].valor = 38;
    
    tablaR[26][50].tipo = D;
    tablaR[26][50].valor = 37;
    
    tablaR[26][51].tipo = D;
    tablaR[26][51].valor = 36;
    
    tablaR[26][52].tipo = D;
    tablaR[26][52].valor = 35;
    
    tablaR[26][57].tipo = D;
    tablaR[26][57].valor = 34;
    
    tablaR[26][58].tipo = D;
    tablaR[26][58].valor = 33;
    
    tablaR[26][59].tipo = D;
    tablaR[26][59].valor = 32;
    
    tablaR[27][1].tipo = R;
    tablaR[27][1].valor = 17;
    
    tablaR[27][2].tipo = R;
    tablaR[27][2].valor = 17;
    
    tablaR[27][3].tipo = R;
    tablaR[27][3].valor = 17;
    
    tablaR[27][4].tipo = R;
    tablaR[27][4].valor = 17;
    
    tablaR[27][6].tipo = R;
    tablaR[27][6].valor = 17;
    
    tablaR[27][7].tipo = R;
    tablaR[27][7].valor = 17;
    
    tablaR[27][19].tipo = R;
    tablaR[27][19].valor = 17;
    
    tablaR[27][20].tipo = R;
    tablaR[27][20].valor = 17;
    
    tablaR[27][22].tipo = R;
    tablaR[27][22].valor = 17;
    
    tablaR[27][23].tipo = R;
    tablaR[27][23].valor = 17;
    
    tablaR[27][24].tipo = R;
    tablaR[27][24].valor = 17;
    
    tablaR[27][27].tipo = D;
    tablaR[27][27].valor = 29;
    
    tablaR[27][31].tipo = D;
    tablaR[27][31].valor = 8;
    
    tablaR[27][32].tipo = D;
    tablaR[27][32].valor = 7;
    
    tablaR[27][33].tipo = D;
    tablaR[27][33].valor = 6;
    
    tablaR[27][36].tipo = D;
    tablaR[27][36].valor = 3;
    
    tablaR[27][43].tipo = D;
    tablaR[27][43].valor = 27;
    
    tablaR[27][44].tipo = D;
    tablaR[27][44].valor = 31;
    
    tablaR[28][24].tipo = D;
    tablaR[28][24].valor = 30;
    
    tablaR[29][7].tipo = R;
    tablaR[29][7].valor = 5;
    
    tablaR[30][0].tipo = R;
    tablaR[30][0].valor = 14;
    
    tablaR[31][1].tipo = R;
    tablaR[31][1].valor = 16;
    
    tablaR[31][2].tipo = R;
    tablaR[31][2].valor = 16;
    
    tablaR[31][3].tipo = R;
    tablaR[31][3].valor = 16;
    
    tablaR[31][4].tipo = R;
    tablaR[31][4].valor = 16;
    
    tablaR[31][6].tipo = R;
    tablaR[31][6].valor = 16;
    
    tablaR[31][7].tipo = R;
    tablaR[31][7].valor = 16;
    
    tablaR[31][19].tipo = R;
    tablaR[31][19].valor = 16;
    
    tablaR[31][20].tipo = R;
    tablaR[31][20].valor = 16;
    
    tablaR[31][22].tipo = R;
    tablaR[31][22].valor = 16;
    
    tablaR[31][23].tipo = R;
    tablaR[31][23].valor = 16;
    
    tablaR[31][24].tipo = R;
    tablaR[31][24].valor = 16;
    
    tablaR[32][5].tipo = R;
    tablaR[32][5].valor = 52;
    
    tablaR[32][8].tipo = R;
    tablaR[32][8].valor = 52;
    
    tablaR[32][9].tipo = R;
    tablaR[32][9].valor = 52;
    
    tablaR[32][10].tipo = R;
    tablaR[32][10].valor = 52;
    
    tablaR[32][11].tipo = R;
    tablaR[32][11].valor = 52;
    
    tablaR[32][12].tipo = R;
    tablaR[32][12].valor = 52;
    
    tablaR[32][13].tipo = R;
    tablaR[32][13].valor = 52;
    
    tablaR[32][14].tipo = R;
    tablaR[32][14].valor = 52;
    
    tablaR[32][15].tipo = R;
    tablaR[32][15].valor = 52;
    
    tablaR[32][16].tipo = R;
    tablaR[32][16].valor = 52;
    
    tablaR[32][17].tipo = R;
    tablaR[32][17].valor = 52;
    
    tablaR[32][18].tipo = R;
    tablaR[32][18].valor = 52;
    
    tablaR[32][19].tipo = R;
    tablaR[32][19].valor = 52;
    
    tablaR[33][19].tipo = D;
    tablaR[33][19].valor = 74;
    
    tablaR[34][5].tipo = R;
    tablaR[34][5].valor = 34;
    
    tablaR[34][8].tipo = R;
    tablaR[34][8].valor = 34;
    
    tablaR[34][9].tipo = R;
    tablaR[34][9].valor = 34;
    
    tablaR[34][10].tipo = R;
    tablaR[34][10].valor = 34;
    
    tablaR[34][11].tipo = R;
    tablaR[34][11].valor = 34;
    
    tablaR[34][12].tipo = R;
    tablaR[34][12].valor = 34;
    
    tablaR[34][13].tipo = R;
    tablaR[34][13].valor = 34;
    
    tablaR[34][14].tipo = R;
    tablaR[34][14].valor = 34;
    
    tablaR[34][15].tipo = R;
    tablaR[34][15].valor = 34;
    
    tablaR[34][16].tipo = R;
    tablaR[34][16].valor = 34;
    
    tablaR[34][17].tipo = R;
    tablaR[34][17].valor = 34;
    
    tablaR[34][18].tipo = R;
    tablaR[34][18].valor = 34;
    
    tablaR[34][19].tipo = R;
    tablaR[34][19].valor = 34;
    
    tablaR[35][5].tipo = R;
    tablaR[35][5].valor = 32;
    
    tablaR[35][8].tipo = D;
    tablaR[35][8].valor = 73;
    
    tablaR[35][9].tipo = D;
    tablaR[35][9].valor = 72;
    
    tablaR[35][10].tipo = D;
    tablaR[35][10].valor = 71;
    
    tablaR[35][11].tipo = D;
    tablaR[35][11].valor = 70;
    
    tablaR[35][12].tipo = D;
    tablaR[35][12].valor = 69;
    
    tablaR[35][13].tipo = D;
    tablaR[35][13].valor = 68;
    
    tablaR[35][14].tipo = D;
    tablaR[35][14].valor = 67;
    
    tablaR[35][15].tipo = D;
    tablaR[35][15].valor = 66;
    
    tablaR[35][16].tipo = D;
    tablaR[35][16].valor = 65;
    
    tablaR[35][17].tipo = D;
    tablaR[35][17].valor = 64;
    
    tablaR[35][18].tipo = D;
    tablaR[35][18].valor = 63;
    
    tablaR[35][19].tipo = R;
    tablaR[35][19].valor = 32;
    
    tablaR[35][53].tipo = D;
    tablaR[35][53].valor = 62;
    
    tablaR[35][54].tipo = D;
    tablaR[35][54].valor = 61;
    
    tablaR[35][55].tipo = D;
    tablaR[35][55].valor = 60;
    
    tablaR[35][56].tipo = D;
    tablaR[35][56].valor = 59;
    
    tablaR[36][5].tipo = R;
    tablaR[36][5].valor = 31;
    
    tablaR[36][19].tipo = R;
    tablaR[36][19].valor = 31;
    
    tablaR[37][1].tipo = R;
    tablaR[37][1].valor = 21;
    
    tablaR[37][2].tipo = R;
    tablaR[37][2].valor = 21;
    
    tablaR[37][3].tipo = R;
    tablaR[37][3].valor = 21;
    
    tablaR[37][4].tipo = R;
    tablaR[37][4].valor = 21;
    
    tablaR[37][6].tipo = R;
    tablaR[37][6].valor = 21;
    
    tablaR[37][7].tipo = R;
    tablaR[37][7].valor = 21;
    
    tablaR[37][19].tipo = R;
    tablaR[37][19].valor = 21;
    
    tablaR[37][20].tipo = R;
    tablaR[37][20].valor = 21;
    
    tablaR[37][21].tipo = R;
    tablaR[37][21].valor = 21;
    
    tablaR[37][22].tipo = R;
    tablaR[37][22].valor = 21;
    
    tablaR[37][23].tipo = R;
    tablaR[37][23].valor = 21;
    
    tablaR[37][24].tipo = R;
    tablaR[37][24].valor = 21;
    
    tablaR[38][1].tipo = R;
    tablaR[38][1].valor = 23;
    
    tablaR[38][2].tipo = R;
    tablaR[38][2].valor = 23;
    
    tablaR[38][3].tipo = R;
    tablaR[38][3].valor = 23;
    
    tablaR[38][4].tipo = R;
    tablaR[38][4].valor = 23;
    
    tablaR[38][6].tipo = R;
    tablaR[38][6].valor = 23;
    
    tablaR[38][7].tipo = R;
    tablaR[38][7].valor = 23;
    
    tablaR[38][19].tipo = R;
    tablaR[38][19].valor = 23;
    
    tablaR[38][20].tipo = R;
    tablaR[38][20].valor = 23;
    
    tablaR[38][21].tipo = R;
    tablaR[38][21].valor = 23;
    
    tablaR[38][22].tipo = R;
    tablaR[38][22].valor = 23;
    
    tablaR[38][23].tipo = R;
    tablaR[38][23].valor = 23;
    
    tablaR[38][24].tipo = R;
    tablaR[38][24].valor = 23;
    
    tablaR[39][1].tipo = D;
    tablaR[39][1].valor = 52;
    
    tablaR[39][2].tipo = D;
    tablaR[39][2].valor = 51;
    
    tablaR[39][3].tipo = D;
    tablaR[39][3].valor = 50;
    
    tablaR[39][4].tipo = D;
    tablaR[39][4].valor = 49;
    
    tablaR[39][6].tipo = D;
    tablaR[39][6].valor = 48;
    
    tablaR[39][7].tipo = D;
    tablaR[39][7].valor = 47;
    
    tablaR[39][19].tipo = D;
    tablaR[39][19].valor = 46;
    
    tablaR[39][20].tipo = D;
    tablaR[39][20].valor = 45;
    
    tablaR[39][22].tipo = D;
    tablaR[39][22].valor = 44;
    
    tablaR[39][23].tipo = D;
    tablaR[39][23].valor = 43;
    
    tablaR[39][24].tipo = R;
    tablaR[39][24].valor = 19;
    
    tablaR[39][45].tipo = D;
    tablaR[39][45].valor = 58;
    
    tablaR[39][46].tipo = D;
    tablaR[39][46].valor = 41;
    
    tablaR[39][47].tipo = D;
    tablaR[39][47].valor = 40;
    
    tablaR[39][48].tipo = D;
    tablaR[39][48].valor = 39;
    
    tablaR[39][49].tipo = D;
    tablaR[39][49].valor = 38;
    
    tablaR[39][50].tipo = D;
    tablaR[39][50].valor = 37;
    
    tablaR[39][51].tipo = D;
    tablaR[39][51].valor = 36;
    
    tablaR[39][52].tipo = D;
    tablaR[39][52].valor = 35;
    
    tablaR[39][57].tipo = D;
    tablaR[39][57].valor = 34;
    
    tablaR[39][58].tipo = D;
    tablaR[39][58].valor = 33;
    
    tablaR[39][59].tipo = D;
    tablaR[39][59].valor = 32;
    
    tablaR[40][1].tipo = R;
    tablaR[40][1].valor = 20;
    
    tablaR[40][2].tipo = R;
    tablaR[40][2].valor = 20;
    
    tablaR[40][3].tipo = R;
    tablaR[40][3].valor = 20;
    
    tablaR[40][4].tipo = R;
    tablaR[40][4].valor = 20;
    
    tablaR[40][6].tipo = R;
    tablaR[40][6].valor = 20;
    
    tablaR[40][7].tipo = R;
    tablaR[40][7].valor = 20;
    
    tablaR[40][19].tipo = R;
    tablaR[40][19].valor = 20;
    
    tablaR[40][20].tipo = R;
    tablaR[40][20].valor = 20;
    
    tablaR[40][21].tipo = R;
    tablaR[40][21].valor = 20;
    
    tablaR[40][22].tipo = R;
    tablaR[40][22].valor = 20;
    
    tablaR[40][23].tipo = R;
    tablaR[40][23].valor = 20;
    
    tablaR[40][24].tipo = R;
    tablaR[40][24].valor = 20;
    
    tablaR[41][1].tipo = R;
    tablaR[41][1].valor = 22;
    
    tablaR[41][2].tipo = R;
    tablaR[41][2].valor = 22;
    
    tablaR[41][3].tipo = R;
    tablaR[41][3].valor = 22;
    
    tablaR[41][4].tipo = R;
    tablaR[41][4].valor = 22;
    
    tablaR[41][6].tipo = R;
    tablaR[41][6].valor = 22;
    
    tablaR[41][7].tipo = R;
    tablaR[41][7].valor = 22;
    
    tablaR[41][19].tipo = R;
    tablaR[41][19].valor = 22;
    
    tablaR[41][20].tipo = R;
    tablaR[41][20].valor = 22;
    
    tablaR[41][21].tipo = R;
    tablaR[41][21].valor = 22;
    
    tablaR[41][22].tipo = R;
    tablaR[41][22].valor = 22;
    
    tablaR[41][23].tipo = R;
    tablaR[41][23].valor = 22;
    
    tablaR[41][24].tipo = R;
    tablaR[41][24].valor = 22;
    
    tablaR[42][24].tipo = R;
    tablaR[42][24].valor = 15;
    
    tablaR[43][6].tipo = D;
    tablaR[43][6].valor = 57;
    
    tablaR[44][6].tipo = D;
    tablaR[44][6].valor = 56;
    
    tablaR[45][1].tipo = D;
    tablaR[45][1].valor = 52;
    
    tablaR[45][2].tipo = D;
    tablaR[45][2].valor = 51;
    
    tablaR[45][3].tipo = D;
    tablaR[45][3].valor = 50;
    
    tablaR[45][4].tipo = D;
    tablaR[45][4].valor = 49;
    
    tablaR[45][6].tipo = D;
    tablaR[45][6].valor = 48;
    
    tablaR[45][7].tipo = D;
    tablaR[45][7].valor = 47;
    
    tablaR[45][19].tipo = D;
    tablaR[45][19].valor = 55;
    
    tablaR[45][51].tipo = D;
    tablaR[45][51].valor = 36;
    
    tablaR[45][52].tipo = D;
    tablaR[45][52].valor = 35;
    
    tablaR[45][57].tipo = D;
    tablaR[45][57].valor = 34;
    
    tablaR[45][58].tipo = D;
    tablaR[45][58].valor = 54;
    
    tablaR[45][59].tipo = D;
    tablaR[45][59].valor = 32;
    
    tablaR[46][1].tipo = R;
    tablaR[46][1].valor = 24;
    
    tablaR[46][2].tipo = R;
    tablaR[46][2].valor = 24;
    
    tablaR[46][3].tipo = R;
    tablaR[46][3].valor = 24;
    
    tablaR[46][4].tipo = R;
    tablaR[46][4].valor = 24;
    
    tablaR[46][6].tipo = R;
    tablaR[46][6].valor = 24;
    
    tablaR[46][7].tipo = R;
    tablaR[46][7].valor = 24;
    
    tablaR[46][19].tipo = R;
    tablaR[46][19].valor = 24;
    
    tablaR[46][20].tipo = R;
    tablaR[46][20].valor = 24;
    
    tablaR[46][21].tipo = R;
    tablaR[46][21].valor = 24;
    
    tablaR[46][22].tipo = R;
    tablaR[46][22].valor = 24;
    
    tablaR[46][23].tipo = R;
    tablaR[46][23].valor = 24;
    
    tablaR[46][24].tipo = R;
    tablaR[46][24].valor = 24;
    
    tablaR[47][5].tipo = R;
    tablaR[47][5].valor = 51;
    
    tablaR[47][8].tipo = R;
    tablaR[47][8].valor = 51;
    
    tablaR[47][9].tipo = R;
    tablaR[47][9].valor = 51;
    
    tablaR[47][10].tipo = R;
    tablaR[47][10].valor = 51;
    
    tablaR[47][11].tipo = R;
    tablaR[47][11].valor = 51;
    
    tablaR[47][12].tipo = R;
    tablaR[47][12].valor = 51;
    
    tablaR[47][13].tipo = R;
    tablaR[47][13].valor = 51;
    
    tablaR[47][14].tipo = R;
    tablaR[47][14].valor = 51;
    
    tablaR[47][15].tipo = R;
    tablaR[47][15].valor = 51;
    
    tablaR[47][16].tipo = R;
    tablaR[47][16].valor = 51;
    
    tablaR[47][17].tipo = R;
    tablaR[47][17].valor = 51;
    
    tablaR[47][18].tipo = R;
    tablaR[47][18].valor = 51;
    
    tablaR[47][19].tipo = R;
    tablaR[47][19].valor = 51;
    
    tablaR[48][1].tipo = D;
    tablaR[48][1].valor = 52;
    
    tablaR[48][2].tipo = D;
    tablaR[48][2].valor = 51;
    
    tablaR[48][3].tipo = D;
    tablaR[48][3].valor = 50;
    
    tablaR[48][4].tipo = D;
    tablaR[48][4].valor = 49;
    
    tablaR[48][6].tipo = D;
    tablaR[48][6].valor = 48;
    
    tablaR[48][7].tipo = D;
    tablaR[48][7].valor = 47;
    
    tablaR[48][51].tipo = D;
    tablaR[48][51].valor = 36;
    
    tablaR[48][52].tipo = D;
    tablaR[48][52].valor = 35;
    
    tablaR[48][57].tipo = D;
    tablaR[48][57].valor = 34;
    
    tablaR[48][58].tipo = D;
    tablaR[48][58].valor = 53;
    
    tablaR[48][59].tipo = D;
    tablaR[48][59].valor = 32;
    
    tablaR[49][5].tipo = R;
    tablaR[49][5].valor = 54;
    
    tablaR[49][8].tipo = R;
    tablaR[49][8].valor = 54;
    
    tablaR[49][9].tipo = R;
    tablaR[49][9].valor = 54;
    
    tablaR[49][10].tipo = R;
    tablaR[49][10].valor = 54;
    
    tablaR[49][11].tipo = R;
    tablaR[49][11].valor = 54;
    
    tablaR[49][12].tipo = R;
    tablaR[49][12].valor = 54;
    
    tablaR[49][13].tipo = R;
    tablaR[49][13].valor = 54;
    
    tablaR[49][14].tipo = R;
    tablaR[49][14].valor = 54;
    
    tablaR[49][15].tipo = R;
    tablaR[49][15].valor = 54;
    
    tablaR[49][16].tipo = R;
    tablaR[49][16].valor = 54;
    
    tablaR[49][17].tipo = R;
    tablaR[49][17].valor = 54;
    
    tablaR[49][18].tipo = R;
    tablaR[49][18].valor = 54;
    
    tablaR[49][19].tipo = R;
    tablaR[49][19].valor = 54;
    
    tablaR[50][5].tipo = R;
    tablaR[50][5].valor = 55;
    
    tablaR[50][8].tipo = R;
    tablaR[50][8].valor = 55;
    
    tablaR[50][9].tipo = R;
    tablaR[50][9].valor = 55;
    
    tablaR[50][10].tipo = R;
    tablaR[50][10].valor = 55;
    
    tablaR[50][11].tipo = R;
    tablaR[50][11].valor = 55;
    
    tablaR[50][12].tipo = R;
    tablaR[50][12].valor = 55;
    
    tablaR[50][13].tipo = R;
    tablaR[50][13].valor = 55;
    
    tablaR[50][14].tipo = R;
    tablaR[50][14].valor = 55;
    
    tablaR[50][15].tipo = R;
    tablaR[50][15].valor = 55;
    
    tablaR[50][16].tipo = R;
    tablaR[50][16].valor = 55;
    
    tablaR[50][17].tipo = R;
    tablaR[50][17].valor = 55;
    
    tablaR[50][18].tipo = R;
    tablaR[50][18].valor = 55;
    
    tablaR[50][19].tipo = R;
    tablaR[50][19].valor = 55;
    
    tablaR[51][5].tipo = R;
    tablaR[51][5].valor = 56;
    
    tablaR[51][8].tipo = R;
    tablaR[51][8].valor = 56;
    
    tablaR[51][9].tipo = R;
    tablaR[51][9].valor = 56;
    
    tablaR[51][10].tipo = R;
    tablaR[51][10].valor = 56;
    
    tablaR[51][11].tipo = R;
    tablaR[51][11].valor = 56;
    
    tablaR[51][12].tipo = R;
    tablaR[51][12].valor = 56;
    
    tablaR[51][13].tipo = R;
    tablaR[51][13].valor = 56;
    
    tablaR[51][14].tipo = R;
    tablaR[51][14].valor = 56;
    
    tablaR[51][15].tipo = R;
    tablaR[51][15].valor = 56;
    
    tablaR[51][16].tipo = R;
    tablaR[51][16].valor = 56;
    
    tablaR[51][17].tipo = R;
    tablaR[51][17].valor = 56;
    
    tablaR[51][18].tipo = R;
    tablaR[51][18].valor = 56;
    
    tablaR[51][19].tipo = R;
    tablaR[51][19].valor = 56;
    
    tablaR[52][5].tipo = R;
    tablaR[52][5].valor = 57;
    
    tablaR[52][8].tipo = R;
    tablaR[52][8].valor = 57;
    
    tablaR[52][9].tipo = R;
    tablaR[52][9].valor = 57;
    
    tablaR[52][10].tipo = R;
    tablaR[52][10].valor = 57;
    
    tablaR[52][11].tipo = R;
    tablaR[52][11].valor = 57;
    
    tablaR[52][12].tipo = R;
    tablaR[52][12].valor = 57;
    
    tablaR[52][13].tipo = R;
    tablaR[52][13].valor = 57;
    
    tablaR[52][14].tipo = R;
    tablaR[52][14].valor = 57;
    
    tablaR[52][15].tipo = R;
    tablaR[52][15].valor = 57;
    
    tablaR[52][16].tipo = R;
    tablaR[52][16].valor = 57;
    
    tablaR[52][17].tipo = R;
    tablaR[52][17].valor = 57;
    
    tablaR[52][18].tipo = R;
    tablaR[52][18].valor = 57;
    
    tablaR[52][19].tipo = R;
    tablaR[52][19].valor = 57;
    
    tablaR[53][5].tipo = D;
    tablaR[53][5].valor = 85;
    
    tablaR[54][19].tipo = D;
    tablaR[54][19].valor = 84;
    
    tablaR[55][1].tipo = R;
    tablaR[55][1].valor = 29;
    
    tablaR[55][2].tipo = R;
    tablaR[55][2].valor = 29;
    
    tablaR[55][3].tipo = R;
    tablaR[55][3].valor = 29;
    
    tablaR[55][4].tipo = R;
    tablaR[55][4].valor = 29;
    
    tablaR[55][6].tipo = R;
    tablaR[55][6].valor = 29;
    
    tablaR[55][7].tipo = R;
    tablaR[55][7].valor = 29;
    
    tablaR[55][19].tipo = R;
    tablaR[55][19].valor = 29;
    
    tablaR[55][20].tipo = R;
    tablaR[55][20].valor = 29;
    
    tablaR[55][21].tipo = R;
    tablaR[55][21].valor = 29;
    
    tablaR[55][22].tipo = R;
    tablaR[55][22].valor = 29;
    
    tablaR[55][23].tipo = R;
    tablaR[55][23].valor = 29;
    
    tablaR[55][24].tipo = R;
    tablaR[55][24].valor = 29;
    
    tablaR[56][1].tipo = D;
    tablaR[56][1].valor = 52;
    
    tablaR[56][2].tipo = D;
    tablaR[56][2].valor = 51;
    
    tablaR[56][3].tipo = D;
    tablaR[56][3].valor = 50;
    
    tablaR[56][4].tipo = D;
    tablaR[56][4].valor = 49;
    
    tablaR[56][6].tipo = D;
    tablaR[56][6].valor = 48;
    
    tablaR[56][7].tipo = D;
    tablaR[56][7].valor = 47;
    
    tablaR[56][51].tipo = D;
    tablaR[56][51].valor = 36;
    
    tablaR[56][52].tipo = D;
    tablaR[56][52].valor = 35;
    
    tablaR[56][57].tipo = D;
    tablaR[56][57].valor = 34;
    
    tablaR[56][58].tipo = D;
    tablaR[56][58].valor = 83;
    
    tablaR[56][59].tipo = D;
    tablaR[56][59].valor = 32;
    
    tablaR[57][1].tipo = D;
    tablaR[57][1].valor = 52;
    
    tablaR[57][2].tipo = D;
    tablaR[57][2].valor = 51;
    
    tablaR[57][3].tipo = D;
    tablaR[57][3].valor = 50;
    
    tablaR[57][4].tipo = D;
    tablaR[57][4].valor = 49;
    
    tablaR[57][6].tipo = D;
    tablaR[57][6].valor = 48;
    
    tablaR[57][7].tipo = D;
    tablaR[57][7].valor = 47;
    
    tablaR[57][51].tipo = D;
    tablaR[57][51].valor = 36;
    
    tablaR[57][52].tipo = D;
    tablaR[57][52].valor = 35;
    
    tablaR[57][57].tipo = D;
    tablaR[57][57].valor = 34;
    
    tablaR[57][58].tipo = D;
    tablaR[57][58].valor = 82;
    
    tablaR[57][59].tipo = D;
    tablaR[57][59].valor = 32;
    
    tablaR[58][24].tipo = R;
    tablaR[58][24].valor = 18;
    
    tablaR[59][1].tipo = R;
    tablaR[59][1].valor = 37;
    
    tablaR[59][2].tipo = R;
    tablaR[59][2].valor = 37;
    
    tablaR[59][3].tipo = R;
    tablaR[59][3].valor = 37;
    
    tablaR[59][4].tipo = R;
    tablaR[59][4].valor = 37;
    
    tablaR[59][6].tipo = R;
    tablaR[59][6].valor = 37;
    
    tablaR[59][7].tipo = R;
    tablaR[59][7].valor = 37;
    
    tablaR[60][1].tipo = R;
    tablaR[60][1].valor = 36;
    
    tablaR[60][2].tipo = R;
    tablaR[60][2].valor = 36;
    
    tablaR[60][3].tipo = R;
    tablaR[60][3].valor = 36;
    
    tablaR[60][4].tipo = R;
    tablaR[60][4].valor = 36;
    
    tablaR[60][6].tipo = R;
    tablaR[60][6].valor = 36;
    
    tablaR[60][7].tipo = R;
    tablaR[60][7].valor = 36;
    
    tablaR[61][1].tipo = R;
    tablaR[61][1].valor = 35;
    
    tablaR[61][2].tipo = R;
    tablaR[61][2].valor = 35;
    
    tablaR[61][3].tipo = R;
    tablaR[61][3].valor = 35;
    
    tablaR[61][4].tipo = R;
    tablaR[61][4].valor = 35;
    
    tablaR[61][6].tipo = R;
    tablaR[61][6].valor = 35;
    
    tablaR[61][7].tipo = R;
    tablaR[61][7].valor = 35;
    
    tablaR[62][1].tipo = D;
    tablaR[62][1].valor = 52;
    
    tablaR[62][2].tipo = D;
    tablaR[62][2].valor = 51;
    
    tablaR[62][3].tipo = D;
    tablaR[62][3].valor = 50;
    
    tablaR[62][4].tipo = D;
    tablaR[62][4].valor = 49;
    
    tablaR[62][6].tipo = D;
    tablaR[62][6].valor = 48;
    
    tablaR[62][7].tipo = D;
    tablaR[62][7].valor = 47;
    
    tablaR[62][57].tipo = D;
    tablaR[62][57].valor = 81;
    
    tablaR[62][59].tipo = D;
    tablaR[62][59].valor = 32;
    
    tablaR[63][18].tipo = D;
    tablaR[63][18].valor = 80;
    
    tablaR[64][17].tipo = D;
    tablaR[64][17].valor = 79;
    
    tablaR[65][13].tipo = D;
    tablaR[65][13].valor = 78;
    
    tablaR[66][13].tipo = D;
    tablaR[66][13].valor = 77;
    
    tablaR[67][1].tipo = R;
    tablaR[67][1].valor = 42;
    
    tablaR[67][2].tipo = R;
    tablaR[67][2].valor = 42;
    
    tablaR[67][3].tipo = R;
    tablaR[67][3].valor = 42;
    
    tablaR[67][4].tipo = R;
    tablaR[67][4].valor = 42;
    
    tablaR[67][6].tipo = R;
    tablaR[67][6].valor = 42;
    
    tablaR[67][7].tipo = R;
    tablaR[67][7].valor = 42;
    
    tablaR[67][13].tipo = D;
    tablaR[67][13].valor = 76;
    
    tablaR[68][13].tipo = D;
    tablaR[68][13].valor = 75;
    
    tablaR[69][1].tipo = R;
    tablaR[69][1].valor = 46;
    
    tablaR[69][2].tipo = R;
    tablaR[69][2].valor = 46;
    
    tablaR[69][3].tipo = R;
    tablaR[69][3].valor = 46;
    
    tablaR[69][4].tipo = R;
    tablaR[69][4].valor = 46;
    
    tablaR[69][6].tipo = R;
    tablaR[69][6].valor = 46;
    
    tablaR[69][7].tipo = R;
    tablaR[69][7].valor = 46;
    
    tablaR[70][1].tipo = R;
    tablaR[70][1].valor = 47;
    
    tablaR[70][2].tipo = R;
    tablaR[70][2].valor = 47;
    
    tablaR[70][3].tipo = R;
    tablaR[70][3].valor = 47;
    
    tablaR[70][4].tipo = R;
    tablaR[70][4].valor = 47;
    
    tablaR[70][6].tipo = R;
    tablaR[70][6].valor = 47;
    
    tablaR[70][7].tipo = R;
    tablaR[70][7].valor = 47;
    
    tablaR[71][1].tipo = R;
    tablaR[71][1].valor = 48;
    
    tablaR[71][2].tipo = R;
    tablaR[71][2].valor = 48;
    
    tablaR[71][3].tipo = R;
    tablaR[71][3].valor = 48;
    
    tablaR[71][4].tipo = R;
    tablaR[71][4].valor = 48;
    
    tablaR[71][6].tipo = R;
    tablaR[71][6].valor = 48;
    
    tablaR[71][7].tipo = R;
    tablaR[71][7].valor = 48;
    
    tablaR[72][1].tipo = R;
    tablaR[72][1].valor = 49;
    
    tablaR[72][2].tipo = R;
    tablaR[72][2].valor = 49;
    
    tablaR[72][3].tipo = R;
    tablaR[72][3].valor = 49;
    
    tablaR[72][4].tipo = R;
    tablaR[72][4].valor = 49;
    
    tablaR[72][6].tipo = R;
    tablaR[72][6].valor = 49;
    
    tablaR[72][7].tipo = R;
    tablaR[72][7].valor = 49;
    
    tablaR[73][1].tipo = R;
    tablaR[73][1].valor = 50;
    
    tablaR[73][2].tipo = R;
    tablaR[73][2].valor = 50;
    
    tablaR[73][3].tipo = R;
    tablaR[73][3].valor = 50;
    
    tablaR[73][4].tipo = R;
    tablaR[73][4].valor = 50;
    
    tablaR[73][6].tipo = R;
    tablaR[73][6].valor = 50;
    
    tablaR[73][7].tipo = R;
    tablaR[73][7].valor = 50;
    
    tablaR[74][1].tipo = R;
    tablaR[74][1].valor = 30;
    
    tablaR[74][2].tipo = R;
    tablaR[74][2].valor = 30;
    
    tablaR[74][3].tipo = R;
    tablaR[74][3].valor = 30;
    
    tablaR[74][4].tipo = R;
    tablaR[74][4].valor = 30;
    
    tablaR[74][6].tipo = R;
    tablaR[74][6].valor = 30;
    
    tablaR[74][7].tipo = R;
    tablaR[74][7].valor = 30;
    
    tablaR[74][19].tipo = R;
    tablaR[74][19].valor = 30;
    
    tablaR[74][20].tipo = R;
    tablaR[74][20].valor = 30;
    
    tablaR[74][21].tipo = R;
    tablaR[74][21].valor = 30;
    
    tablaR[74][22].tipo = R;
    tablaR[74][22].valor = 30;
    
    tablaR[74][23].tipo = R;
    tablaR[74][23].valor = 30;
    
    tablaR[74][24].tipo = R;
    tablaR[74][24].valor = 30;
    
    tablaR[75][1].tipo = R;
    tablaR[75][1].valor = 40;
    
    tablaR[75][2].tipo = R;
    tablaR[75][2].valor = 40;
    
    tablaR[75][3].tipo = R;
    tablaR[75][3].valor = 40;
    
    tablaR[75][4].tipo = R;
    tablaR[75][4].valor = 40;
    
    tablaR[75][6].tipo = R;
    tablaR[75][6].valor = 40;
    
    tablaR[75][7].tipo = R;
    tablaR[75][7].valor = 40;
    
    tablaR[76][1].tipo = R;
    tablaR[76][1].valor = 45;
    
    tablaR[76][2].tipo = R;
    tablaR[76][2].valor = 45;
    
    tablaR[76][3].tipo = R;
    tablaR[76][3].valor = 45;
    
    tablaR[76][4].tipo = R;
    tablaR[76][4].valor = 45;
    
    tablaR[76][6].tipo = R;
    tablaR[76][6].valor = 45;
    
    tablaR[76][7].tipo = R;
    tablaR[76][7].valor = 45;
    
    tablaR[77][1].tipo = R;
    tablaR[77][1].valor = 43;
    
    tablaR[77][2].tipo = R;
    tablaR[77][2].valor = 43;
    
    tablaR[77][3].tipo = R;
    tablaR[77][3].valor = 43;
    
    tablaR[77][4].tipo = R;
    tablaR[77][4].valor = 43;
    
    tablaR[77][6].tipo = R;
    tablaR[77][6].valor = 43;
    
    tablaR[77][7].tipo = R;
    tablaR[77][7].valor = 43;
    
    tablaR[78][1].tipo = R;
    tablaR[78][1].valor = 41;
    
    tablaR[78][2].tipo = R;
    tablaR[78][2].valor = 41;
    
    tablaR[78][3].tipo = R;
    tablaR[78][3].valor = 41;
    
    tablaR[78][4].tipo = R;
    tablaR[78][4].valor = 41;
    
    tablaR[78][6].tipo = R;
    tablaR[78][6].valor = 41;
    
    tablaR[78][7].tipo = R;
    tablaR[78][7].valor = 41;
    
    tablaR[79][1].tipo = R;
    tablaR[79][1].valor = 39;
    
    tablaR[79][2].tipo = R;
    tablaR[79][2].valor = 39;
    
    tablaR[79][3].tipo = R;
    tablaR[79][3].valor = 39;
    
    tablaR[79][4].tipo = R;
    tablaR[79][4].valor = 39;
    
    tablaR[79][6].tipo = R;
    tablaR[79][6].valor = 39;
    
    tablaR[79][7].tipo = R;
    tablaR[79][7].valor = 39;
    
    tablaR[80][1].tipo = R;
    tablaR[80][1].valor = 38;
    
    tablaR[80][2].tipo = R;
    tablaR[80][2].valor = 38;
    
    tablaR[80][3].tipo = R;
    tablaR[80][3].valor = 38;
    
    tablaR[80][4].tipo = R;
    tablaR[80][4].valor = 38;
    
    tablaR[80][6].tipo = R;
    tablaR[80][6].valor = 38;
    
    tablaR[80][7].tipo = R;
    tablaR[80][7].valor = 38;
    
    tablaR[81][5].tipo = R;
    tablaR[81][5].valor = 33;
    
    tablaR[81][8].tipo = R;
    tablaR[81][8].valor = 33;
    
    tablaR[81][9].tipo = R;
    tablaR[81][9].valor = 33;
    
    tablaR[81][10].tipo = R;
    tablaR[81][10].valor = 33;
    
    tablaR[81][11].tipo = R;
    tablaR[81][11].valor = 33;
    
    tablaR[81][12].tipo = R;
    tablaR[81][12].valor = 33;
    
    tablaR[81][13].tipo = R;
    tablaR[81][13].valor = 33;
    
    tablaR[81][14].tipo = R;
    tablaR[81][14].valor = 33;
    
    tablaR[81][15].tipo = R;
    tablaR[81][15].valor = 33;
    
    tablaR[81][16].tipo = R;
    tablaR[81][16].valor = 33;
    
    tablaR[81][17].tipo = R;
    tablaR[81][17].valor = 33;
    
    tablaR[81][18].tipo = R;
    tablaR[81][18].valor = 33;
    
    tablaR[81][19].tipo = R;
    tablaR[81][19].valor = 33;
    
    tablaR[82][5].tipo = D;
    tablaR[82][5].valor = 87;
    
    tablaR[83][5].tipo = D;
    tablaR[83][5].valor = 86;
    
    tablaR[84][1].tipo = R;
    tablaR[84][1].valor = 28;
    
    tablaR[84][2].tipo = R;
    tablaR[84][2].valor = 28;
    
    tablaR[84][3].tipo = R;
    tablaR[84][3].valor = 28;
    
    tablaR[84][4].tipo = R;
    tablaR[84][4].valor = 28;
    
    tablaR[84][6].tipo = R;
    tablaR[84][6].valor = 28;
    
    tablaR[84][7].tipo = R;
    tablaR[84][7].valor = 28;
    
    tablaR[84][19].tipo = R;
    tablaR[84][19].valor = 28;
    
    tablaR[84][20].tipo = R;
    tablaR[84][20].valor = 28;
    
    tablaR[84][21].tipo = R;
    tablaR[84][21].valor = 28;
    
    tablaR[84][22].tipo = R;
    tablaR[84][22].valor = 28;
    
    tablaR[84][23].tipo = R;
    tablaR[84][23].valor = 28;
    
    tablaR[84][24].tipo = R;
    tablaR[84][24].valor = 28;
    
    tablaR[85][5].tipo = R;
    tablaR[85][5].valor = 53;
    
    tablaR[85][8].tipo = R;
    tablaR[85][8].valor = 53;
    
    tablaR[85][9].tipo = R;
    tablaR[85][9].valor = 53;
    
    tablaR[85][10].tipo = R;
    tablaR[85][10].valor = 53;
    
    tablaR[85][11].tipo = R;
    tablaR[85][11].valor = 53;
    
    tablaR[85][12].tipo = R;
    tablaR[85][12].valor = 53;
    
    tablaR[85][13].tipo = R;
    tablaR[85][13].valor = 53;
    
    tablaR[85][14].tipo = R;
    tablaR[85][14].valor = 53;
    
    tablaR[85][15].tipo = R;
    tablaR[85][15].valor = 53;
    
    tablaR[85][16].tipo = R;
    tablaR[85][16].valor = 53;
    
    tablaR[85][17].tipo = R;
    tablaR[85][17].valor = 53;
    
    tablaR[85][18].tipo = R;
    tablaR[85][18].valor = 53;
    
    tablaR[85][19].tipo = R;
    tablaR[85][19].valor = 53;
    
    tablaR[86][1].tipo = D;
    tablaR[86][1].valor = 52;
    
    tablaR[86][2].tipo = D;
    tablaR[86][2].valor = 51;
    
    tablaR[86][3].tipo = D;
    tablaR[86][3].valor = 50;
    
    tablaR[86][4].tipo = D;
    tablaR[86][4].valor = 49;
    
    tablaR[86][6].tipo = D;
    tablaR[86][6].valor = 48;
    
    tablaR[86][7].tipo = D;
    tablaR[86][7].valor = 47;
    
    tablaR[86][19].tipo = D;
    tablaR[86][19].valor = 46;
    
    tablaR[86][20].tipo = D;
    tablaR[86][20].valor = 45;
    
    tablaR[86][22].tipo = D;
    tablaR[86][22].valor = 44;
    
    tablaR[86][23].tipo = D;
    tablaR[86][23].valor = 43;
    
    tablaR[86][46].tipo = D;
    tablaR[86][46].valor = 41;
    
    tablaR[86][47].tipo = D;
    tablaR[86][47].valor = 40;
    
    tablaR[86][48].tipo = D;
    tablaR[86][48].valor = 89;
    
    tablaR[86][49].tipo = D;
    tablaR[86][49].valor = 38;
    
    tablaR[86][50].tipo = D;
    tablaR[86][50].valor = 37;
    
    tablaR[86][51].tipo = D;
    tablaR[86][51].valor = 36;
    
    tablaR[86][52].tipo = D;
    tablaR[86][52].valor = 35;
    
    tablaR[86][57].tipo = D;
    tablaR[86][57].valor = 34;
    
    tablaR[86][58].tipo = D;
    tablaR[86][58].valor = 33;
    
    tablaR[86][59].tipo = D;
    tablaR[86][59].valor = 32;
    
    tablaR[87][1].tipo = D;
    tablaR[87][1].valor = 52;
    
    tablaR[87][2].tipo = D;
    tablaR[87][2].valor = 51;
    
    tablaR[87][3].tipo = D;
    tablaR[87][3].valor = 50;
    
    tablaR[87][4].tipo = D;
    tablaR[87][4].valor = 49;
    
    tablaR[87][6].tipo = D;
    tablaR[87][6].valor = 48;
    
    tablaR[87][7].tipo = D;
    tablaR[87][7].valor = 47;
    
    tablaR[87][19].tipo = D;
    tablaR[87][19].valor = 46;
    
    tablaR[87][20].tipo = D;
    tablaR[87][20].valor = 45;
    
    tablaR[87][22].tipo = D;
    tablaR[87][22].valor = 44;
    
    tablaR[87][23].tipo = D;
    tablaR[87][23].valor = 43;
    
    tablaR[87][46].tipo = D;
    tablaR[87][46].valor = 41;
    
    tablaR[87][47].tipo = D;
    tablaR[87][47].valor = 40;
    
    tablaR[87][48].tipo = D;
    tablaR[87][48].valor = 88;
    
    tablaR[87][49].tipo = D;
    tablaR[87][49].valor = 38;
    
    tablaR[87][50].tipo = D;
    tablaR[87][50].valor = 37;
    
    tablaR[87][51].tipo = D;
    tablaR[87][51].valor = 36;
    
    tablaR[87][52].tipo = D;
    tablaR[87][52].valor = 35;
    
    tablaR[87][57].tipo = D;
    tablaR[87][57].valor = 34;
    
    tablaR[87][58].tipo = D;
    tablaR[87][58].valor = 33;
    
    tablaR[87][59].tipo = D;
    tablaR[87][59].valor = 32;
    
    tablaR[88][1].tipo = R;
    tablaR[88][1].valor = 25;
    
    tablaR[88][2].tipo = R;
    tablaR[88][2].valor = 25;
    
    tablaR[88][3].tipo = R;
    tablaR[88][3].valor = 25;
    
    tablaR[88][4].tipo = R;
    tablaR[88][4].valor = 25;
    
    tablaR[88][6].tipo = R;
    tablaR[88][6].valor = 25;
    
    tablaR[88][7].tipo = R;
    tablaR[88][7].valor = 25;
    
    tablaR[88][19].tipo = R;
    tablaR[88][19].valor = 25;
    
    tablaR[88][20].tipo = R;
    tablaR[88][20].valor = 25;
    
    tablaR[88][21].tipo = R;
    tablaR[88][21].valor = 25;
    
    tablaR[88][22].tipo = R;
    tablaR[88][22].valor = 25;
    
    tablaR[88][23].tipo = R;
    tablaR[88][23].valor = 25;
    
    tablaR[88][24].tipo = R;
    tablaR[88][24].valor = 25;
    
    tablaR[89][1].tipo = R;
    tablaR[89][1].valor = 26;
    
    tablaR[89][2].tipo = R;
    tablaR[89][2].valor = 26;
    
    tablaR[89][3].tipo = R;
    tablaR[89][3].valor = 26;
    
    tablaR[89][4].tipo = R;
    tablaR[89][4].valor = 26;
    
    tablaR[89][6].tipo = R;
    tablaR[89][6].valor = 26;
    
    tablaR[89][7].tipo = R;
    tablaR[89][7].valor = 26;
    
    tablaR[89][19].tipo = R;
    tablaR[89][19].valor = 26;
    
    tablaR[89][20].tipo = R;
    tablaR[89][20].valor = 26;
    
    tablaR[89][21].tipo = R;
    tablaR[89][21].valor = 26;
    
    tablaR[89][22].tipo = R;
    tablaR[89][22].valor = 26;
    
    tablaR[89][23].tipo = R;
    tablaR[89][23].valor = 26;
    
    tablaR[89][24].tipo = R;
    tablaR[89][24].valor = 26;
    
    tablaR[90][1].tipo = D;
    tablaR[90][1].valor = 52;
    
    tablaR[90][2].tipo = D;
    tablaR[90][2].valor = 51;
    
    tablaR[90][3].tipo = D;
    tablaR[90][3].valor = 50;
    
    tablaR[90][4].tipo = D;
    tablaR[90][4].valor = 49;
    
    tablaR[90][6].tipo = D;
    tablaR[90][6].valor = 48;
    
    tablaR[90][7].tipo = D;
    tablaR[90][7].valor = 47;
    
    tablaR[90][19].tipo = D;
    tablaR[90][19].valor = 46;
    
    tablaR[90][20].tipo = D;
    tablaR[90][20].valor = 45;
    
    tablaR[90][22].tipo = D;
    tablaR[90][22].valor = 44;
    
    tablaR[90][23].tipo = D;
    tablaR[90][23].valor = 43;
    
    tablaR[90][46].tipo = D;
    tablaR[90][46].valor = 41;
    
    tablaR[90][47].tipo = D;
    tablaR[90][47].valor = 40;
    
    tablaR[90][48].tipo = D;
    tablaR[90][48].valor = 91;
    
    tablaR[90][49].tipo = D;
    tablaR[90][49].valor = 38;
    
    tablaR[90][50].tipo = D;
    tablaR[90][50].valor = 37;
    
    tablaR[90][51].tipo = D;
    tablaR[90][51].valor = 36;
    
    tablaR[90][52].tipo = D;
    tablaR[90][52].valor = 35;
    
    tablaR[90][57].tipo = D;
    tablaR[90][57].valor = 34;
    
    tablaR[90][58].tipo = D;
    tablaR[90][58].valor = 33;
    
    tablaR[90][59].tipo = D;
    tablaR[90][59].valor = 32;
    
    tablaR[91][1].tipo = R;
    tablaR[91][1].valor = 27;
    
    tablaR[91][2].tipo = R;
    tablaR[91][2].valor = 27;
    
    tablaR[91][3].tipo = R;
    tablaR[91][3].valor = 27;
    
    tablaR[91][4].tipo = R;
    tablaR[91][4].valor = 27;
    
    tablaR[91][6].tipo = R;
    tablaR[91][6].valor = 27;
    
    tablaR[91][7].tipo = R;
    tablaR[91][7].valor = 27;
    
    tablaR[91][19].tipo = R;
    tablaR[91][19].valor = 27;
    
    tablaR[91][20].tipo = R;
    tablaR[91][20].valor = 27;
    
    tablaR[91][21].tipo = R;
    tablaR[91][21].valor = 27;
    
    tablaR[91][22].tipo = R;
    tablaR[91][22].valor = 27;
    
    tablaR[91][23].tipo = R;
    tablaR[91][23].valor = 27;
    
    tablaR[91][24].tipo = R;
    tablaR[91][24].valor = 27;
}
