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
    fprintf(stdout,"Size: %d\n\n", pila.size);
	
}

/**
 * Metodo que hace las corridas del analizador lexico
 *
 * @param void
 * @return void
 **/
int anasin(){
    initStack(&pila);
	push(&pila, convierteAInt("$"));
	push(&pila, convierteAInt("0"));
	int i = 0, t;
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
			cero	= gramatica[actual.valor].cadenaDerivacion[0];
			uno		= gramatica[actual.valor].cadenaDerivacion[1];
			if(gramatica[actual.valor ].derivaciones > 2){
                dos		= gramatica[actual.valor].cadenaDerivacion[2];
            }
			//fprintf(stdout, "cero:%s Uno:%s\n",cero,uno);
			
			// Si la derivacion no es a epsilon se hace pop
			if (!eq(uno,"epsilon")) {
				
				while ((p = convierteAString(pop(&pila))),uno) {
					//fprintf(stdout, "Pop:%s\n",p);
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
	noTerminales = 19;
	terminales	 = 21;
	estados		 = 71;
	derivacionesGramatica = 36;
    siguientes = 0;
	
	// Crea arreglo de no terminales
    arregloNoTerminales[0] = "PROGRAM";
    arregloNoTerminales[1] = "VAR_TYPE";
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
    arregloNoTerminales[13] = "ASSIGN_EXP";
    arregloNoTerminales[14] = "BINARY_EXP";
    arregloNoTerminales[15] = "BINARY_OP";
    arregloNoTerminales[16] = "PRIMARY_EXPR";
    arregloNoTerminales[17] = "EXPRESSION";
    arregloNoTerminales[18] = "CONSTANT";
	
    // Crea arreglo de terminales
    arregloTerminales[0] = "$";
    arregloTerminales[1] = "float_literal";
    arregloTerminales[2] = "int_literal";
    arregloTerminales[3] = "right_parenthesis";
    arregloTerminales[4] = "left_parenthesis";
    arregloTerminales[5] = "var_name";
    arregloTerminales[6] = "arith_op";
    arregloTerminales[7] = "rel_op";
    arregloTerminales[8] = "boolean_op";
    arregloTerminales[9] = "equal";
    arregloTerminales[10] = "semi_colon";
    arregloTerminales[11] = "return";
    arregloTerminales[12] = "right_curly_bracket";
    arregloTerminales[13] = "left_curly_bracket";
    arregloTerminales[14] = "else";
    arregloTerminales[15] = "if";
    arregloTerminales[16] = "while";
    arregloTerminales[17] = "main";
    arregloTerminales[18] = "int";
    arregloTerminales[19] = "comma";
    arregloTerminales[20] = "float";

	
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
    gramatica[1].cadenaDerivacion[1] = "VAR_TYPE";
    gramatica[1].cadenaDerivacion[2] = "VAR_LIST";
    gramatica[1].cadenaDerivacion[3] = "semi_colon";
    gramatica[1].derivaciones		 = 4;
    
    // VAR_TYPE -> int
    gramatica[2].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[2].cadenaDerivacion[1] = "int";
    gramatica[2].derivaciones = 2;
    
    // VAR_TYPE -> float
    gramatica[3].cadenaDerivacion[0] = "VAR_TYPE";
    gramatica[3].cadenaDerivacion[1] = "float";
    gramatica[3].derivaciones = 2;
    
    // VAR_LIST -> VAR_LIST comma VAR_ITEM
    gramatica[4].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[4].cadenaDerivacion[1] = "VAR_LIST";
    gramatica[4].cadenaDerivacion[2] = "comma";
    gramatica[4].cadenaDerivacion[3] = "VAR_ITEM";
    gramatica[4].derivaciones = 4;
    
    // VAR_LIST -> VAR_ITEM
    gramatica[5].cadenaDerivacion[0] = "VAR_LIST";
    gramatica[5].cadenaDerivacion[1] = "VAR_ITEM";
    gramatica[5].derivaciones = 2;
    
    // VAR_ITEM -> var_name
    gramatica[6].cadenaDerivacion[0] = "VAR_ITEM";
    gramatica[6].cadenaDerivacion[1] = "var_name";
    gramatica[6].derivaciones = 2;
    
    // MAIN_DEF -> int main left_parenthesis right_parenthesis left_curly_bracket FUNCTION_BODY right_curly_bracket.
    gramatica[7].cadenaDerivacion[0] = "MAIN_DEF";
    gramatica[7].cadenaDerivacion[1] = "int";
    gramatica[7].cadenaDerivacion[2] = "main";
    gramatica[7].cadenaDerivacion[3] = "left_parenthesis";
    gramatica[7].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[7].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[7].cadenaDerivacion[6] = "FUNCTION_BODY";
    gramatica[7].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[7].derivaciones = 8;
    
    // FUNCTION_BODY -> INTERNAL_DECLARATIONS STATEMENT_LIST.
    gramatica[8].cadenaDerivacion[0] = "FUNCTION_BODY";
    gramatica[8].cadenaDerivacion[1] = "INTERNAL_DECLARATIONS";
    gramatica[8].cadenaDerivacion[2] = "STATEMENT_LIST";
    gramatica[8].derivaciones = 3;
    
    // INTERNAL_DECLARATIONS -> DECLARATION INTERNAL_DECLARATIONS
    gramatica[9].cadenaDerivacion[0] = "INTERNAL_DECLARATIONS";
    gramatica[9].cadenaDerivacion[1] = "DECLARATION";
    gramatica[9].cadenaDerivacion[2] = "INTERNAL_DECLARATIONS";
    gramatica[9].derivaciones = 3;
    
    // INTERNAL_DECLARATIONS -> <epsilon>
    gramatica[10].cadenaDerivacion[0] = "INTERNAL_DECLARATIONS";
    gramatica[10].cadenaDerivacion[1] = "epsilon";
    gramatica[10].derivaciones = 2;
    
    // STATEMENT_LIST -> STATEMENT STATEMENT_LIST
    gramatica[11].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[11].cadenaDerivacion[1] = "STATEMENT";
    gramatica[11].cadenaDerivacion[2] = "STATEMENT_LIST";
    gramatica[11].derivaciones = 3;
    
    // STATEMENT_LIST -> <epsilon>
    gramatica[12].cadenaDerivacion[0] = "STATEMENT_LIST";
    gramatica[12].cadenaDerivacion[1] = "epsilon";
    gramatica[12].derivaciones = 2;
    
    // STATEMENT -> IF_STATEMENT
    gramatica[13].cadenaDerivacion[0] = "STATEMENT";
    gramatica[13].cadenaDerivacion[1] = "IF_STATEMENT";
    gramatica[13].derivaciones = 2;
    
    // STATEMENT -> EXPRESSION semi_colon
    gramatica[14].cadenaDerivacion[0] = "STATEMENT";
    gramatica[14].cadenaDerivacion[1] = "EXPRESSION_STATEMENT";
    gramatica[14].cadenaDerivacion[2] = "semi_colon";
    gramatica[14].derivaciones = 3;
    
    // STATEMENT -> WHILE_STATEMENT
    gramatica[15].cadenaDerivacion[0] = "STATEMENT";
    gramatica[15].cadenaDerivacion[1] = "WHILE_STATEMENT";
    gramatica[15].derivaciones = 2;
    
    // STATEMENT -> RETURN_STATEMENT
    gramatica[16].cadenaDerivacion[0] = "STATEMENT";
    gramatica[16].cadenaDerivacion[1] = "RETURN_STATEMENT";
    gramatica[16].derivaciones = 2;
    
    // STATEMENT -> semi_colon
    gramatica[17].cadenaDerivacion[0] = "STATEMENT";
    gramatica[17].cadenaDerivacion[1] = "semi_colon";
    gramatica[17].derivaciones = 2;
    
    // WHILE_STATEMENT -> while left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket
    gramatica[18].cadenaDerivacion[0] = "WHILE_STATEMENT";
    gramatica[18].cadenaDerivacion[1] = "while";
    gramatica[18].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[18].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[18].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[18].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[18].cadenaDerivacion[6] = "STATEMENT";
    gramatica[18].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[18].derivaciones = 8;
    
    // IF_STATEMENT -> if left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket
    gramatica[19].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[19].cadenaDerivacion[1] = "if";
    gramatica[19].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[19].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[19].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[19].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[19].cadenaDerivacion[6] = "STATEMENT";
    gramatica[19].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[19].derivaciones = 8;
    
    
    // IF_STATEMENT -> if left_parenthesis EXPRESSION right_parenthesis left_curly_bracket STATEMENT right_curly_bracket else left_curly_bracket STATEMENT right_curly_bracket
    gramatica[20].cadenaDerivacion[0] = "IF_STATEMENT";
    gramatica[20].cadenaDerivacion[1] = "if";
    gramatica[20].cadenaDerivacion[2] = "left_parenthesis";
    gramatica[20].cadenaDerivacion[3] = "EXPRESSION";
    gramatica[20].cadenaDerivacion[4] = "right_parenthesis";
    gramatica[20].cadenaDerivacion[5] = "left_curly_bracket";
    gramatica[20].cadenaDerivacion[6] = "STATEMENT";
    gramatica[20].cadenaDerivacion[7] = "right_curly_bracket";
    gramatica[20].cadenaDerivacion[8] = "else";
    gramatica[20].cadenaDerivacion[9] = "left_curly_bracket";
    gramatica[20].cadenaDerivacion[10] = "STATEMENT";
    gramatica[20].cadenaDerivacion[11] = "right_curly_bracket";
    gramatica[20].derivaciones = 12;
    
    // RETURN_STATEMENT -> return EXPRESSION semi_colon
    gramatica[21].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[21].cadenaDerivacion[1] = "return";
    gramatica[21].cadenaDerivacion[2] = "EXPRESSION";
    gramatica[21].cadenaDerivacion[3] = "semi_colon";
    gramatica[21].derivaciones = 4;
    
    // RETURN_STATEMENT -> return semi_colon
    gramatica[22].cadenaDerivacion[0] = "RETURN_STATEMENT";
    gramatica[22].cadenaDerivacion[1] = "return";
    gramatica[22].cadenaDerivacion[2] = "semi_colon";
    gramatica[22].derivaciones = 3;
    
    // EXPRESSION -> ASSIGN_EXP
    gramatica[23].cadenaDerivacion[0] = "EXPRESSION";
    gramatica[23].cadenaDerivacion[1] = "ASSIGN_EXP";
    gramatica[23].derivaciones = 2;
    
    // ASSIGN_EXP -> var_name equal BINARY_EXP
    gramatica[24].cadenaDerivacion[0] = "var_name";
    gramatica[24].cadenaDerivacion[1] = "equal";
    gramatica[24].cadenaDerivacion[2] = "BINARY_EXP";
    gramatica[24].derivaciones = 3;
    
    // ASSIGN_EXP -> BINARY_EXP
    gramatica[25].cadenaDerivacion[0] = "ASSIGN_EXP";
    gramatica[25].cadenaDerivacion[1] = "BINARY_EXP";
    gramatica[25].derivaciones = 2;
    
    // BINARY_EXP -> BINARY_EXP BINARY_OP PRIMARY_EXPR
    gramatica[26].cadenaDerivacion[0] = "BINARY_EXP";
    gramatica[26].cadenaDerivacion[1] = "BINARY_EXP";
    gramatica[26].cadenaDerivacion[2] = "BINARY_OP";
    gramatica[26].cadenaDerivacion[3] = "PRIMARY_EXPR";
    gramatica[26].derivaciones = 4;
    
    // BINARY_EXP -> PRIMARY_EXPR
    gramatica[27].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[27].cadenaDerivacion[1] = "PRIMARY_EXPR";
    gramatica[27].derivaciones = 2;
    
    // BINARY_OP -> boolean_op
    gramatica[28].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[28].cadenaDerivacion[1] = "boolean_op";
    gramatica[28].derivaciones = 2;
    
    // BINARY_OP -> rel_op
    gramatica[29].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[29].cadenaDerivacion[1] = "rel_op";
    gramatica[29].derivaciones = 2;
    
    // BINARY_OP -> arith_op
    gramatica[30].cadenaDerivacion[0] = "BINARY_OP";
    gramatica[30].cadenaDerivacion[1] = "arith_op";
    gramatica[30].derivaciones = 2;
    
    // PRIMARY_EXPR -> var_name
    gramatica[31].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[31].cadenaDerivacion[1] = "var_name";
    gramatica[31].derivaciones = 2;
    
    // PRIMARY_EXPR -> CONSTANT
    gramatica[32].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[32].cadenaDerivacion[1] = "CONSTANT";
    gramatica[32].derivaciones = 2;
    
    // PRIMARY_EXPR -> left_parenthesis PRIMARY_EXPR right_parenthesis
    gramatica[33].cadenaDerivacion[0] = "PRIMARY_EXPR";
    gramatica[33].cadenaDerivacion[1] = "left_parenthesis";
    gramatica[33].cadenaDerivacion[2] = "PRIMARY_EXPR";
    gramatica[33].cadenaDerivacion[3] = "right_parenthesis";
    gramatica[33].derivaciones = 4;
    
    // CONSTANT -> int_literal
    gramatica[34].cadenaDerivacion[0] = "CONSTANT";
    gramatica[34].cadenaDerivacion[1] = "int_literal";
    gramatica[34].derivaciones = 2;
    
    // CONSTANT -> float_literal
    gramatica[35].cadenaDerivacion[0] = "CONSTANT";
    gramatica[35].cadenaDerivacion[1] = "float_literal";
    gramatica[35].derivaciones = 2;
    
    
	// Inicializa la tabla con valores default
	inicializaTabla(tablaR);
    
	// Tabla
    tablaR[0][18].tipo = D;
    tablaR[0][18].valor = 3;
    
    tablaR[0][21].tipo = D;
    tablaR[0][21].valor = 2;
    
    tablaR[0][25].tipo = D;
    tablaR[0][25].valor = 1;
    
    tablaR[1][0].tipo = R;
    tablaR[1][0].valor = 0;
    
    tablaR[2][0].tipo = ACEPTA;
    tablaR[2][0].valor = ERR;
    
    tablaR[3][17].tipo = D;
    tablaR[3][17].valor = 4;
    
    tablaR[4][4].tipo = D;
    tablaR[4][4].valor = 5;
    
    tablaR[5][3].tipo = D;
    tablaR[5][3].valor = 6;
    
    tablaR[6][13].tipo = D;
    tablaR[6][13].valor = 7;
    
    tablaR[7][1].tipo = R;
    tablaR[7][1].valor = 10;
    
    tablaR[7][2].tipo = R;
    tablaR[7][2].valor = 10;
    
    tablaR[7][4].tipo = R;
    tablaR[7][4].valor = 10;
    
    tablaR[7][5].tipo = R;
    tablaR[7][5].valor = 10;
    
    tablaR[7][10].tipo = R;
    tablaR[7][10].valor = 10;
    
    tablaR[7][11].tipo = R;
    tablaR[7][11].valor = 10;
    
    tablaR[7][12].tipo = R;
    tablaR[7][12].valor = 10;
    
    tablaR[7][15].tipo = R;
    tablaR[7][15].valor = 10;
    
    tablaR[7][16].tipo = R;
    tablaR[7][16].valor = 10;
    
    tablaR[7][18].tipo = D;
    tablaR[7][18].valor = 13;
    
    tablaR[7][20].tipo = D;
    tablaR[7][20].valor = 12;
    
    tablaR[7][22].tipo = D;
    tablaR[7][22].valor = 11;
    
    tablaR[7][26].tipo = D;
    tablaR[7][26].valor = 10;
    
    tablaR[7][27].tipo = D;
    tablaR[7][27].valor = 9;
    
    tablaR[7][28].tipo = D;
    tablaR[7][28].valor = 8;
    
    tablaR[8][1].tipo = D;
    tablaR[8][1].valor = 36;
    
    tablaR[8][2].tipo = D;
    tablaR[8][2].valor = 35;
    
    tablaR[8][4].tipo = D;
    tablaR[8][4].valor = 34;
    
    tablaR[8][5].tipo = D;
    tablaR[8][5].valor = 33;
    
    tablaR[8][10].tipo = D;
    tablaR[8][10].valor = 32;
    
    tablaR[8][11].tipo = D;
    tablaR[8][11].valor = 31;
    
    tablaR[8][12].tipo = R;
    tablaR[8][12].valor = 12;
    
    tablaR[8][15].tipo = D;
    tablaR[8][15].valor = 30;
    
    tablaR[8][16].tipo = D;
    tablaR[8][16].valor = 29;
    
    tablaR[8][29].tipo = D;
    tablaR[8][29].valor = 28;
    
    tablaR[8][30].tipo = D;
    tablaR[8][30].valor = 27;
    
    tablaR[8][31].tipo = D;
    tablaR[8][31].valor = 26;
    
    tablaR[8][32].tipo = D;
    tablaR[8][32].valor = 25;
    
    tablaR[8][33].tipo = D;
    tablaR[8][33].valor = 24;
    
    tablaR[8][34].tipo = D;
    tablaR[8][34].valor = 23;
    
    tablaR[8][35].tipo = D;
    tablaR[8][35].valor = 22;
    
    tablaR[8][37].tipo = D;
    tablaR[8][37].valor = 21;
    
    tablaR[8][38].tipo = D;
    tablaR[8][38].valor = 20;
    
    tablaR[8][39].tipo = D;
    tablaR[8][39].valor = 19;
    
    tablaR[9][1].tipo = R;
    tablaR[9][1].valor = 10;
    
    tablaR[9][2].tipo = R;
    tablaR[9][2].valor = 10;
    
    tablaR[9][4].tipo = R;
    tablaR[9][4].valor = 10;
    
    tablaR[9][5].tipo = R;
    tablaR[9][5].valor = 10;
    
    tablaR[9][10].tipo = R;
    tablaR[9][10].valor = 10;
    
    tablaR[9][11].tipo = R;
    tablaR[9][11].valor = 10;
    
    tablaR[9][12].tipo = R;
    tablaR[9][12].valor = 10;
    
    tablaR[9][15].tipo = R;
    tablaR[9][15].valor = 10;
    
    tablaR[9][16].tipo = R;
    tablaR[9][16].valor = 10;
    
    tablaR[9][18].tipo = D;
    tablaR[9][18].valor = 13;
    
    tablaR[9][20].tipo = D;
    tablaR[9][20].valor = 12;
    
    tablaR[9][22].tipo = D;
    tablaR[9][22].valor = 11;
    
    tablaR[9][27].tipo = D;
    tablaR[9][27].valor = 9;
    
    tablaR[9][28].tipo = D;
    tablaR[9][28].valor = 18;
    
    tablaR[10][12].tipo = D;
    tablaR[10][12].valor = 17;
    
    tablaR[11][5].tipo = D;
    tablaR[11][5].valor = 16;
    
    tablaR[11][23].tipo = D;
    tablaR[11][23].valor = 15;
    
    tablaR[11][24].tipo = D;
    tablaR[11][24].valor = 14;
    
    tablaR[12][5].tipo = R;
    tablaR[12][5].valor = 3;
    
    tablaR[13][5].tipo = R;
    tablaR[13][5].valor = 2;
    
    tablaR[14][10].tipo = R;
    tablaR[14][10].valor = 5;
    
    tablaR[14][19].tipo = R;
    tablaR[14][19].valor = 5;
    
    tablaR[15][10].tipo = D;
    tablaR[15][10].valor = 50;
    
    tablaR[15][19].tipo = D;
    tablaR[15][19].valor = 49;
    
    tablaR[16][10].tipo = R;
    tablaR[16][10].valor = 6;
    
    tablaR[16][19].tipo = R;
    tablaR[16][19].valor = 6;
    
    tablaR[17][0].tipo = R;
    tablaR[17][0].valor = 7;
    
    tablaR[18][1].tipo = R;
    tablaR[18][1].valor = 9;
    
    tablaR[18][2].tipo = R;
    tablaR[18][2].valor = 9;
    
    tablaR[18][4].tipo = R;
    tablaR[18][4].valor = 9;
    
    tablaR[18][5].tipo = R;
    tablaR[18][5].valor = 9;
    
    tablaR[18][10].tipo = R;
    tablaR[18][10].valor = 9;
    
    tablaR[18][11].tipo = R;
    tablaR[18][11].valor = 9;
    
    tablaR[18][12].tipo = R;
    tablaR[18][12].valor = 9;
    
    tablaR[18][15].tipo = R;
    tablaR[18][15].valor = 9;
    
    tablaR[18][16].tipo = R;
    tablaR[18][16].valor = 9;
    
    tablaR[19][3].tipo = R;
    tablaR[19][3].valor = 32;
    
    tablaR[19][6].tipo = R;
    tablaR[19][6].valor = 32;
    
    tablaR[19][7].tipo = R;
    tablaR[19][7].valor = 32;
    
    tablaR[19][8].tipo = R;
    tablaR[19][8].valor = 32;
    
    tablaR[19][10].tipo = R;
    tablaR[19][10].valor = 32;
    
    tablaR[20][10].tipo = D;
    tablaR[20][10].valor = 48;
    
    tablaR[21][3].tipo = R;
    tablaR[21][3].valor = 27;
    
    tablaR[21][6].tipo = R;
    tablaR[21][6].valor = 27;
    
    tablaR[21][7].tipo = R;
    tablaR[21][7].valor = 27;
    
    tablaR[21][8].tipo = R;
    tablaR[21][8].valor = 27;
    
    tablaR[21][10].tipo = R;
    tablaR[21][10].valor = 27;
    
    tablaR[22][3].tipo = R;
    tablaR[22][3].valor = 25;
    
    tablaR[22][6].tipo = D;
    tablaR[22][6].valor = 47;
    
    tablaR[22][7].tipo = D;
    tablaR[22][7].valor = 46;
    
    tablaR[22][8].tipo = D;
    tablaR[22][8].valor = 45;
    
    tablaR[22][10].tipo = R;
    tablaR[22][10].valor = 25;
    
    tablaR[22][36].tipo = D;
    tablaR[22][36].valor = 44;
    
    tablaR[23][3].tipo = R;
    tablaR[23][3].valor = 23;
    
    tablaR[23][10].tipo = R;
    tablaR[23][10].valor = 23;
    
    tablaR[24][1].tipo = R;
    tablaR[24][1].valor = 16;
    
    tablaR[24][2].tipo = R;
    tablaR[24][2].valor = 16;
    
    tablaR[24][4].tipo = R;
    tablaR[24][4].valor = 16;
    
    tablaR[24][5].tipo = R;
    tablaR[24][5].valor = 16;
    
    tablaR[24][10].tipo = R;
    tablaR[24][10].valor = 16;
    
    tablaR[24][11].tipo = R;
    tablaR[24][11].valor = 16;
    
    tablaR[24][12].tipo = R;
    tablaR[24][12].valor = 16;
    
    tablaR[24][15].tipo = R;
    tablaR[24][15].valor = 16;
    
    tablaR[24][16].tipo = R;
    tablaR[24][16].valor = 16;
    
    tablaR[25][1].tipo = D;
    tablaR[25][1].valor = 36;
    
    tablaR[25][2].tipo = D;
    tablaR[25][2].valor = 35;
    
    tablaR[25][4].tipo = D;
    tablaR[25][4].valor = 34;
    
    tablaR[25][5].tipo = D;
    tablaR[25][5].valor = 33;
    
    tablaR[25][10].tipo = D;
    tablaR[25][10].valor = 32;
    
    tablaR[25][11].tipo = D;
    tablaR[25][11].valor = 31;
    
    tablaR[25][12].tipo = R;
    tablaR[25][12].valor = 12;
    
    tablaR[25][15].tipo = D;
    tablaR[25][15].valor = 30;
    
    tablaR[25][16].tipo = D;
    tablaR[25][16].valor = 29;
    
    tablaR[25][29].tipo = D;
    tablaR[25][29].valor = 43;
    
    tablaR[25][30].tipo = D;
    tablaR[25][30].valor = 27;
    
    tablaR[25][31].tipo = D;
    tablaR[25][31].valor = 26;
    
    tablaR[25][32].tipo = D;
    tablaR[25][32].valor = 25;
    
    tablaR[25][33].tipo = D;
    tablaR[25][33].valor = 24;
    
    tablaR[25][34].tipo = D;
    tablaR[25][34].valor = 23;
    
    tablaR[25][35].tipo = D;
    tablaR[25][35].valor = 22;
    
    tablaR[25][37].tipo = D;
    tablaR[25][37].valor = 21;
    
    tablaR[25][38].tipo = D;
    tablaR[25][38].valor = 20;
    
    tablaR[25][39].tipo = D;
    tablaR[25][39].valor = 19;
    
    tablaR[26][1].tipo = R;
    tablaR[26][1].valor = 13;
    
    tablaR[26][2].tipo = R;
    tablaR[26][2].valor = 13;
    
    tablaR[26][4].tipo = R;
    tablaR[26][4].valor = 13;
    
    tablaR[26][5].tipo = R;
    tablaR[26][5].valor = 13;
    
    tablaR[26][10].tipo = R;
    tablaR[26][10].valor = 13;
    
    tablaR[26][11].tipo = R;
    tablaR[26][11].valor = 13;
    
    tablaR[26][12].tipo = R;
    tablaR[26][12].valor = 13;
    
    tablaR[26][15].tipo = R;
    tablaR[26][15].valor = 13;
    
    tablaR[26][16].tipo = R;
    tablaR[26][16].valor = 13;
    
    tablaR[27][1].tipo = R;
    tablaR[27][1].valor = 15;
    
    tablaR[27][2].tipo = R;
    tablaR[27][2].valor = 15;
    
    tablaR[27][4].tipo = R;
    tablaR[27][4].valor = 15;
    
    tablaR[27][5].tipo = R;
    tablaR[27][5].valor = 15;
    
    tablaR[27][10].tipo = R;
    tablaR[27][10].valor = 15;
    
    tablaR[27][11].tipo = R;
    tablaR[27][11].valor = 15;
    
    tablaR[27][12].tipo = R;
    tablaR[27][12].valor = 15;
    
    tablaR[27][15].tipo = R;
    tablaR[27][15].valor = 15;
    
    tablaR[27][16].tipo = R;
    tablaR[27][16].valor = 15;
    
    tablaR[28][12].tipo = R;
    tablaR[28][12].valor = 8;
    
    tablaR[29][4].tipo = D;
    tablaR[29][4].valor = 42;
    
    tablaR[30][4].tipo = D;
    tablaR[30][4].valor = 41;
    
    tablaR[31][1].tipo = D;
    tablaR[31][1].valor = 36;
    
    tablaR[31][2].tipo = D;
    tablaR[31][2].valor = 35;
    
    tablaR[31][4].tipo = D;
    tablaR[31][4].valor = 34;
    
    tablaR[31][5].tipo = D;
    tablaR[31][5].valor = 33;
    
    tablaR[31][10].tipo = D;
    tablaR[31][10].valor = 40;
    
    tablaR[31][34].tipo = D;
    tablaR[31][34].valor = 23;
    
    tablaR[31][35].tipo = D;
    tablaR[31][35].valor = 22;
    
    tablaR[31][37].tipo = D;
    tablaR[31][37].valor = 21;
    
    tablaR[31][38].tipo = D;
    tablaR[31][38].valor = 39;
    
    tablaR[31][39].tipo = D;
    tablaR[31][39].valor = 19;
    
    tablaR[32][1].tipo = R;
    tablaR[32][1].valor = 17;
    
    tablaR[32][2].tipo = R;
    tablaR[32][2].valor = 17;
    
    tablaR[32][4].tipo = R;
    tablaR[32][4].valor = 17;
    
    tablaR[32][5].tipo = R;
    tablaR[32][5].valor = 17;
    
    tablaR[32][10].tipo = R;
    tablaR[32][10].valor = 17;
    
    tablaR[32][11].tipo = R;
    tablaR[32][11].valor = 17;
    
    tablaR[32][12].tipo = R;
    tablaR[32][12].valor = 17;
    
    tablaR[32][15].tipo = R;
    tablaR[32][15].valor = 17;
    
    tablaR[32][16].tipo = R;
    tablaR[32][16].valor = 17;
    
    tablaR[33][3].tipo = R;
    tablaR[33][3].valor = 31;
    
    tablaR[33][6].tipo = R;
    tablaR[33][6].valor = 31;
    
    tablaR[33][7].tipo = R;
    tablaR[33][7].valor = 31;
    
    tablaR[33][8].tipo = R;
    tablaR[33][8].valor = 31;
    
    tablaR[33][9].tipo = D;
    tablaR[33][9].valor = 38;
    
    tablaR[33][10].tipo = R;
    tablaR[33][10].valor = 31;
    
    tablaR[34][1].tipo = D;
    tablaR[34][1].valor = 36;
    
    tablaR[34][2].tipo = D;
    tablaR[34][2].valor = 35;
    
    tablaR[34][4].tipo = D;
    tablaR[34][4].valor = 34;
    
    tablaR[34][5].tipo = D;
    tablaR[34][5].valor = 33;
    
    tablaR[34][34].tipo = D;
    tablaR[34][34].valor = 23;
    
    tablaR[34][35].tipo = D;
    tablaR[34][35].valor = 22;
    
    tablaR[34][37].tipo = D;
    tablaR[34][37].valor = 21;
    
    tablaR[34][38].tipo = D;
    tablaR[34][38].valor = 37;
    
    tablaR[34][39].tipo = D;
    tablaR[34][39].valor = 19;
    
    tablaR[35][3].tipo = R;
    tablaR[35][3].valor = 34;
    
    tablaR[35][6].tipo = R;
    tablaR[35][6].valor = 34;
    
    tablaR[35][7].tipo = R;
    tablaR[35][7].valor = 34;
    
    tablaR[35][8].tipo = R;
    tablaR[35][8].valor = 34;
    
    tablaR[35][10].tipo = R;
    tablaR[35][10].valor = 34;
    
    tablaR[36][3].tipo = R;
    tablaR[36][3].valor = 35;
    
    tablaR[36][6].tipo = R;
    tablaR[36][6].valor = 35;
    
    tablaR[36][7].tipo = R;
    tablaR[36][7].valor = 35;
    
    tablaR[36][8].tipo = R;
    tablaR[36][8].valor = 35;
    
    tablaR[36][10].tipo = R;
    tablaR[36][10].valor = 35;
    
    tablaR[37][3].tipo = D;
    tablaR[37][3].valor = 58;
    
    tablaR[38][1].tipo = D;
    tablaR[38][1].valor = 36;
    
    tablaR[38][2].tipo = D;
    tablaR[38][2].valor = 35;
    
    tablaR[38][4].tipo = D;
    tablaR[38][4].valor = 34;
    
    tablaR[38][5].tipo = D;
    tablaR[38][5].valor = 57;
    
    tablaR[38][35].tipo = D;
    tablaR[38][35].valor = 56;
    
    tablaR[38][37].tipo = D;
    tablaR[38][37].valor = 21;
    
    tablaR[38][39].tipo = D;
    tablaR[38][39].valor = 19;
    
    tablaR[39][10].tipo = D;
    tablaR[39][10].valor = 55;
    
    tablaR[40][1].tipo = R;
    tablaR[40][1].valor = 22;
    
    tablaR[40][2].tipo = R;
    tablaR[40][2].valor = 22;
    
    tablaR[40][4].tipo = R;
    tablaR[40][4].valor = 22;
    
    tablaR[40][5].tipo = R;
    tablaR[40][5].valor = 22;
    
    tablaR[40][10].tipo = R;
    tablaR[40][10].valor = 22;
    
    tablaR[40][11].tipo = R;
    tablaR[40][11].valor = 22;
    
    tablaR[40][12].tipo = R;
    tablaR[40][12].valor = 22;
    
    tablaR[40][15].tipo = R;
    tablaR[40][15].valor = 22;
    
    tablaR[40][16].tipo = R;
    tablaR[40][16].valor = 22;
    
    tablaR[41][1].tipo = D;
    tablaR[41][1].valor = 36;
    
    tablaR[41][2].tipo = D;
    tablaR[41][2].valor = 35;
    
    tablaR[41][4].tipo = D;
    tablaR[41][4].valor = 34;
    
    tablaR[41][5].tipo = D;
    tablaR[41][5].valor = 33;
    
    tablaR[41][34].tipo = D;
    tablaR[41][34].valor = 23;
    
    tablaR[41][35].tipo = D;
    tablaR[41][35].valor = 22;
    
    tablaR[41][37].tipo = D;
    tablaR[41][37].valor = 21;
    
    tablaR[41][38].tipo = D;
    tablaR[41][38].valor = 54;
    
    tablaR[41][39].tipo = D;
    tablaR[41][39].valor = 19;
    
    tablaR[42][1].tipo = D;
    tablaR[42][1].valor = 36;
    
    tablaR[42][2].tipo = D;
    tablaR[42][2].valor = 35;
    
    tablaR[42][4].tipo = D;
    tablaR[42][4].valor = 34;
    
    tablaR[42][5].tipo = D;
    tablaR[42][5].valor = 33;
    
    tablaR[42][34].tipo = D;
    tablaR[42][34].valor = 23;
    
    tablaR[42][35].tipo = D;
    tablaR[42][35].valor = 22;
    
    tablaR[42][37].tipo = D;
    tablaR[42][37].valor = 21;
    
    tablaR[42][38].tipo = D;
    tablaR[42][38].valor = 53;
    
    tablaR[42][39].tipo = D;
    tablaR[42][39].valor = 19;
    
    tablaR[43][12].tipo = R;
    tablaR[43][12].valor = 11;
    
    tablaR[44][1].tipo = D;
    tablaR[44][1].valor = 36;
    
    tablaR[44][2].tipo = D;
    tablaR[44][2].valor = 35;
    
    tablaR[44][4].tipo = D;
    tablaR[44][4].valor = 34;
    
    tablaR[44][5].tipo = D;
    tablaR[44][5].valor = 57;
    
    tablaR[44][37].tipo = D;
    tablaR[44][37].valor = 52;
    
    tablaR[44][39].tipo = D;
    tablaR[44][39].valor = 19;
    
    tablaR[45][1].tipo = R;
    tablaR[45][1].valor = 28;
    
    tablaR[45][2].tipo = R;
    tablaR[45][2].valor = 28;
    
    tablaR[45][4].tipo = R;
    tablaR[45][4].valor = 28;
    
    tablaR[45][5].tipo = R;
    tablaR[45][5].valor = 28;
    
    tablaR[46][1].tipo = R;
    tablaR[46][1].valor = 29;
    
    tablaR[46][2].tipo = R;
    tablaR[46][2].valor = 29;
    
    tablaR[46][4].tipo = R;
    tablaR[46][4].valor = 29;
    
    tablaR[46][5].tipo = R;
    tablaR[46][5].valor = 29;
    
    tablaR[47][1].tipo = R;
    tablaR[47][1].valor = 30;
    
    tablaR[47][2].tipo = R;
    tablaR[47][2].valor = 30;
    
    tablaR[47][4].tipo = R;
    tablaR[47][4].valor = 30;
    
    tablaR[47][5].tipo = R;
    tablaR[47][5].valor = 30;
    
    tablaR[48][1].tipo = R;
    tablaR[48][1].valor = 14;
    
    tablaR[48][2].tipo = R;
    tablaR[48][2].valor = 14;
    
    tablaR[48][4].tipo = R;
    tablaR[48][4].valor = 14;
    
    tablaR[48][5].tipo = R;
    tablaR[48][5].valor = 14;
    
    tablaR[48][10].tipo = R;
    tablaR[48][10].valor = 14;
    
    tablaR[48][11].tipo = R;
    tablaR[48][11].valor = 14;
    
    tablaR[48][12].tipo = R;
    tablaR[48][12].valor = 14;
    
    tablaR[48][15].tipo = R;
    tablaR[48][15].valor = 14;
    
    tablaR[48][16].tipo = R;
    tablaR[48][16].valor = 14;
    
    tablaR[49][5].tipo = D;
    tablaR[49][5].valor = 16;
    
    tablaR[49][24].tipo = D;
    tablaR[49][24].valor = 51;
    
    tablaR[50][1].tipo = R;
    tablaR[50][1].valor = 1;
    
    tablaR[50][2].tipo = R;
    tablaR[50][2].valor = 1;
    
    tablaR[50][4].tipo = R;
    tablaR[50][4].valor = 1;
    
    tablaR[50][5].tipo = R;
    tablaR[50][5].valor = 1;
    
    tablaR[50][10].tipo = R;
    tablaR[50][10].valor = 1;
    
    tablaR[50][11].tipo = R;
    tablaR[50][11].valor = 1;
    
    tablaR[50][12].tipo = R;
    tablaR[50][12].valor = 1;
    
    tablaR[50][15].tipo = R;
    tablaR[50][15].valor = 1;
    
    tablaR[50][16].tipo = R;
    tablaR[50][16].valor = 1;
    
    tablaR[50][18].tipo = R;
    tablaR[50][18].valor = 1;
    
    tablaR[50][20].tipo = R;
    tablaR[50][20].valor = 1;
    
    tablaR[51][10].tipo = R;
    tablaR[51][10].valor = 4;
    
    tablaR[51][19].tipo = R;
    tablaR[51][19].valor = 4;
    
    tablaR[52][3].tipo = R;
    tablaR[52][3].valor = 26;
    
    tablaR[52][6].tipo = R;
    tablaR[52][6].valor = 26;
    
    tablaR[52][7].tipo = R;
    tablaR[52][7].valor = 26;
    
    tablaR[52][8].tipo = R;
    tablaR[52][8].valor = 26;
    
    tablaR[52][10].tipo = R;
    tablaR[52][10].valor = 26;
    
    tablaR[53][3].tipo = D;
    tablaR[53][3].valor = 60;
    
    tablaR[54][3].tipo = D;
    tablaR[54][3].valor = 59;
    
    tablaR[55][1].tipo = R;
    tablaR[55][1].valor = 21;
    
    tablaR[55][2].tipo = R;
    tablaR[55][2].valor = 21;
    
    tablaR[55][4].tipo = R;
    tablaR[55][4].valor = 21;
    
    tablaR[55][5].tipo = R;
    tablaR[55][5].valor = 21;
    
    tablaR[55][10].tipo = R;
    tablaR[55][10].valor = 21;
    
    tablaR[55][11].tipo = R;
    tablaR[55][11].valor = 21;
    
    tablaR[55][12].tipo = R;
    tablaR[55][12].valor = 21;
    
    tablaR[55][15].tipo = R;
    tablaR[55][15].valor = 21;
    
    tablaR[55][16].tipo = R;
    tablaR[55][16].valor = 21;
    
    tablaR[56][3].tipo = R;
    tablaR[56][3].valor = 24;
    
    tablaR[56][6].tipo = D;
    tablaR[56][6].valor = 47;
    
    tablaR[56][7].tipo = D;
    tablaR[56][7].valor = 46;
    
    tablaR[56][8].tipo = D;
    tablaR[56][8].valor = 45;
    
    tablaR[56][10].tipo = R;
    tablaR[56][10].valor = 24;
    
    tablaR[56][36].tipo = D;
    tablaR[56][36].valor = 44;
    
    tablaR[57][6].tipo = R;
    tablaR[57][6].valor = 31;
    
    tablaR[57][7].tipo = R;
    tablaR[57][7].valor = 31;
    
    tablaR[57][8].tipo = R;
    tablaR[57][8].valor = 31;
    
    tablaR[58][3].tipo = R;
    tablaR[58][3].valor = 33;
    
    tablaR[58][6].tipo = R;
    tablaR[58][6].valor = 33;
    
    tablaR[58][7].tipo = R;
    tablaR[58][7].valor = 33;
    
    tablaR[58][8].tipo = R;
    tablaR[58][8].valor = 33;
    
    tablaR[58][10].tipo = R;
    tablaR[58][10].valor = 33;
    
    tablaR[59][13].tipo = D;
    tablaR[59][13].valor = 62;
    
    tablaR[60][13].tipo = D;
    tablaR[60][13].valor = 61;
    
    tablaR[61][1].tipo = D;
    tablaR[61][1].valor = 36;
    
    tablaR[61][2].tipo = D;
    tablaR[61][2].valor = 35;
    
    tablaR[61][4].tipo = D;
    tablaR[61][4].valor = 34;
    
    tablaR[61][5].tipo = D;
    tablaR[61][5].valor = 33;
    
    tablaR[61][10].tipo = D;
    tablaR[61][10].valor = 32;
    
    tablaR[61][11].tipo = D;
    tablaR[61][11].valor = 31;
    
    tablaR[61][15].tipo = D;
    tablaR[61][15].valor = 30;
    
    tablaR[61][16].tipo = D;
    tablaR[61][16].valor = 29;
    
    tablaR[61][30].tipo = D;
    tablaR[61][30].valor = 27;
    
    tablaR[61][31].tipo = D;
    tablaR[61][31].valor = 26;
    
    tablaR[61][32].tipo = D;
    tablaR[61][32].valor = 64;
    
    tablaR[61][33].tipo = D;
    tablaR[61][33].valor = 24;
    
    tablaR[61][34].tipo = D;
    tablaR[61][34].valor = 23;
    
    tablaR[61][35].tipo = D;
    tablaR[61][35].valor = 22;
    
    tablaR[61][37].tipo = D;
    tablaR[61][37].valor = 21;
    
    tablaR[61][38].tipo = D;
    tablaR[61][38].valor = 20;
    
    tablaR[61][39].tipo = D;
    tablaR[61][39].valor = 19;
    
    tablaR[62][1].tipo = D;
    tablaR[62][1].valor = 36;
    
    tablaR[62][2].tipo = D;
    tablaR[62][2].valor = 35;
    
    tablaR[62][4].tipo = D;
    tablaR[62][4].valor = 34;
    
    tablaR[62][5].tipo = D;
    tablaR[62][5].valor = 33;
    
    tablaR[62][10].tipo = D;
    tablaR[62][10].valor = 32;
    
    tablaR[62][11].tipo = D;
    tablaR[62][11].valor = 31;
    
    tablaR[62][15].tipo = D;
    tablaR[62][15].valor = 30;
    
    tablaR[62][16].tipo = D;
    tablaR[62][16].valor = 29;
    
    tablaR[62][30].tipo = D;
    tablaR[62][30].valor = 27;
    
    tablaR[62][31].tipo = D;
    tablaR[62][31].valor = 26;
    
    tablaR[62][32].tipo = D;
    tablaR[62][32].valor = 63;
    
    tablaR[62][33].tipo = D;
    tablaR[62][33].valor = 24;
    
    tablaR[62][34].tipo = D;
    tablaR[62][34].valor = 23;
    
    tablaR[62][35].tipo = D;
    tablaR[62][35].valor = 22;
    
    tablaR[62][37].tipo = D;
    tablaR[62][37].valor = 21;
    
    tablaR[62][38].tipo = D;
    tablaR[62][38].valor = 20;
    
    tablaR[62][39].tipo = D;
    tablaR[62][39].valor = 19;
    
    tablaR[63][12].tipo = D;
    tablaR[63][12].valor = 66;
    
    tablaR[64][12].tipo = D;
    tablaR[64][12].valor = 65;
    
    tablaR[65][1].tipo = R;
    tablaR[65][1].valor = 18;
    
    tablaR[65][2].tipo = R;
    tablaR[65][2].valor = 18;
    
    tablaR[65][4].tipo = R;
    tablaR[65][4].valor = 18;
    
    tablaR[65][5].tipo = R;
    tablaR[65][5].valor = 18;
    
    tablaR[65][10].tipo = R;
    tablaR[65][10].valor = 18;
    
    tablaR[65][11].tipo = R;
    tablaR[65][11].valor = 18;
    
    tablaR[65][12].tipo = R;
    tablaR[65][12].valor = 18;
    
    tablaR[65][15].tipo = R;
    tablaR[65][15].valor = 18;
    
    tablaR[65][16].tipo = R;
    tablaR[65][16].valor = 18;
    
    tablaR[66][1].tipo = R;
    tablaR[66][1].valor = 19;
    
    tablaR[66][2].tipo = R;
    tablaR[66][2].valor = 19;
    
    tablaR[66][4].tipo = R;
    tablaR[66][4].valor = 19;
    
    tablaR[66][5].tipo = R;
    tablaR[66][5].valor = 19;
    
    tablaR[66][10].tipo = R;
    tablaR[66][10].valor = 19;
    
    tablaR[66][11].tipo = R;
    tablaR[66][11].valor = 19;
    
    tablaR[66][12].tipo = R;
    tablaR[66][12].valor = 19;
    
    tablaR[66][14].tipo = D;
    tablaR[66][14].valor = 67;
    
    tablaR[66][15].tipo = R;
    tablaR[66][15].valor = 19;
    
    tablaR[66][16].tipo = R;
    tablaR[66][16].valor = 19;
    
    tablaR[67][13].tipo = D;
    tablaR[67][13].valor = 68;
    
    tablaR[68][1].tipo = D;
    tablaR[68][1].valor = 36;
    
    tablaR[68][2].tipo = D;
    tablaR[68][2].valor = 35;
    
    tablaR[68][4].tipo = D;
    tablaR[68][4].valor = 34;
    
    tablaR[68][5].tipo = D;
    tablaR[68][5].valor = 33;
    
    tablaR[68][10].tipo = D;
    tablaR[68][10].valor = 32;
    
    tablaR[68][11].tipo = D;
    tablaR[68][11].valor = 31;
    
    tablaR[68][15].tipo = D;
    tablaR[68][15].valor = 30;
    
    tablaR[68][16].tipo = D;
    tablaR[68][16].valor = 29;
    
    tablaR[68][30].tipo = D;
    tablaR[68][30].valor = 27;
    
    tablaR[68][31].tipo = D;
    tablaR[68][31].valor = 26;
    
    tablaR[68][32].tipo = D;
    tablaR[68][32].valor = 69;
    
    tablaR[68][33].tipo = D;
    tablaR[68][33].valor = 24;
    
    tablaR[68][34].tipo = D;
    tablaR[68][34].valor = 23;
    
    tablaR[68][35].tipo = D;
    tablaR[68][35].valor = 22;
    
    tablaR[68][37].tipo = D;
    tablaR[68][37].valor = 21;
    
    tablaR[68][38].tipo = D;
    tablaR[68][38].valor = 20;
    
    tablaR[68][39].tipo = D;
    tablaR[68][39].valor = 19;
    
    tablaR[69][12].tipo = D;
    tablaR[69][12].valor = 70;
    
    tablaR[70][1].tipo = R;
    tablaR[70][1].valor = 20;
    
    tablaR[70][2].tipo = R;
    tablaR[70][2].valor = 20;
    
    tablaR[70][4].tipo = R;
    tablaR[70][4].valor = 20;
    
    tablaR[70][5].tipo = R;
    tablaR[70][5].valor = 20;
    
    tablaR[70][10].tipo = R;
    tablaR[70][10].valor = 20;
    
    tablaR[70][11].tipo = R;
    tablaR[70][11].valor = 20;
    
    tablaR[70][12].tipo = R;
    tablaR[70][12].valor = 20;
    
    tablaR[70][15].tipo = R;
    tablaR[70][15].valor = 20;
    
    tablaR[70][16].tipo = R;
    tablaR[70][16].valor = 20;
}
