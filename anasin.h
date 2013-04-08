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
#include "ctree.c"

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

/* Estructura token del MC, contiene un token de la tabla
 * nombre = nombre del token (variable)
 * tipo   = tipo de variable (float o int)
 * valor  = contenido de la variable
 * valorInicial = define si el token contiene un valor inicial o un valor actualizado por el anasem
 **/
typedef struct token {
	char *nombre[20];
    char *tipo[20];
    char *valor[20];
	int valorInicial;
}token;


FILE * tablaReglasFuente;
// Globales de cantidades
int noTerminales;
int terminales;
int estados;
int derivacionesGramatica; // Cuantas derivaciones en la gramatica
// Arreglo con valores de terminales y no terminales en string
char *arregloTerminales[22];
char *arregloNoTerminales[19];
char buf[BUFSIZ];
extern char escritura[BUFSIZ];
char *input[BUFSIZ]; // La traduccion, si es un no terminal o un terminal, por ejemplo VAR_TYPE
char *inputReal[BUFSIZ]; // El buffer tal cual (los valores)
// Arreglo temporal de tokens
char *tokenTemp[BUFSIZ];
// Variables inputSize y cuantosTokens compartidas con anasin
int inputSize;
int cuantosTokens; // Cantidad de tokens para la tabla de simbolos (float e int) en este caso

int errorSintactico = 0;
// Tabla de analisis SLR1
regla **tablaR;
derivacion *gramatica;
siguiente *siguientes;
token *tokens;
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
		return itoaC(num - 100);
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
    
    if(atoi(str) <0){
		printf("STR: %s\n", str);
		fprintf(stdout, "********************************%i", atoi(str));
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
 * Regresa 1 si el string dado es un terminal
 *
 * @param *char
 * @return int
 **/
int esTerminal(char *str){
	int i;
	for (i=0; i<terminales; i++) {
		if (eq(arregloTerminales[i], str)) {
			return 1;
		}
	}
    return 0;
}

/**
 * Regresa 1 si el string dado es un terminal
 *
 * @param *char
 * @return int
 **/
int esNoTerminal(char *str){
	int i;
	for (i=0; i<noTerminales; i++) {
		if (eq(arregloNoTerminales[i], str)) {
			return 1;
		}
	}
    return 0;
}

/**
 * Convierte el string dado a su localidad en la cadena de tokens
 *
 * @param *char
 * @return int
 **/
int getTokenIndex(char *str){
	int i;
	for (i=0; i<cuantosTokens; i++) {
		if (eq((char *) tokens[i].nombre, str)) {
			return i;
		}
	}
	return -1;
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
 * Imprime los tokens
 *
 * @param void
 * @return void
 **/
void imprimeTokens(){
	int i;
	fprintf(stdout, "\n*******TOKENS*******\nExisten %d tokens:\n",cuantosTokens);
	for (i = 0; i<cuantosTokens; i++) {
		fprintf(stdout, "%i:\t Nombre: %s \t Tipo: %s \t Valor: %s\n",i,(char *) tokens[i].nombre,(char *) tokens[i].tipo,(char *) tokens[i].valor);
	}
    fprintf(stdout, "\n");
}

/**
 * Estilo de impresion para el arbol
 *
 * @param void
 * @return void
 **/
void print_string(void* data, int indent, int islastchild, unsigned int* bitmask) {
    int i;
    
    /* print the current node's data with appropriate indentation */
    for (i = 1; i < indent; ++i) {
        /* use bitmasks to suppress printing unnecessary branches */
        if (!((*bitmask) & (1 << i))) {
            printf ("|   ");
        }
        else
            printf ("    ");
    }
    if (indent) {
        if (islastchild)
            printf ("`-- ");
        else
            printf ("|-- ");
    }
    printf ("%s\n", (char *)data);
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
			fprintf(stdout, "\t%s\t%-60s\n","","PASOS DE LA PILA");
			break;
			// Derivacion normal D#
		case 1:
			fprintf(stdout,"PILA:\t%s\nENTRADA:%s\n", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "ACCION:\tD%d",valor);
			fprintf(stdout, "\n");
			break;
			// Reduccion R#
		case 2:
			fprintf(stdout,"PILA:\t%s\nENTRADA:%s\n", imprimePila(ret), imprimeInputReal(ret1,i));
			// Tipo de accion
			fprintf(stdout, "ACCION: R%d: ",valor+1);
			imprimeGramatica(valor);
			fprintf(stdout, "\n");
			break;
			// Acepta
		case 3:
			fprintf(stdout, "PILA:\t%s\nENTRADA:%s\nCadena Aceptada\n", imprimePila(ret), imprimeInput(ret1,i));
			break;
			// Error Sintactico
		case 4:
			fprintf(stdout, "PILA:\t%s\nENTRADA:%s\nError Sintactico.", imprimePila(ret), imprimeInputReal(ret1,i));
			fprintf(stdout, "\n");
			errorSintactico = 1;
			break;
		default:
			break;
	}
    fprintf(stdout,"\n");
	
}

/**
 * Metodo que hace las corridas del analizador sintactico
 *
 * @param void
 * @return void
 **/
int anasin(){
    initStack(&pila);
	push(&pila, convierteAInt("$"));
	push(&pila, convierteAInt("0"));
	int i = 0, t, localidad;
	char *aux, *uno, *cero, *dos, *p, *auxVarType;
	// Para escribir menos
	regla actual;
	
	// Imprime Header
	imprimeFormato(0,-1,-1);
    
    // Inicializar el arbol
    struct Node* root = create_tree("#");
    
    // Inicializar valores del arbol
    int hijosDerivaciones, i_hijo;
	int cuentaHijo = 0;
	struct Node* hijos[50];
	int cuentaCreaHijo;
    struct Node* izq;
    struct Node* nodeNoTerminal;
    struct Node* nodeTerminal;
    
	while (1) {
        // Arbol
        fprintf(stdout, "\n--<Arbol Sintactico>--\n");
<<<<<<< HEAD
        traverse_node(root, print_string);
        fprintf(stdout, "\n--<Preorder>--\n");
        pre_order(root);
        fprintf(stdout, "\n");
=======
        //traverse_node(root, print_string);
>>>>>>> dfaa264034d718fc43e83a81adfb33f4385435af
        
		// Toma primer elemento
		aux = convierteAString(top(&pila));
        //fprintf(stdout, "(%d,%s) Tabla: %d", atoi(aux),input[i],tablaR[atoi(aux)][convierteAMat(input[i])].tipo);
		// Guardas lo que hay en la tabla en una variable
		actual = tablaR[atoi(aux)][convierteAMat(input[i])];
        //fprintf(stdout, "\ninput[i]:%s   Mat:%i",input[i],convierteAMat(input[i]));
		//fprintf(stdout, "\nactual valor:%i\n\n",actual.valor);
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
			push(&pila, convierteAInt(itoaC(actual.valor)));
			//fprintf(stdout, "\nAcastoy, Meto %s %d\n",input[i],actual.valor);
			//char ret[BUFSIZ];
			//fprintf(stdout, "DESPLAZA\n");
			//fprintf(stdout,"%s\t\n\n", imprimePila(ret));
			
            // Incrementa Valor
            i++;
		} else if (actual.tipo == R) {
			// Pop hasta encontrar en pila el primer valor de derivacion
			cero	= gramatica[actual.valor].cadenaDerivacion[0];
			uno		= gramatica[actual.valor].cadenaDerivacion[1];
			if(gramatica[actual.valor].derivaciones > 2){
				dos		= gramatica[actual.valor].cadenaDerivacion[2];
			}
			//fprintf(stdout, "cero:%s Uno:%s\n",cero,uno);

            hijosDerivaciones = gramatica[actual.valor].derivaciones - 1;
            cuentaHijo = 0;
            
            //fprintf(stdout, "hijosDerivaciones: %d\n", hijosDerivaciones);
            
            // Inicializa el arreglo con la cantidad de derivaciones que se necesitan
            for(cuentaCreaHijo = 0; cuentaCreaHijo < hijosDerivaciones; cuentaCreaHijo++){
                hijos[cuentaCreaHijo] = (struct Node *) malloc(sizeof(struct Node));
            }
            
			// Reducciones a int o float
			if(actual.valor == 2 || actual.valor == 3){
				//fprintf(stdout, "%s\n", "\nHago reduccion a VAR_TYPE\n\n ");
				auxVarType = uno;
			}else if(actual.valor == 6){
				// Reducciones a el nombre de la variable
				//fprintf(stdout, "%s\n", "\nHago reduccion a VAR_ITEM\n\n ");
				
				if(!eq(auxVarType,"")){
					// El valor top de la pila checarlo con inputReal
					localidad = getTokenIndex(inputReal[i-1]);
					
<<<<<<< HEAD
					// Guardar valor 
					strcpy((char *) tokens[localidad].tipo, auxVarType);
          //printf("TIPO DE VALIABLE: %s\n", auxVarType);
=======
					// Guardar valor
					strcpy((char *) tokens[localidad].tipo, auxVarType);
                    printf("TIPO DE VALIABLE: %s\n", auxVarType);
>>>>>>> dfaa264034d718fc43e83a81adfb33f4385435af
					
				} else if(actual.valor == 1){
					// Poner vartype en 0
					auxVarType = "";
				}
			}
			
			// Reducciones a int_literal y float_literal para sacar los respectivos valores
			// i = ; i-1 = valor i-2 = '=' i-3 = tipo
			if(actual.valor == 34 || actual.valor == 35){
				//fprintf(stdout, "%s\n", "\nHago reduccion a LITERAL\n\n ");
				localidad = getTokenIndex(inputReal[i-3]);
				
				// Guardar valor 
				if(tokens[localidad].valorInicial == 1){
					strcpy((char *) tokens[localidad].valor, inputReal[i-1]);
          //printf("VALOR DE VARIABLE: %s\n", inputReal[i-1]);
					//tokens[localidad].valorInicial = 0;
				}
			}
             
            
			// Si la derivacion no es a epsilon se hace pop
			if (!eq(uno,"epsilon")) {
				
				while ((p = convierteAString(pop(&pila))),uno) {
					//fprintf(stdout, "Pop:%s\n",p);
					if (eq(p,"$")) {
						imprimeFormato(4, i, -1);
						fprintf(stdout, "No encontre %s, salir.\n",uno);
						return -1;
					}
                    // Chequeo para el arbol
                    if(esTerminal(p)){
                        //fprintf(stdout, "\nValor: %s - Terminal", p);
                        // Lo metes directo como hijo de slash
                        
                        /*
                            TODO: en vez de agregar p se agrega el valor de p
                         */
                        
                        fprintf(stdout, "\nAGREGA:%s\n",p);
                        //nodeTerminal = create_node_under(root, p, 1);
                        nodeTerminal = create_node_under(root, tokens[localidad].tipo, tokens[localidad].valor);
                        hijos[cuentaHijo] = nodeTerminal;
                        cuentaHijo++;
                        
                    } else if(esNoTerminal(p)){
                        //fprintf(stdout, "\nValor: %s - No Terminal", p);
                        // Lo buscas en el arbol entre los hijos del guitarrista de guns n roses
                        // Al encontrarlo marcas el nodo como hijo
                        //fprintf(stdout, "\nBusco %s...", p);
                        nodeNoTerminal = searchFirstLevel(root, p, strcmp);
                        if(nodeNoTerminal == NULL){
                            //fprintf(stdout, "No se encuentra.\n");
                        } else {
                            //fprintf(stdout, "Agregado a hijos[].\n");
                        }
                        hijos[cuentaHijo] = nodeNoTerminal;
                        cuentaHijo++;
                    }
                    
                    if (eq(p,uno)) {
                        // Agregas el lado izquierdo (cero) al arbol
                        //fprintf(stdout, "\nAgrega lado izquierdo...\n");
                        izq = create_node_under(root, cero, 0);
                        //traverse_node(root, print_string);
                        //printf("\nMueves los hijos abajo\n");
                        for (i_hijo = 0; i_hijo < cuentaHijo; i_hijo++) {
                            move_node_under(hijos[i_hijo], izq);
                        }
                        
                        //traverse_node(root, print_string);
                        
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
			}else{
                // Deriva a epsilon: agregas el no terminal como hijo de /
                fprintf(stdout, "\nDeriva a epsilon, agrega el terminal como hijo de /...\n");
                izq = create_node_under(root, cero, 0);
                //traverse_node(root, print_string);
            }
            
			// t siempre va a ser un numero (el renglon de la tabla)
			t = top(&pila);
			// Agrega el derivado a la pila
			push(&pila,		convierteAInt(cero));
			push(&pila,		convierteAInt(itoaC(tablaR[atoi(convierteAString(t))][convierteAMat(cero)].valor)));
            
			imprimeFormato(2, i, actual.valor);
            /*char ret[BUFSIZ];
            fprintf(stdout, "METELO\n");
            fprintf(stdout,"%s\t\n\n", imprimePila(ret));*/
            
		} else if (actual.tipo == ERR) {
			imprimeFormato(4, i, -1);
			return -1;
		}
		//fprintf(stdout, "pila:%s\n",imprimePila(ret));
        fprintf(stdout, "\n-------------------------------------------\n");
        traverse_node(root, print_string);
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
void inicializaGramatica(){
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
	int row,i;
	for (row=0; row<estados; row++) {
		tablaR[row] = (regla*)malloc(sizeof(regla) * (terminales + noTerminales + 1));
	}
	
    inicializaTabla(tablaR);
    
	// Arreglo gramatica
	gramatica = (derivacion *)malloc(sizeof(derivacion)*derivacionesGramatica);
	
    // Arreglo tokens de 100 (default)
    tokens = (token *)malloc(sizeof(token)*cuantosTokens);
    
    // Pasa los valores del token temporal a la estructura
    for (i = 0; i<cuantosTokens; i++) {
        strcpy((char *) tokens[i].nombre, tokenTemp[i]);
        strcpy((char *) tokens[i].tipo, "");
        strcpy((char *) tokens[i].valor, "");
		tokens[i].valorInicial = 1;
    }
    
    imprimeTokens();
    
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
    gramatica[14].cadenaDerivacion[1] = "EXPRESSION";
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
    gramatica[24].cadenaDerivacion[0] = "ASSIGN_EXP";
    gramatica[24].cadenaDerivacion[1] = "var_name";
    gramatica[24].cadenaDerivacion[2] = "equal";
    gramatica[24].cadenaDerivacion[3] = "BINARY_EXP";
    gramatica[24].derivaciones = 4;
    
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
    gramatica[27].cadenaDerivacion[0] = "BINARY_EXP";
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
    gramatica[33].cadenaDerivacion[2] = "EXPRESSION";
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
    tablaR[0][18].valor = 1;
    
    tablaR[0][21].tipo = D;
    tablaR[0][21].valor = 3;
    
    tablaR[0][25].tipo = D;
    tablaR[0][25].valor = 2;
    
    tablaR[1][17].tipo = D;
    tablaR[1][17].valor = 4;
    
    tablaR[2][0].tipo = R;
    tablaR[2][0].valor = 0;
    
    tablaR[3][0].tipo = ACEPTA;
    tablaR[3][0].valor = ERR;
    
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
    tablaR[7][18].valor = 9;
    
    tablaR[7][20].tipo = D;
    tablaR[7][20].valor = 8;
    
    tablaR[7][22].tipo = D;
    tablaR[7][22].valor = 13;
    
    tablaR[7][26].tipo = D;
    tablaR[7][26].valor = 11;
    
    tablaR[7][27].tipo = D;
    tablaR[7][27].valor = 10;
    
    tablaR[7][28].tipo = D;
    tablaR[7][28].valor = 12;
    
    tablaR[8][5].tipo = R;
    tablaR[8][5].valor = 3;
    
    tablaR[9][5].tipo = R;
    tablaR[9][5].valor = 2;
    
    tablaR[10][1].tipo = R;
    tablaR[10][1].valor = 10;
    
    tablaR[10][2].tipo = R;
    tablaR[10][2].valor = 10;
    
    tablaR[10][4].tipo = R;
    tablaR[10][4].valor = 10;
    
    tablaR[10][5].tipo = R;
    tablaR[10][5].valor = 10;
    
    tablaR[10][10].tipo = R;
    tablaR[10][10].valor = 10;
    
    tablaR[10][11].tipo = R;
    tablaR[10][11].valor = 10;
    
    tablaR[10][12].tipo = R;
    tablaR[10][12].valor = 10;
    
    tablaR[10][15].tipo = R;
    tablaR[10][15].valor = 10;
    
    tablaR[10][16].tipo = R;
    tablaR[10][16].valor = 10;
    
    tablaR[10][18].tipo = D;
    tablaR[10][18].valor = 9;
    
    tablaR[10][20].tipo = D;
    tablaR[10][20].valor = 8;
    
    tablaR[10][22].tipo = D;
    tablaR[10][22].valor = 13;
    
    tablaR[10][27].tipo = D;
    tablaR[10][27].valor = 10;
    
    tablaR[10][28].tipo = D;
    tablaR[10][28].valor = 14;
    
    tablaR[11][12].tipo = D;
    tablaR[11][12].valor = 15;
    
    tablaR[12][1].tipo = D;
    tablaR[12][1].valor = 18;
    
    tablaR[12][2].tipo = D;
    tablaR[12][2].valor = 20;
    
    tablaR[12][4].tipo = D;
    tablaR[12][4].valor = 16;
    
    tablaR[12][5].tipo = D;
    tablaR[12][5].valor = 22;
    
    tablaR[12][10].tipo = D;
    tablaR[12][10].valor = 17;
    
    tablaR[12][11].tipo = D;
    tablaR[12][11].valor = 21;
    
    tablaR[12][12].tipo = R;
    tablaR[12][12].valor = 12;
    
    tablaR[12][15].tipo = D;
    tablaR[12][15].valor = 19;
    
    tablaR[12][16].tipo = D;
    tablaR[12][16].valor = 23;
    
    tablaR[12][29].tipo = D;
    tablaR[12][29].valor = 32;
    
    tablaR[12][30].tipo = D;
    tablaR[12][30].valor = 33;
    
    tablaR[12][31].tipo = D;
    tablaR[12][31].valor = 28;
    
    tablaR[12][32].tipo = D;
    tablaR[12][32].valor = 31;
    
    tablaR[12][33].tipo = D;
    tablaR[12][33].valor = 30;
    
    tablaR[12][34].tipo = D;
    tablaR[12][34].valor = 24;
    
    tablaR[12][35].tipo = D;
    tablaR[12][35].valor = 25;
    
    tablaR[12][37].tipo = D;
    tablaR[12][37].valor = 29;
    
    tablaR[12][38].tipo = D;
    tablaR[12][38].valor = 27;
    
    tablaR[12][39].tipo = D;
    tablaR[12][39].valor = 26;
    
    tablaR[13][5].tipo = D;
    tablaR[13][5].valor = 34;
    
    tablaR[13][23].tipo = D;
    tablaR[13][23].valor = 36;
    
    tablaR[13][24].tipo = D;
    tablaR[13][24].valor = 35;
    
    tablaR[14][1].tipo = R;
    tablaR[14][1].valor = 9;
    
    tablaR[14][2].tipo = R;
    tablaR[14][2].valor = 9;
    
    tablaR[14][4].tipo = R;
    tablaR[14][4].valor = 9;
    
    tablaR[14][5].tipo = R;
    tablaR[14][5].valor = 9;
    
    tablaR[14][10].tipo = R;
    tablaR[14][10].valor = 9;
    
    tablaR[14][11].tipo = R;
    tablaR[14][11].valor = 9;
    
    tablaR[14][12].tipo = R;
    tablaR[14][12].valor = 9;
    
    tablaR[14][15].tipo = R;
    tablaR[14][15].valor = 9;
    
    tablaR[14][16].tipo = R;
    tablaR[14][16].valor = 9;
    
    tablaR[15][0].tipo = R;
    tablaR[15][0].valor = 7;
    
    tablaR[16][1].tipo = D;
    tablaR[16][1].valor = 18;
    
    tablaR[16][2].tipo = D;
    tablaR[16][2].valor = 20;
    
    tablaR[16][4].tipo = D;
    tablaR[16][4].valor = 16;
    
    tablaR[16][5].tipo = D;
    tablaR[16][5].valor = 22;
    
    tablaR[16][34].tipo = D;
    tablaR[16][34].valor = 24;
    
    tablaR[16][35].tipo = D;
    tablaR[16][35].valor = 25;
    
    tablaR[16][37].tipo = D;
    tablaR[16][37].valor = 29;
    
    tablaR[16][38].tipo = D;
    tablaR[16][38].valor = 37;
    
    tablaR[16][39].tipo = D;
    tablaR[16][39].valor = 26;
    
    tablaR[17][1].tipo = R;
    tablaR[17][1].valor = 17;
    
    tablaR[17][2].tipo = R;
    tablaR[17][2].valor = 17;
    
    tablaR[17][4].tipo = R;
    tablaR[17][4].valor = 17;
    
    tablaR[17][5].tipo = R;
    tablaR[17][5].valor = 17;
    
    tablaR[17][10].tipo = R;
    tablaR[17][10].valor = 17;
    
    tablaR[17][11].tipo = R;
    tablaR[17][11].valor = 17;
    
    tablaR[17][12].tipo = R;
    tablaR[17][12].valor = 17;
    
    tablaR[17][15].tipo = R;
    tablaR[17][15].valor = 17;
    
    tablaR[17][16].tipo = R;
    tablaR[17][16].valor = 17;
    
    tablaR[18][3].tipo = R;
    tablaR[18][3].valor = 35;
    
    tablaR[18][6].tipo = R;
    tablaR[18][6].valor = 35;
    
    tablaR[18][7].tipo = R;
    tablaR[18][7].valor = 35;
    
    tablaR[18][8].tipo = R;
    tablaR[18][8].valor = 35;
    
    tablaR[18][10].tipo = R;
    tablaR[18][10].valor = 35;
    
    tablaR[19][4].tipo = D;
    tablaR[19][4].valor = 38;
    
    tablaR[20][3].tipo = R;
    tablaR[20][3].valor = 34;
    
    tablaR[20][6].tipo = R;
    tablaR[20][6].valor = 34;
    
    tablaR[20][7].tipo = R;
    tablaR[20][7].valor = 34;
    
    tablaR[20][8].tipo = R;
    tablaR[20][8].valor = 34;
    
    tablaR[20][10].tipo = R;
    tablaR[20][10].valor = 34;
    
    tablaR[21][1].tipo = D;
    tablaR[21][1].valor = 18;
    
    tablaR[21][2].tipo = D;
    tablaR[21][2].valor = 20;
    
    tablaR[21][4].tipo = D;
    tablaR[21][4].valor = 16;
    
    tablaR[21][5].tipo = D;
    tablaR[21][5].valor = 22;
    
    tablaR[21][10].tipo = D;
    tablaR[21][10].valor = 39;
    
    tablaR[21][34].tipo = D;
    tablaR[21][34].valor = 24;
    
    tablaR[21][35].tipo = D;
    tablaR[21][35].valor = 25;
    
    tablaR[21][37].tipo = D;
    tablaR[21][37].valor = 29;
    
    tablaR[21][38].tipo = D;
    tablaR[21][38].valor = 40;
    
    tablaR[21][39].tipo = D;
    tablaR[21][39].valor = 26;
    
    tablaR[22][3].tipo = R;
    tablaR[22][3].valor = 31;
    
    tablaR[22][6].tipo = R;
    tablaR[22][6].valor = 31;
    
    tablaR[22][7].tipo = R;
    tablaR[22][7].valor = 31;
    
    tablaR[22][8].tipo = R;
    tablaR[22][8].valor = 31;
    
    tablaR[22][9].tipo = D;
    tablaR[22][9].valor = 41;
    
    tablaR[22][10].tipo = R;
    tablaR[22][10].valor = 31;
    
    tablaR[23][4].tipo = D;
    tablaR[23][4].valor = 42;
    
    tablaR[24][3].tipo = R;
    tablaR[24][3].valor = 23;
    
    tablaR[24][10].tipo = R;
    tablaR[24][10].valor = 23;
    
    tablaR[25][3].tipo = R;
    tablaR[25][3].valor = 25;
    
    tablaR[25][6].tipo = D;
    tablaR[25][6].valor = 43;
    
    tablaR[25][7].tipo = D;
    tablaR[25][7].valor = 45;
    
    tablaR[25][8].tipo = D;
    tablaR[25][8].valor = 44;
    
    tablaR[25][10].tipo = R;
    tablaR[25][10].valor = 25;
    
    tablaR[25][36].tipo = D;
    tablaR[25][36].valor = 46;
    
    tablaR[26][3].tipo = R;
    tablaR[26][3].valor = 32;
    
    tablaR[26][6].tipo = R;
    tablaR[26][6].valor = 32;
    
    tablaR[26][7].tipo = R;
    tablaR[26][7].valor = 32;
    
    tablaR[26][8].tipo = R;
    tablaR[26][8].valor = 32;
    
    tablaR[26][10].tipo = R;
    tablaR[26][10].valor = 32;
    
    tablaR[27][10].tipo = D;
    tablaR[27][10].valor = 47;
    
    tablaR[28][1].tipo = R;
    tablaR[28][1].valor = 13;
    
    tablaR[28][2].tipo = R;
    tablaR[28][2].valor = 13;
    
    tablaR[28][4].tipo = R;
    tablaR[28][4].valor = 13;
    
    tablaR[28][5].tipo = R;
    tablaR[28][5].valor = 13;
    
    tablaR[28][10].tipo = R;
    tablaR[28][10].valor = 13;
    
    tablaR[28][11].tipo = R;
    tablaR[28][11].valor = 13;
    
    tablaR[28][12].tipo = R;
    tablaR[28][12].valor = 13;
    
    tablaR[28][15].tipo = R;
    tablaR[28][15].valor = 13;
    
    tablaR[28][16].tipo = R;
    tablaR[28][16].valor = 13;
    
    tablaR[29][3].tipo = R;
    tablaR[29][3].valor = 27;
    
    tablaR[29][6].tipo = R;
    tablaR[29][6].valor = 27;
    
    tablaR[29][7].tipo = R;
    tablaR[29][7].valor = 27;
    
    tablaR[29][8].tipo = R;
    tablaR[29][8].valor = 27;
    
    tablaR[29][10].tipo = R;
    tablaR[29][10].valor = 27;
    
    tablaR[30][1].tipo = R;
    tablaR[30][1].valor = 16;
    
    tablaR[30][2].tipo = R;
    tablaR[30][2].valor = 16;
    
    tablaR[30][4].tipo = R;
    tablaR[30][4].valor = 16;
    
    tablaR[30][5].tipo = R;
    tablaR[30][5].valor = 16;
    
    tablaR[30][10].tipo = R;
    tablaR[30][10].valor = 16;
    
    tablaR[30][11].tipo = R;
    tablaR[30][11].valor = 16;
    
    tablaR[30][12].tipo = R;
    tablaR[30][12].valor = 16;
    
    tablaR[30][15].tipo = R;
    tablaR[30][15].valor = 16;
    
    tablaR[30][16].tipo = R;
    tablaR[30][16].valor = 16;
    
    tablaR[31][1].tipo = D;
    tablaR[31][1].valor = 18;
    
    tablaR[31][2].tipo = D;
    tablaR[31][2].valor = 20;
    
    tablaR[31][4].tipo = D;
    tablaR[31][4].valor = 16;
    
    tablaR[31][5].tipo = D;
    tablaR[31][5].valor = 22;
    
    tablaR[31][10].tipo = D;
    tablaR[31][10].valor = 17;
    
    tablaR[31][11].tipo = D;
    tablaR[31][11].valor = 21;
    
    tablaR[31][12].tipo = R;
    tablaR[31][12].valor = 12;
    
    tablaR[31][15].tipo = D;
    tablaR[31][15].valor = 19;
    
    tablaR[31][16].tipo = D;
    tablaR[31][16].valor = 23;
    
    tablaR[31][29].tipo = D;
    tablaR[31][29].valor = 48;
    
    tablaR[31][30].tipo = D;
    tablaR[31][30].valor = 33;
    
    tablaR[31][31].tipo = D;
    tablaR[31][31].valor = 28;
    
    tablaR[31][32].tipo = D;
    tablaR[31][32].valor = 31;
    
    tablaR[31][33].tipo = D;
    tablaR[31][33].valor = 30;
    
    tablaR[31][34].tipo = D;
    tablaR[31][34].valor = 24;
    
    tablaR[31][35].tipo = D;
    tablaR[31][35].valor = 25;
    
    tablaR[31][37].tipo = D;
    tablaR[31][37].valor = 29;
    
    tablaR[31][38].tipo = D;
    tablaR[31][38].valor = 27;
    
    tablaR[31][39].tipo = D;
    tablaR[31][39].valor = 26;
    
    tablaR[32][12].tipo = R;
    tablaR[32][12].valor = 8;
    
    tablaR[33][1].tipo = R;
    tablaR[33][1].valor = 15;
    
    tablaR[33][2].tipo = R;
    tablaR[33][2].valor = 15;
    
    tablaR[33][4].tipo = R;
    tablaR[33][4].valor = 15;
    
    tablaR[33][5].tipo = R;
    tablaR[33][5].valor = 15;
    
    tablaR[33][10].tipo = R;
    tablaR[33][10].valor = 15;
    
    tablaR[33][11].tipo = R;
    tablaR[33][11].valor = 15;
    
    tablaR[33][12].tipo = R;
    tablaR[33][12].valor = 15;
    
    tablaR[33][15].tipo = R;
    tablaR[33][15].valor = 15;
    
    tablaR[33][16].tipo = R;
    tablaR[33][16].valor = 15;
    
    tablaR[34][10].tipo = R;
    tablaR[34][10].valor = 6;
    
    tablaR[34][19].tipo = R;
    tablaR[34][19].valor = 6;
    
    tablaR[35][10].tipo = R;
    tablaR[35][10].valor = 5;
    
    tablaR[35][19].tipo = R;
    tablaR[35][19].valor = 5;
    
    tablaR[36][10].tipo = D;
    tablaR[36][10].valor = 50;
    
    tablaR[36][19].tipo = D;
    tablaR[36][19].valor = 49;
    
    tablaR[37][3].tipo = D;
    tablaR[37][3].valor = 51;
    
    tablaR[38][1].tipo = D;
    tablaR[38][1].valor = 18;
    
    tablaR[38][2].tipo = D;
    tablaR[38][2].valor = 20;
    
    tablaR[38][4].tipo = D;
    tablaR[38][4].valor = 16;
    
    tablaR[38][5].tipo = D;
    tablaR[38][5].valor = 22;
    
    tablaR[38][34].tipo = D;
    tablaR[38][34].valor = 24;
    
    tablaR[38][35].tipo = D;
    tablaR[38][35].valor = 25;
    
    tablaR[38][37].tipo = D;
    tablaR[38][37].valor = 29;
    
    tablaR[38][38].tipo = D;
    tablaR[38][38].valor = 52;
    
    tablaR[38][39].tipo = D;
    tablaR[38][39].valor = 26;
    
    tablaR[39][1].tipo = R;
    tablaR[39][1].valor = 22;
    
    tablaR[39][2].tipo = R;
    tablaR[39][2].valor = 22;
    
    tablaR[39][4].tipo = R;
    tablaR[39][4].valor = 22;
    
    tablaR[39][5].tipo = R;
    tablaR[39][5].valor = 22;
    
    tablaR[39][10].tipo = R;
    tablaR[39][10].valor = 22;
    
    tablaR[39][11].tipo = R;
    tablaR[39][11].valor = 22;
    
    tablaR[39][12].tipo = R;
    tablaR[39][12].valor = 22;
    
    tablaR[39][15].tipo = R;
    tablaR[39][15].valor = 22;
    
    tablaR[39][16].tipo = R;
    tablaR[39][16].valor = 22;
    
    tablaR[40][10].tipo = D;
    tablaR[40][10].valor = 53;
    
    tablaR[41][1].tipo = D;
    tablaR[41][1].valor = 18;
    
    tablaR[41][2].tipo = D;
    tablaR[41][2].valor = 20;
    
    tablaR[41][4].tipo = D;
    tablaR[41][4].valor = 16;
    
    tablaR[41][5].tipo = D;
    tablaR[41][5].valor = 54;
    
    tablaR[41][35].tipo = D;
    tablaR[41][35].valor = 55;
    
    tablaR[41][37].tipo = D;
    tablaR[41][37].valor = 29;
    
    tablaR[41][39].tipo = D;
    tablaR[41][39].valor = 26;
    
    tablaR[42][1].tipo = D;
    tablaR[42][1].valor = 18;
    
    tablaR[42][2].tipo = D;
    tablaR[42][2].valor = 20;
    
    tablaR[42][4].tipo = D;
    tablaR[42][4].valor = 16;
    
    tablaR[42][5].tipo = D;
    tablaR[42][5].valor = 22;
    
    tablaR[42][34].tipo = D;
    tablaR[42][34].valor = 24;
    
    tablaR[42][35].tipo = D;
    tablaR[42][35].valor = 25;
    
    tablaR[42][37].tipo = D;
    tablaR[42][37].valor = 29;
    
    tablaR[42][38].tipo = D;
    tablaR[42][38].valor = 56;
    
    tablaR[42][39].tipo = D;
    tablaR[42][39].valor = 26;
    
    tablaR[43][1].tipo = R;
    tablaR[43][1].valor = 30;
    
    tablaR[43][2].tipo = R;
    tablaR[43][2].valor = 30;
    
    tablaR[43][4].tipo = R;
    tablaR[43][4].valor = 30;
    
    tablaR[43][5].tipo = R;
    tablaR[43][5].valor = 30;
    
    tablaR[44][1].tipo = R;
    tablaR[44][1].valor = 28;
    
    tablaR[44][2].tipo = R;
    tablaR[44][2].valor = 28;
    
    tablaR[44][4].tipo = R;
    tablaR[44][4].valor = 28;
    
    tablaR[44][5].tipo = R;
    tablaR[44][5].valor = 28;
    
    tablaR[45][1].tipo = R;
    tablaR[45][1].valor = 29;
    
    tablaR[45][2].tipo = R;
    tablaR[45][2].valor = 29;
    
    tablaR[45][4].tipo = R;
    tablaR[45][4].valor = 29;
    
    tablaR[45][5].tipo = R;
    tablaR[45][5].valor = 29;
    
    tablaR[46][1].tipo = D;
    tablaR[46][1].valor = 18;
    
    tablaR[46][2].tipo = D;
    tablaR[46][2].valor = 20;
    
    tablaR[46][4].tipo = D;
    tablaR[46][4].valor = 16;
    
    tablaR[46][5].tipo = D;
    tablaR[46][5].valor = 54;
    
    tablaR[46][37].tipo = D;
    tablaR[46][37].valor = 57;
    
    tablaR[46][39].tipo = D;
    tablaR[46][39].valor = 26;
    
    tablaR[47][1].tipo = R;
    tablaR[47][1].valor = 14;
    
    tablaR[47][2].tipo = R;
    tablaR[47][2].valor = 14;
    
    tablaR[47][4].tipo = R;
    tablaR[47][4].valor = 14;
    
    tablaR[47][5].tipo = R;
    tablaR[47][5].valor = 14;
    
    tablaR[47][10].tipo = R;
    tablaR[47][10].valor = 14;
    
    tablaR[47][11].tipo = R;
    tablaR[47][11].valor = 14;
    
    tablaR[47][12].tipo = R;
    tablaR[47][12].valor = 14;
    
    tablaR[47][15].tipo = R;
    tablaR[47][15].valor = 14;
    
    tablaR[47][16].tipo = R;
    tablaR[47][16].valor = 14;
    
    tablaR[48][12].tipo = R;
    tablaR[48][12].valor = 11;
    
    tablaR[49][5].tipo = D;
    tablaR[49][5].valor = 34;
    
    tablaR[49][24].tipo = D;
    tablaR[49][24].valor = 58;
    
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
    
    tablaR[51][3].tipo = R;
    tablaR[51][3].valor = 33;
    
    tablaR[51][6].tipo = R;
    tablaR[51][6].valor = 33;
    
    tablaR[51][7].tipo = R;
    tablaR[51][7].valor = 33;
    
    tablaR[51][8].tipo = R;
    tablaR[51][8].valor = 33;
    
    tablaR[51][10].tipo = R;
    tablaR[51][10].valor = 33;
    
    tablaR[52][3].tipo = D;
    tablaR[52][3].valor = 59;
    
    tablaR[53][1].tipo = R;
    tablaR[53][1].valor = 21;
    
    tablaR[53][2].tipo = R;
    tablaR[53][2].valor = 21;
    
    tablaR[53][4].tipo = R;
    tablaR[53][4].valor = 21;
    
    tablaR[53][5].tipo = R;
    tablaR[53][5].valor = 21;
    
    tablaR[53][10].tipo = R;
    tablaR[53][10].valor = 21;
    
    tablaR[53][11].tipo = R;
    tablaR[53][11].valor = 21;
    
    tablaR[53][12].tipo = R;
    tablaR[53][12].valor = 21;
    
    tablaR[53][15].tipo = R;
    tablaR[53][15].valor = 21;
    
    tablaR[53][16].tipo = R;
    tablaR[53][16].valor = 21;
    
    tablaR[54][3].tipo = R;
    tablaR[54][3].valor = 31;
    
    tablaR[54][6].tipo = R;
    tablaR[54][6].valor = 31;
    
    tablaR[54][7].tipo = R;
    tablaR[54][7].valor = 31;
    
    tablaR[54][8].tipo = R;
    tablaR[54][8].valor = 31;
    
    tablaR[54][10].tipo = R;
    tablaR[54][10].valor = 31;
    
    tablaR[55][3].tipo = R;
    tablaR[55][3].valor = 24;
    
    tablaR[55][6].tipo = D;
    tablaR[55][6].valor = 43;
    
    tablaR[55][7].tipo = D;
    tablaR[55][7].valor = 45;
    
    tablaR[55][8].tipo = D;
    tablaR[55][8].valor = 44;
    
    tablaR[55][10].tipo = R;
    tablaR[55][10].valor = 24;
    
    tablaR[55][36].tipo = D;
    tablaR[55][36].valor = 46;
    
    tablaR[56][3].tipo = D;
    tablaR[56][3].valor = 60;
    
    tablaR[57][3].tipo = R;
    tablaR[57][3].valor = 26;
    
    tablaR[57][6].tipo = R;
    tablaR[57][6].valor = 26;
    
    tablaR[57][7].tipo = R;
    tablaR[57][7].valor = 26;
    
    tablaR[57][8].tipo = R;
    tablaR[57][8].valor = 26;
    
    tablaR[57][10].tipo = R;
    tablaR[57][10].valor = 26;
    
    tablaR[58][10].tipo = R;
    tablaR[58][10].valor = 4;
    
    tablaR[58][19].tipo = R;
    tablaR[58][19].valor = 4;
    
    tablaR[59][13].tipo = D;
    tablaR[59][13].valor = 61;
    
    tablaR[60][13].tipo = D;
    tablaR[60][13].valor = 62;
    
    tablaR[61][1].tipo = D;
    tablaR[61][1].valor = 18;
    
    tablaR[61][2].tipo = D;
    tablaR[61][2].valor = 20;
    
    tablaR[61][4].tipo = D;
    tablaR[61][4].valor = 16;
    
    tablaR[61][5].tipo = D;
    tablaR[61][5].valor = 22;
    
    tablaR[61][10].tipo = D;
    tablaR[61][10].valor = 17;
    
    tablaR[61][11].tipo = D;
    tablaR[61][11].valor = 21;
    
    tablaR[61][15].tipo = D;
    tablaR[61][15].valor = 19;
    
    tablaR[61][16].tipo = D;
    tablaR[61][16].valor = 23;
    
    tablaR[61][30].tipo = D;
    tablaR[61][30].valor = 33;
    
    tablaR[61][31].tipo = D;
    tablaR[61][31].valor = 28;
    
    tablaR[61][32].tipo = D;
    tablaR[61][32].valor = 63;
    
    tablaR[61][33].tipo = D;
    tablaR[61][33].valor = 30;
    
    tablaR[61][34].tipo = D;
    tablaR[61][34].valor = 24;
    
    tablaR[61][35].tipo = D;
    tablaR[61][35].valor = 25;
    
    tablaR[61][37].tipo = D;
    tablaR[61][37].valor = 29;
    
    tablaR[61][38].tipo = D;
    tablaR[61][38].valor = 27;
    
    tablaR[61][39].tipo = D;
    tablaR[61][39].valor = 26;
    
    tablaR[62][1].tipo = D;
    tablaR[62][1].valor = 18;
    
    tablaR[62][2].tipo = D;
    tablaR[62][2].valor = 20;
    
    tablaR[62][4].tipo = D;
    tablaR[62][4].valor = 16;
    
    tablaR[62][5].tipo = D;
    tablaR[62][5].valor = 22;
    
    tablaR[62][10].tipo = D;
    tablaR[62][10].valor = 17;
    
    tablaR[62][11].tipo = D;
    tablaR[62][11].valor = 21;
    
    tablaR[62][15].tipo = D;
    tablaR[62][15].valor = 19;
    
    tablaR[62][16].tipo = D;
    tablaR[62][16].valor = 23;
    
    tablaR[62][30].tipo = D;
    tablaR[62][30].valor = 33;
    
    tablaR[62][31].tipo = D;
    tablaR[62][31].valor = 28;
    
    tablaR[62][32].tipo = D;
    tablaR[62][32].valor = 64;
    
    tablaR[62][33].tipo = D;
    tablaR[62][33].valor = 30;
    
    tablaR[62][34].tipo = D;
    tablaR[62][34].valor = 24;
    
    tablaR[62][35].tipo = D;
    tablaR[62][35].valor = 25;
    
    tablaR[62][37].tipo = D;
    tablaR[62][37].valor = 29;
    
    tablaR[62][38].tipo = D;
    tablaR[62][38].valor = 27;
    
    tablaR[62][39].tipo = D;
    tablaR[62][39].valor = 26;
    
    tablaR[63][12].tipo = D;
    tablaR[63][12].valor = 65;
    
    tablaR[64][12].tipo = D;
    tablaR[64][12].valor = 66;
    
    tablaR[65][1].tipo = R;
    tablaR[65][1].valor = 19;
    
    tablaR[65][2].tipo = R;
    tablaR[65][2].valor = 19;
    
    tablaR[65][4].tipo = R;
    tablaR[65][4].valor = 19;
    
    tablaR[65][5].tipo = R;
    tablaR[65][5].valor = 19;
    
    tablaR[65][10].tipo = R;
    tablaR[65][10].valor = 19;
    
    tablaR[65][11].tipo = R;
    tablaR[65][11].valor = 19;
    
    tablaR[65][12].tipo = R;
    tablaR[65][12].valor = 19;
    
    tablaR[65][14].tipo = D;
    tablaR[65][14].valor = 67;
    
    tablaR[65][15].tipo = R;
    tablaR[65][15].valor = 19;
    
    tablaR[65][16].tipo = R;
    tablaR[65][16].valor = 19;
    
    tablaR[66][1].tipo = R;
    tablaR[66][1].valor = 18;
    
    tablaR[66][2].tipo = R;
    tablaR[66][2].valor = 18;
    
    tablaR[66][4].tipo = R;
    tablaR[66][4].valor = 18;
    
    tablaR[66][5].tipo = R;
    tablaR[66][5].valor = 18;
    
    tablaR[66][10].tipo = R;
    tablaR[66][10].valor = 18;
    
    tablaR[66][11].tipo = R;
    tablaR[66][11].valor = 18;
    
    tablaR[66][12].tipo = R;
    tablaR[66][12].valor = 18;
    
    tablaR[66][15].tipo = R;
    tablaR[66][15].valor = 18;
    
    tablaR[66][16].tipo = R;
    tablaR[66][16].valor = 18;
    
    tablaR[67][13].tipo = D;
    tablaR[67][13].valor = 68;
    
    tablaR[68][1].tipo = D;
    tablaR[68][1].valor = 18;
    
    tablaR[68][2].tipo = D;
    tablaR[68][2].valor = 20;
    
    tablaR[68][4].tipo = D;
    tablaR[68][4].valor = 16;
    
    tablaR[68][5].tipo = D;
    tablaR[68][5].valor = 22;
    
    tablaR[68][10].tipo = D;
    tablaR[68][10].valor = 17;
    
    tablaR[68][11].tipo = D;
    tablaR[68][11].valor = 21;
    
    tablaR[68][15].tipo = D;
    tablaR[68][15].valor = 19;
    
    tablaR[68][16].tipo = D;
    tablaR[68][16].valor = 23;
    
    tablaR[68][30].tipo = D;
    tablaR[68][30].valor = 33;
    
    tablaR[68][31].tipo = D;
    tablaR[68][31].valor = 28;
    
    tablaR[68][32].tipo = D;
    tablaR[68][32].valor = 69;
    
    tablaR[68][33].tipo = D;
    tablaR[68][33].valor = 30;
    
    tablaR[68][34].tipo = D;
    tablaR[68][34].valor = 24;
    
    tablaR[68][35].tipo = D;
    tablaR[68][35].valor = 25;
    
    tablaR[68][37].tipo = D;
    tablaR[68][37].valor = 29;
    
    tablaR[68][38].tipo = D;
    tablaR[68][38].valor = 27;
    
    tablaR[68][39].tipo = D;
    tablaR[68][39].valor = 26;
    
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
