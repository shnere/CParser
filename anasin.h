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
			
			// Incrementa Valor
			i++;
		} else if (actual.tipo == R) {
			
			// Pop hasta encontrar en pila el primer valor de derivacion
			
			cero	= gramatica[actual.valor].cadenaDerivacion[0];
			uno		= gramatica[actual.valor].cadenaDerivacion[1];
			if(gramatica[actual.valor].derivaciones > 2)
                dos		= gramatica[actual.valor].cadenaDerivacion[2];
			//fprintf(stdout, "cero:%s Uno:%s\n",cero,uno);
			
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
	noTerminales = 16;
	terminales	 = 19;
	estados		 = 70;
	derivacionesGramatica = 30;
    siguientes = 0;
	
	// Crea arreglo de no terminales
    arregloNoTerminales[0] = "PROGRAM";
    arregloNoTerminales[1] = "VAR_LIST";
    arregloNoTerminales[2] = "VAR_ITEM";
    arregloNoTerminales[3] = "MAIN_DEF";
    arregloNoTerminales[4] = "FUNCTION_BODY";
    arregloNoTerminales[5] = "DECLARATION";
    arregloNoTerminales[6] = "INTERNAL_DECLARATIONS";
    arregloNoTerminales[7] = "STATEMENT_LIST";
    arregloNoTerminales[8] = "WHILE_STATEMENT";
    arregloNoTerminales[9] = "IF_STATEMENT";
    arregloNoTerminales[10] = "STATEMENT";
    arregloNoTerminales[11] = "RETURN_STATEMENT";
    arregloNoTerminales[12] = "EXPRESSION";
    arregloNoTerminales[13] = "ASSIGN_EXP";
    arregloNoTerminales[14] = "BINARY_OP";
    arregloNoTerminales[15] = "PRIMARY_EXPR";
	
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
    
    // EXPRESSION -> PRIMARY_EXPR BINARY_OP PRIMARY_EXPR
    gramatica[21].cadenaDerivacion[0] = "EXPRESSION";
    gramatica[21].cadenaDerivacion[1] = "PRIMARY_EXPR";
    gramatica[21].cadenaDerivacion[2] = "BINARY_OP";
    gramatica[21].cadenaDerivacion[3] = "PRIMARY_EXPR";
    gramatica[21].derivaciones = 4;
    
    // EXPRESSION -> ASSIGN_EXP
    gramatica[22].cadenaDerivacion[0] = "EXPRESSION";
    gramatica[22].cadenaDerivacion[1] = "ASSIGN_EXP";
    gramatica[22].derivaciones = 2;
    
    // ASSIGN_EXP-> var_name equal PRIMARY_EXPR BINARY_OP PRIMARY_EXPR
    gramatica[23].cadenaDerivacion[0] = "ASSIGN_EXP";
    gramatica[23].cadenaDerivacion[1] = "var_name";
    gramatica[23].cadenaDerivacion[2] = "equal";
    gramatica[23].cadenaDerivacion[3] = "PRIMARY_EXPR";
    gramatica[23].cadenaDerivacion[4] = "BINARY_OP";
    gramatica[23].cadenaDerivacion[5] = "PRIMARY_EXPR";
    gramatica[23].derivaciones = 6;
    
    // BINARY_OP -> boolean_op
    gramatica[24].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[24].cadenaDerivacion[1] = "boolean_op";
    gramatica[24].derivaciones = 2;
    
    // BINARY_OP -> rel_op
    gramatica[25].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[25].cadenaDerivacion[1] = "rel_op";
    gramatica[25].derivaciones = 2;
    
    // BINARY_OP -> arith_op
    gramatica[26].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[26].cadenaDerivacion[1] = "arith_op";
    gramatica[26].derivaciones = 2;
    
    // PRIMARY_EXPR -> var_name
    gramatica[27].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[27].cadenaDerivacion[1] = "var_name";
    gramatica[27].derivaciones = 2;
    
    // PRIMARY_EXPR -> constant
    gramatica[28].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[28].cadenaDerivacion[1] = "constant";
    gramatica[28].derivaciones = 2;
    
    // PRIMARY_EXPR -> left_parenthesis PRIMARY_EXPR right_parenthesis
    gramatica[29].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[29].cadenaDerivacion[1] = "left_parenthesis";
    gramatica[29].cadenaDerivacion[2] = "PRIMARY_EXPR";
    gramatica[29].cadenaDerivacion[3] = "right_parenthesis";
    gramatica[29].derivaciones = 4;
    
	// Inicializa la tabla con valores default
	inicializaTabla(tablaR);
    
	// Tabla
    tablaR[0][17].tipo = D;
    tablaR[0][17].valor = 3;
    
    tablaR[0][19].tipo = D;
    tablaR[0][19].valor = 2;
    
    tablaR[0][22].tipo = D;
    tablaR[0][22].valor = 1;
    
    tablaR[1][0].tipo = R;
    tablaR[1][0].valor = 0;
    
    tablaR[2][0].tipo = ACEPTA;
    tablaR[2][0].valor = ERR;
    
    tablaR[3][16].tipo = D;
    tablaR[3][16].valor = 4;
    
    tablaR[4][2].tipo = D;
    tablaR[4][2].valor = 5;
    
    tablaR[5][1].tipo = D;
    tablaR[5][1].valor = 6;
    
    tablaR[6][12].tipo = D;
    tablaR[6][12].valor = 7;
    
    tablaR[7][2].tipo = R;
    tablaR[7][2].valor = 8;
    
    tablaR[7][3].tipo = R;
    tablaR[7][3].valor = 8;
    
    tablaR[7][4].tipo = R;
    tablaR[7][4].valor = 8;
    
    tablaR[7][9].tipo = R;
    tablaR[7][9].valor = 8;
    
    tablaR[7][10].tipo = R;
    tablaR[7][10].valor = 8;
    
    tablaR[7][11].tipo = R;
    tablaR[7][11].valor = 8;
    
    tablaR[7][14].tipo = R;
    tablaR[7][14].valor = 8;
    
    tablaR[7][15].tipo = R;
    tablaR[7][15].valor = 8;
    
    tablaR[7][17].tipo = D;
    tablaR[7][17].valor = 11;
    
    tablaR[7][23].tipo = D;
    tablaR[7][23].valor = 10;
    
    tablaR[7][24].tipo = D;
    tablaR[7][24].valor = 9;
    
    tablaR[7][25].tipo = D;
    tablaR[7][25].valor = 8;
    
    tablaR[8][2].tipo = D;
    tablaR[8][2].valor = 31;
    
    tablaR[8][3].tipo = D;
    tablaR[8][3].valor = 30;
    
    tablaR[8][4].tipo = D;
    tablaR[8][4].valor = 29;
    
    tablaR[8][9].tipo = D;
    tablaR[8][9].valor = 28;
    
    tablaR[8][10].tipo = D;
    tablaR[8][10].valor = 27;
    
    tablaR[8][11].tipo = R;
    tablaR[8][11].valor = 10;
    
    tablaR[8][14].tipo = D;
    tablaR[8][14].valor = 26;
    
    tablaR[8][15].tipo = D;
    tablaR[8][15].valor = 25;
    
    tablaR[8][26].tipo = D;
    tablaR[8][26].valor = 24;
    
    tablaR[8][27].tipo = D;
    tablaR[8][27].valor = 23;
    
    tablaR[8][28].tipo = D;
    tablaR[8][28].valor = 22;
    
    tablaR[8][29].tipo = D;
    tablaR[8][29].valor = 21;
    
    tablaR[8][30].tipo = D;
    tablaR[8][30].valor = 20;
    
    tablaR[8][31].tipo = D;
    tablaR[8][31].valor = 19;
    
    tablaR[8][32].tipo = D;
    tablaR[8][32].valor = 18;
    
    tablaR[8][34].tipo = D;
    tablaR[8][34].valor = 17;
    
    tablaR[9][2].tipo = R;
    tablaR[9][2].valor = 8;
    
    tablaR[9][3].tipo = R;
    tablaR[9][3].valor = 8;
    
    tablaR[9][4].tipo = R;
    tablaR[9][4].valor = 8;
    
    tablaR[9][9].tipo = R;
    tablaR[9][9].valor = 8;
    
    tablaR[9][10].tipo = R;
    tablaR[9][10].valor = 8;
    
    tablaR[9][11].tipo = R;
    tablaR[9][11].valor = 8;
    
    tablaR[9][14].tipo = R;
    tablaR[9][14].valor = 8;
    
    tablaR[9][15].tipo = R;
    tablaR[9][15].valor = 8;
    
    tablaR[9][17].tipo = D;
    tablaR[9][17].valor = 11;
    
    tablaR[9][24].tipo = D;
    tablaR[9][24].valor = 9;
    
    tablaR[9][25].tipo = D;
    tablaR[9][25].valor = 16;
    
    tablaR[10][11].tipo = D;
    tablaR[10][11].valor = 15;
    
    tablaR[11][4].tipo = D;
    tablaR[11][4].valor = 14;
    
    tablaR[11][20].tipo = D;
    tablaR[11][20].valor = 13;
    
    tablaR[11][21].tipo = D;
    tablaR[11][21].valor = 12;
    
    tablaR[12][9].tipo = R;
    tablaR[12][9].valor = 3;
    
    tablaR[12][18].tipo = R;
    tablaR[12][18].valor = 3;
    
    tablaR[13][9].tipo = D;
    tablaR[13][9].valor = 46;
    
    tablaR[13][18].tipo = D;
    tablaR[13][18].valor = 45;
    
    tablaR[14][9].tipo = R;
    tablaR[14][9].valor = 4;
    
    tablaR[14][18].tipo = R;
    tablaR[14][18].valor = 4;
    
    tablaR[15][0].tipo = R;
    tablaR[15][0].valor = 5;
    
    tablaR[16][2].tipo = R;
    tablaR[16][2].valor = 7;
    
    tablaR[16][3].tipo = R;
    tablaR[16][3].valor = 7;
    
    tablaR[16][4].tipo = R;
    tablaR[16][4].valor = 7;
    
    tablaR[16][9].tipo = R;
    tablaR[16][9].valor = 7;
    
    tablaR[16][10].tipo = R;
    tablaR[16][10].valor = 7;
    
    tablaR[16][11].tipo = R;
    tablaR[16][11].valor = 7;
    
    tablaR[16][14].tipo = R;
    tablaR[16][14].valor = 7;
    
    tablaR[16][15].tipo = R;
    tablaR[16][15].valor = 7;
    
    tablaR[17][5].tipo = D;
    tablaR[17][5].valor = 44;
    
    tablaR[17][6].tipo = D;
    tablaR[17][6].valor = 43;
    
    tablaR[17][7].tipo = D;
    tablaR[17][7].valor = 42;
    
    tablaR[17][33].tipo = D;
    tablaR[17][33].valor = 41;
    
    tablaR[18][1].tipo = R;
    tablaR[18][1].valor = 22;
    
    tablaR[18][9].tipo = R;
    tablaR[18][9].valor = 22;
    
    tablaR[19][9].tipo = D;
    tablaR[19][9].valor = 40;
    
    tablaR[20][2].tipo = R;
    tablaR[20][2].valor = 14;
    
    tablaR[20][3].tipo = R;
    tablaR[20][3].valor = 14;
    
    tablaR[20][4].tipo = R;
    tablaR[20][4].valor = 14;
    
    tablaR[20][9].tipo = R;
    tablaR[20][9].valor = 14;
    
    tablaR[20][10].tipo = R;
    tablaR[20][10].valor = 14;
    
    tablaR[20][11].tipo = R;
    tablaR[20][11].valor = 14;
    
    tablaR[20][14].tipo = R;
    tablaR[20][14].valor = 14;
    
    tablaR[20][15].tipo = R;
    tablaR[20][15].valor = 14;
    
    tablaR[21][2].tipo = D;
    tablaR[21][2].valor = 31;
    
    tablaR[21][3].tipo = D;
    tablaR[21][3].valor = 30;
    
    tablaR[21][4].tipo = D;
    tablaR[21][4].valor = 29;
    
    tablaR[21][9].tipo = D;
    tablaR[21][9].valor = 28;
    
    tablaR[21][10].tipo = D;
    tablaR[21][10].valor = 27;
    
    tablaR[21][11].tipo = R;
    tablaR[21][11].valor = 10;
    
    tablaR[21][14].tipo = D;
    tablaR[21][14].valor = 26;
    
    tablaR[21][15].tipo = D;
    tablaR[21][15].valor = 25;
    
    tablaR[21][26].tipo = D;
    tablaR[21][26].valor = 39;
    
    tablaR[21][27].tipo = D;
    tablaR[21][27].valor = 23;
    
    tablaR[21][28].tipo = D;
    tablaR[21][28].valor = 22;
    
    tablaR[21][29].tipo = D;
    tablaR[21][29].valor = 21;
    
    tablaR[21][30].tipo = D;
    tablaR[21][30].valor = 20;
    
    tablaR[21][31].tipo = D;
    tablaR[21][31].valor = 19;
    
    tablaR[21][32].tipo = D;
    tablaR[21][32].valor = 18;
    
    tablaR[21][34].tipo = D;
    tablaR[21][34].valor = 17;
    
    tablaR[22][2].tipo = R;
    tablaR[22][2].valor = 11;
    
    tablaR[22][3].tipo = R;
    tablaR[22][3].valor = 11;
    
    tablaR[22][4].tipo = R;
    tablaR[22][4].valor = 11;
    
    tablaR[22][9].tipo = R;
    tablaR[22][9].valor = 11;
    
    tablaR[22][10].tipo = R;
    tablaR[22][10].valor = 11;
    
    tablaR[22][11].tipo = R;
    tablaR[22][11].valor = 11;
    
    tablaR[22][14].tipo = R;
    tablaR[22][14].valor = 11;
    
    tablaR[22][15].tipo = R;
    tablaR[22][15].valor = 11;
    
    tablaR[23][2].tipo = R;
    tablaR[23][2].valor = 13;
    
    tablaR[23][3].tipo = R;
    tablaR[23][3].valor = 13;
    
    tablaR[23][4].tipo = R;
    tablaR[23][4].valor = 13;
    
    tablaR[23][9].tipo = R;
    tablaR[23][9].valor = 13;
    
    tablaR[23][10].tipo = R;
    tablaR[23][10].valor = 13;
    
    tablaR[23][11].tipo = R;
    tablaR[23][11].valor = 13;
    
    tablaR[23][14].tipo = R;
    tablaR[23][14].valor = 13;
    
    tablaR[23][15].tipo = R;
    tablaR[23][15].valor = 13;
    
    tablaR[24][11].tipo = R;
    tablaR[24][11].valor = 6;
    
    tablaR[25][2].tipo = D;
    tablaR[25][2].valor = 38;
    
    tablaR[26][2].tipo = D;
    tablaR[26][2].valor = 37;
    
    tablaR[27][2].tipo = D;
    tablaR[27][2].valor = 31;
    
    tablaR[27][3].tipo = D;
    tablaR[27][3].valor = 30;
    
    tablaR[27][4].tipo = D;
    tablaR[27][4].valor = 29;
    
    tablaR[27][9].tipo = D;
    tablaR[27][9].valor = 36;
    
    tablaR[27][31].tipo = D;
    tablaR[27][31].valor = 35;
    
    tablaR[27][32].tipo = D;
    tablaR[27][32].valor = 18;
    
    tablaR[27][34].tipo = D;
    tablaR[27][34].valor = 17;
    
    tablaR[28][2].tipo = R;
    tablaR[28][2].valor = 15;
    
    tablaR[28][3].tipo = R;
    tablaR[28][3].valor = 15;
    
    tablaR[28][4].tipo = R;
    tablaR[28][4].valor = 15;
    
    tablaR[28][9].tipo = R;
    tablaR[28][9].valor = 15;
    
    tablaR[28][10].tipo = R;
    tablaR[28][10].valor = 15;
    
    tablaR[28][11].tipo = R;
    tablaR[28][11].valor = 15;
    
    tablaR[28][14].tipo = R;
    tablaR[28][14].valor = 15;
    
    tablaR[28][15].tipo = R;
    tablaR[28][15].valor = 15;
    
    tablaR[29][5].tipo = R;
    tablaR[29][5].valor = 27;
    
    tablaR[29][6].tipo = R;
    tablaR[29][6].valor = 27;
    
    tablaR[29][7].tipo = R;
    tablaR[29][7].valor = 27;
    
    tablaR[29][8].tipo = D;
    tablaR[29][8].valor = 34;
    
    tablaR[30][1].tipo = R;
    tablaR[30][1].valor = 28;
    
    tablaR[30][5].tipo = R;
    tablaR[30][5].valor = 28;
    
    tablaR[30][6].tipo = R;
    tablaR[30][6].valor = 28;
    
    tablaR[30][7].tipo = R;
    tablaR[30][7].valor = 28;
    
    tablaR[31][2].tipo = D;
    tablaR[31][2].valor = 31;
    
    tablaR[31][3].tipo = D;
    tablaR[31][3].valor = 30;
    
    tablaR[31][4].tipo = D;
    tablaR[31][4].valor = 33;
    
    tablaR[31][34].tipo = D;
    tablaR[31][34].valor = 32;
    
    tablaR[32][1].tipo = D;
    tablaR[32][1].valor = 53;
    
    tablaR[33][1].tipo = R;
    tablaR[33][1].valor = 27;
    
    tablaR[33][5].tipo = R;
    tablaR[33][5].valor = 27;
    
    tablaR[33][6].tipo = R;
    tablaR[33][6].valor = 27;
    
    tablaR[33][7].tipo = R;
    tablaR[33][7].valor = 27;
    
    tablaR[34][2].tipo = D;
    tablaR[34][2].valor = 31;
    
    tablaR[34][3].tipo = D;
    tablaR[34][3].valor = 30;
    
    tablaR[34][4].tipo = D;
    tablaR[34][4].valor = 33;
    
    tablaR[34][34].tipo = D;
    tablaR[34][34].valor = 52;
    
    tablaR[35][9].tipo = D;
    tablaR[35][9].valor = 51;
    
    tablaR[36][2].tipo = R;
    tablaR[36][2].valor = 20;
    
    tablaR[36][3].tipo = R;
    tablaR[36][3].valor = 20;
    
    tablaR[36][4].tipo = R;
    tablaR[36][4].valor = 20;
    
    tablaR[36][9].tipo = R;
    tablaR[36][9].valor = 20;
    
    tablaR[36][10].tipo = R;
    tablaR[36][10].valor = 20;
    
    tablaR[36][11].tipo = R;
    tablaR[36][11].valor = 20;
    
    tablaR[36][14].tipo = R;
    tablaR[36][14].valor = 20;
    
    tablaR[36][15].tipo = R;
    tablaR[36][15].valor = 20;
    
    tablaR[37][2].tipo = D;
    tablaR[37][2].valor = 31;
    
    tablaR[37][3].tipo = D;
    tablaR[37][3].valor = 30;
    
    tablaR[37][4].tipo = D;
    tablaR[37][4].valor = 29;
    
    tablaR[37][31].tipo = D;
    tablaR[37][31].valor = 50;
    
    tablaR[37][32].tipo = D;
    tablaR[37][32].valor = 18;
    
    tablaR[37][34].tipo = D;
    tablaR[37][34].valor = 17;
    
    tablaR[38][2].tipo = D;
    tablaR[38][2].valor = 31;
    
    tablaR[38][3].tipo = D;
    tablaR[38][3].valor = 30;
    
    tablaR[38][4].tipo = D;
    tablaR[38][4].valor = 29;
    
    tablaR[38][31].tipo = D;
    tablaR[38][31].valor = 49;
    
    tablaR[38][32].tipo = D;
    tablaR[38][32].valor = 18;
    
    tablaR[38][34].tipo = D;
    tablaR[38][34].valor = 17;
    
    tablaR[39][11].tipo = R;
    tablaR[39][11].valor = 9;
    
    tablaR[40][2].tipo = R;
    tablaR[40][2].valor = 12;
    
    tablaR[40][3].tipo = R;
    tablaR[40][3].valor = 12;
    
    tablaR[40][4].tipo = R;
    tablaR[40][4].valor = 12;
    
    tablaR[40][9].tipo = R;
    tablaR[40][9].valor = 12;
    
    tablaR[40][10].tipo = R;
    tablaR[40][10].valor = 12;
    
    tablaR[40][11].tipo = R;
    tablaR[40][11].valor = 12;
    
    tablaR[40][14].tipo = R;
    tablaR[40][14].valor = 12;
    
    tablaR[40][15].tipo = R;
    tablaR[40][15].valor = 12;
    
    tablaR[41][2].tipo = D;
    tablaR[41][2].valor = 31;
    
    tablaR[41][3].tipo = D;
    tablaR[41][3].valor = 30;
    
    tablaR[41][4].tipo = D;
    tablaR[41][4].valor = 33;
    
    tablaR[41][34].tipo = D;
    tablaR[41][34].valor = 48;
    
    tablaR[42][2].tipo = R;
    tablaR[42][2].valor = 24;
    
    tablaR[42][3].tipo = R;
    tablaR[42][3].valor = 24;
    
    tablaR[42][4].tipo = R;
    tablaR[42][4].valor = 24;
    
    tablaR[43][2].tipo = R;
    tablaR[43][2].valor = 25;
    
    tablaR[43][3].tipo = R;
    tablaR[43][3].valor = 25;
    
    tablaR[43][4].tipo = R;
    tablaR[43][4].valor = 25;
    
    tablaR[44][2].tipo = R;
    tablaR[44][2].valor = 26;
    
    tablaR[44][3].tipo = R;
    tablaR[44][3].valor = 26;
    
    tablaR[44][4].tipo = R;
    tablaR[44][4].valor = 26;
    
    tablaR[45][4].tipo = D;
    tablaR[45][4].valor = 14;
    
    tablaR[45][21].tipo = D;
    tablaR[45][21].valor = 47;
    
    tablaR[46][2].tipo = R;
    tablaR[46][2].valor = 1;
    
    tablaR[46][3].tipo = R;
    tablaR[46][3].valor = 1;
    
    tablaR[46][4].tipo = R;
    tablaR[46][4].valor = 1;
    
    tablaR[46][9].tipo = R;
    tablaR[46][9].valor = 1;
    
    tablaR[46][10].tipo = R;
    tablaR[46][10].valor = 1;
    
    tablaR[46][11].tipo = R;
    tablaR[46][11].valor = 1;
    
    tablaR[46][14].tipo = R;
    tablaR[46][14].valor = 1;
    
    tablaR[46][15].tipo = R;
    tablaR[46][15].valor = 1;
    
    tablaR[46][17].tipo = R;
    tablaR[46][17].valor = 1;
    
    tablaR[47][9].tipo = R;
    tablaR[47][9].valor = 2;
    
    tablaR[47][18].tipo = R;
    tablaR[47][18].valor = 2;
    
    tablaR[48][1].tipo = R;
    tablaR[48][1].valor = 21;
    
    tablaR[48][9].tipo = R;
    tablaR[48][9].valor = 21;
    
    tablaR[49][1].tipo = D;
    tablaR[49][1].valor = 56;
    
    tablaR[50][1].tipo = D;
    tablaR[50][1].valor = 55;
    
    tablaR[51][2].tipo = R;
    tablaR[51][2].valor = 19;
    
    tablaR[51][3].tipo = R;
    tablaR[51][3].valor = 19;
    
    tablaR[51][4].tipo = R;
    tablaR[51][4].valor = 19;
    
    tablaR[51][9].tipo = R;
    tablaR[51][9].valor = 19;
    
    tablaR[51][10].tipo = R;
    tablaR[51][10].valor = 19;
    
    tablaR[51][11].tipo = R;
    tablaR[51][11].valor = 19;
    
    tablaR[51][14].tipo = R;
    tablaR[51][14].valor = 19;
    
    tablaR[51][15].tipo = R;
    tablaR[51][15].valor = 19;
    
    tablaR[52][5].tipo = D;
    tablaR[52][5].valor = 44;
    
    tablaR[52][6].tipo = D;
    tablaR[52][6].valor = 43;
    
    tablaR[52][7].tipo = D;
    tablaR[52][7].valor = 42;
    
    tablaR[52][33].tipo = D;
    tablaR[52][33].valor = 54;
    
    tablaR[53][1].tipo = R;
    tablaR[53][1].valor = 29;
    
    tablaR[53][5].tipo = R;
    tablaR[53][5].valor = 29;
    
    tablaR[53][6].tipo = R;
    tablaR[53][6].valor = 29;
    
    tablaR[53][7].tipo = R;
    tablaR[53][7].valor = 29;
    
    tablaR[54][2].tipo = D;
    tablaR[54][2].valor = 31;
    
    tablaR[54][3].tipo = D;
    tablaR[54][3].valor = 30;
    
    tablaR[54][4].tipo = D;
    tablaR[54][4].valor = 33;
    
    tablaR[54][34].tipo = D;
    tablaR[54][34].valor = 59;
    
    tablaR[55][12].tipo = D;
    tablaR[55][12].valor = 58;
    
    tablaR[56][12].tipo = D;
    tablaR[56][12].valor = 57;
    
    tablaR[57][2].tipo = D;
    tablaR[57][2].valor = 31;
    
    tablaR[57][3].tipo = D;
    tablaR[57][3].valor = 30;
    
    tablaR[57][4].tipo = D;
    tablaR[57][4].valor = 29;
    
    tablaR[57][9].tipo = D;
    tablaR[57][9].valor = 28;
    
    tablaR[57][10].tipo = D;
    tablaR[57][10].valor = 27;
    
    tablaR[57][14].tipo = D;
    tablaR[57][14].valor = 26;
    
    tablaR[57][15].tipo = D;
    tablaR[57][15].valor = 25;
    
    tablaR[57][27].tipo = D;
    tablaR[57][27].valor = 23;
    
    tablaR[57][28].tipo = D;
    tablaR[57][28].valor = 22;
    
    tablaR[57][29].tipo = D;
    tablaR[57][29].valor = 61;
    
    tablaR[57][30].tipo = D;
    tablaR[57][30].valor = 20;
    
    tablaR[57][31].tipo = D;
    tablaR[57][31].valor = 19;
    
    tablaR[57][32].tipo = D;
    tablaR[57][32].valor = 18;
    
    tablaR[57][34].tipo = D;
    tablaR[57][34].valor = 17;
    
    tablaR[58][2].tipo = D;
    tablaR[58][2].valor = 31;
    
    tablaR[58][3].tipo = D;
    tablaR[58][3].valor = 30;
    
    tablaR[58][4].tipo = D;
    tablaR[58][4].valor = 29;
    
    tablaR[58][9].tipo = D;
    tablaR[58][9].valor = 28;
    
    tablaR[58][10].tipo = D;
    tablaR[58][10].valor = 27;
    
    tablaR[58][14].tipo = D;
    tablaR[58][14].valor = 26;
    
    tablaR[58][15].tipo = D;
    tablaR[58][15].valor = 25;
    
    tablaR[58][27].tipo = D;
    tablaR[58][27].valor = 23;
    
    tablaR[58][28].tipo = D;
    tablaR[58][28].valor = 22;
    
    tablaR[58][29].tipo = D;
    tablaR[58][29].valor = 60;
    
    tablaR[58][30].tipo = D;
    tablaR[58][30].valor = 20;
    
    tablaR[58][31].tipo = D;
    tablaR[58][31].valor = 19;
    
    tablaR[58][32].tipo = D;
    tablaR[58][32].valor = 18;
    
    tablaR[58][34].tipo = D;
    tablaR[58][34].valor = 17;
    
    tablaR[59][1].tipo = R;
    tablaR[59][1].valor = 23;
    
    tablaR[59][9].tipo = R;
    tablaR[59][9].valor = 23;
    
    tablaR[60][11].tipo = D;
    tablaR[60][11].valor = 63;
    
    tablaR[61][11].tipo = D;
    tablaR[61][11].valor = 62;
    
    tablaR[62][2].tipo = R;
    tablaR[62][2].valor = 16;
    
    tablaR[62][3].tipo = R;
    tablaR[62][3].valor = 16;
    
    tablaR[62][4].tipo = R;
    tablaR[62][4].valor = 16;
    
    tablaR[62][9].tipo = R;
    tablaR[62][9].valor = 16;
    
    tablaR[62][10].tipo = R;
    tablaR[62][10].valor = 16;
    
    tablaR[62][11].tipo = R;
    tablaR[62][11].valor = 16;
    
    tablaR[62][14].tipo = R;
    tablaR[62][14].valor = 16;
    
    tablaR[62][15].tipo = R;
    tablaR[62][15].valor = 16;
    
    tablaR[63][2].tipo = R;
    tablaR[63][2].valor = 17;
    
    tablaR[63][3].tipo = R;
    tablaR[63][3].valor = 17;
    
    tablaR[63][4].tipo = R;
    tablaR[63][4].valor = 17;
    
    tablaR[63][9].tipo = R;
    tablaR[63][9].valor = 17;
    
    tablaR[63][10].tipo = R;
    tablaR[63][10].valor = 17;
    
    tablaR[63][11].tipo = R;
    tablaR[63][11].valor = 17;
    
    tablaR[63][13].tipo = D;
    tablaR[63][13].valor = 64;
    
    tablaR[63][14].tipo = R;
    tablaR[63][14].valor = 17;
    
    tablaR[63][15].tipo = R;
    tablaR[63][15].valor = 17;
    
    tablaR[64][12].tipo = D;
    tablaR[64][12].valor = 65;
    
    tablaR[65][2].tipo = D;
    tablaR[65][2].valor = 31;
    
    tablaR[65][3].tipo = D;
    tablaR[65][3].valor = 30;
    
    tablaR[65][4].tipo = D;
    tablaR[65][4].valor = 29;
    
    tablaR[65][9].tipo = D;
    tablaR[65][9].valor = 28;
    
    tablaR[65][10].tipo = D;
    tablaR[65][10].valor = 27;
    
    tablaR[65][14].tipo = D;
    tablaR[65][14].valor = 26;
    
    tablaR[65][15].tipo = D;
    tablaR[65][15].valor = 25;
    
    tablaR[65][27].tipo = D;
    tablaR[65][27].valor = 23;
    
    tablaR[65][28].tipo = D;
    tablaR[65][28].valor = 22;
    
    tablaR[65][29].tipo = D;
    tablaR[65][29].valor = 66;
    
    tablaR[65][30].tipo = D;
    tablaR[65][30].valor = 20;
    
    tablaR[65][31].tipo = D;
    tablaR[65][31].valor = 19;
    
    tablaR[65][32].tipo = D;
    tablaR[65][32].valor = 18;
    
    tablaR[65][34].tipo = D;
    tablaR[65][34].valor = 17;
    
    tablaR[66][11].tipo = D;
    tablaR[66][11].valor = 67;
    
    tablaR[67][2].tipo = R;
    tablaR[67][2].valor = 18;
    
    tablaR[67][3].tipo = R;
    tablaR[67][3].valor = 18;
    
    tablaR[67][4].tipo = R;
    tablaR[67][4].valor = 18;
    
    tablaR[67][9].tipo = R;
    tablaR[67][9].valor = 18;
    
    tablaR[67][10].tipo = R;
    tablaR[67][10].valor = 18;
    
    tablaR[67][11].tipo = R;
    tablaR[67][11].valor = 18;
    
    tablaR[67][14].tipo = R;
    tablaR[67][14].valor = 18;
    
    tablaR[67][15].tipo = R;
    tablaR[67][15].valor = 18;
}
