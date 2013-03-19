/*
 *  analex.h
 *  Analizador Lexico
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

#define cuantosAutomatas 11
#define eq(str1, str2) (strcmp(str1, str2) == 0)

int inputSizeLex = 0;

// Declaracion de variables globales
int fdLexemas;
int fdTabla;
FILE * fuente;
char escritura[BUFSIZ];

char **inputLex;
char **inputRealLex;

// Arreglo de palabras reservadas (keywords)
const char *keywords[5] = {"else","return","if","while","main"};

// Arreglo de tipos de datos
const char *dataType[7] = {"int","float","double","short","long","unsigned",
    "signed"};

int isComment,isString,isComma,isSemiColon;

/* Estructura automata, contiene todo lo necesario para cada automata
 * buffer = buffer donde se escribe el lexema
 * estado = estado en el que se encuentra actualmente el automata
 * contiene un apuntador a funcion que recibe la estructura y el caracter que va a leer
 **/
typedef struct automata {
	char buffer[BUFSIZ];
	int estado;
	int (*funcion)(struct automata *a,char c);
}automata;


/**
 * Reinicia todos los automatas, pone su estado en 0 y limpia el buffer
 * Pone las variable de comentario y cadena de caracteres en 0
 *
 * @param automata[]
 * @return void
 **/
void reset(automata arr[]){
    int i;
    for(i = 0; i < cuantosAutomatas; i++){
        arr[i].estado = 0;
        memset(arr[i].buffer, '\0', BUFSIZ);
    }
	isComment = 0;
	isString = 0;
    isComma = 0;
    isSemiColon = 0;
}

void resetInp(){
	int i;
	inputRealLex = (char **)malloc(sizeof(char)*1024);
	inputLex = (char **)malloc(sizeof(char)*1024);
    for(i = 0; i < 1024; i++){
        inputRealLex[i] = (char *)malloc(BUFSIZ);
		inputLex[i] = (char *)malloc(BUFSIZ);
    }
}

void freeInp(){
	// Da seg fault
	memset(inputRealLex, 0, BUFSIZ);
	memset(inputLex, 0, BUFSIZ);
	free(inputRealLex);
	free(inputLex);
}

void printInput(){
	int i;
	fprintf(stdout, "InputSize:%d\n",inputSizeLex);
	for (i=0; i<=inputSizeLex; i++) {
		fprintf(stdout, "InputLex[%d]:%s\n",i,inputLex[i]);
	}
}

/**
 * Regresa la ultima posicion donde se encuentra el buffer dado
 *
 * @param char[]
 * @return int
 **/
int getLastIndex(char buffer[]) {
	int i = 0;
	while(buffer[i] != '\0') {
		i++;
		if (i >= (BUFSIZ)) {
			break;
		}
	}
	return i;
}

/**
 * Checa si la cadena de caracteres mostrada es un keyword
 *
 * @param string
 * @return int
 **/
int isKeyword(char str[]) {
	int size,i;
	size = sizeof(keywords)/sizeof(keywords[1]);
	for (i=0; i<size; i++) {
		if (strcmp(str, keywords[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Checa si la cadena de caracteres mostrada es un tipo de dato
 *
 * @param string
 * @return int
 **/
int isDataType(char str[]) {
	int size,i;
	size = sizeof(dataType)/sizeof(dataType[1]);
	for (i=0; i<size; i++) {
		if (strcmp(str, dataType[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Escribe en el archivo de texto lexema.txt
 *
 * @param char*
 * @param char*
 * @return void
 **/
void lexema(char *l, char *t){
    // Limpia buffer
	memset(escritura, '\0', BUFSIZ);
	// Escribo buffer
    sprintf(escritura,"%s\t%s\n", l, t);
    write(fdLexemas, escritura, BUFSIZ);
}

/**
 * Escribe en el archivo de texto tabla_simbolos.txt
 *
 * @param char*
 * @return void
 **/
void tabla(char *l){
    // Limpia buffer
	memset(escritura, '\0', BUFSIZ);
	// Escribo buffer
    sprintf(escritura,"%s\n", l);
    write(fdTabla, escritura, BUFSIZ);
}


/****************************************************
 *													*
 *					Automatas						*
 *													*
 ***************************************************/

/**
 * Automata para numeros reales
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataNumerosReales(automata *a , char c) {
	if (isComment == 1 || isString == 1) {(*a).estado = -1;}
	//fprintf(stdout, "reales estado: %d",(*a).estado);
	//fprintf(stdout, "Realesestado:%d char:%c\n",(*a).estado,c);
	switch ((*a).estado) {
		case 0:
			if (c == '+' || c == '-') {
				// Imprime en buffer
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				// Automatas regresan 0 si no es estado final
				return 0;
			}
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (c == '.') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 3;
				return 0;
			}
			return 0;
			break;
		case 1:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			}
			if (c == '.') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 3;
				return 0;
			}
			return 0;
			break;
		case 2:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			
			return 0;
			break;
		case 3:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 4;
				return 0;
			}
			return 0;
			break;
		case 4:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			} else {
                // Numero Real se convierte a "float_literal"
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"float_literal");
				lexema((*a).buffer, "float_literal");
                
                strcpy(inputLex[inputSizeLex],"float_literal");
				strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				// Como leo un caracter despues y no es digito, regreso el apuntador
				fseek(fuente, -1, SEEK_CUR);
				return 1;
			}
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para numeros naturales
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataNumeros(automata *a , char c) {
	if (isComment == 1 || isString == 1) {(*a).estado = -1;}
	//fprintf(stdout, "Numerosestado:%d char:%c\n",(*a).estado,c);
	switch ((*a).estado) {
		case 0:
			if (c == '+' || c == '-') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			return 0;
			break;
		case 1:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			// Si el siguiente valor no es digito entonces ya no es un num. natural
			(*a).estado = -1;
            
			return 0;
			break;
		case 2:
			if (isdigit(c)) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			} else if (c != '.'){
                // Numero Natural se convierte a "int_literal"
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"int_literal");
				lexema((*a).buffer, "int_literal");
                
                strcpy(inputLex[inputSizeLex],"int_literal");
				strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				fseek(fuente, -1, SEEK_CUR);
				return 1;
			} else {
				// Posiblemente un real
				(*a).estado = -1;
				return 0;
			}
            
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para identificadores
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataIdentificadores(automata *a , char c) {
	//fprintf(stdout, "Idestado:%d char:%c\n",(*a).estado,c);
	if (isComment == 1 || isString == 1) {(*a).estado = -1;}
	switch ((*a).estado) {
		case 0:
			if (isalpha(c) || c == '_') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (c == ' ' || c == '\n') {
				return 0;
			}
			// Si comienza con algo mas
			(*a).estado = -1;
			return 0;
			break;
		case 1:
			if (isdigit(c) || isalpha(c) || c == '_') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			} else {
				if (isKeyword((*a).buffer)) {
                    
                    // Checo las 5 keywords porque cada una es diferente
                    if (eq((*a).buffer, "else")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"else");
                        lexema((*a).buffer, "else");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    }else if (eq((*a).buffer, "return")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"return");
                        lexema((*a).buffer, "return");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                        
                    } else if (eq((*a).buffer, "void")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"void");
                        lexema((*a).buffer, "void");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    } else if (eq((*a).buffer, "if")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"if");
                        lexema((*a).buffer, "if");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    } else if (eq((*a).buffer, "while")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"while");
                        lexema((*a).buffer, "while");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    } else if (eq((*a).buffer, "main")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"main");
                        lexema((*a).buffer, "main");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    }
                
				} else if (isDataType((*a).buffer)) {
                    // Checa palabras especiales
                    
                    if (eq((*a).buffer, "int")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"int");
                        lexema((*a).buffer, "int");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    }else if (eq((*a).buffer, "float")) {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"float");
                        lexema((*a).buffer, "float");
                        
                        strcpy(inputLex[inputSizeLex],(*a).buffer);
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                        
                    } else {
                        fprintf(stdout, "%s\t%s\n",(*a).buffer,"var_type");
                        lexema((*a).buffer, "var_type");
                        
                        strcpy(inputLex[inputSizeLex],"var_type");
                        strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                    }
                    
				} else {
                    // No es ni tipo de dato ni palabra reservada, entonces es un identificador
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"var_name");
                    lexema((*a).buffer, "var_name");
                
                    strcpy(inputLex[inputSizeLex],"var_name");
                    strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				}
				tabla((*a).buffer);
                
				inputSizeLex++;
                
				fseek(fuente, -1, SEEK_CUR);
				return 1;
			}
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para comentarios
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataComentarios(automata *a, char c){
	//fprintf(stdout, "Comentestado:%d char:%c\n",(*a).estado,c);
	switch((*a).estado){
        case 0:
            if(c == '/'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
                (*a).estado = 1;
                return 0;
            }
        	return 0;
        	break;
    	case 1:
			if(c == '/'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
    		 	(*a).estado = 2;
				isComment = 1;
    		 	return 0;
    		}if(c == '*'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
    			(*a).estado = 3;
				isComment = 1;
    			return 0;
    		}
            return 0;
            break;
        case 2:
        	if(((int)c) != 10){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
        	} else {
	      		//Estado final de comentario corto
      		    fprintf(stdout, "%s\t%s\n",(*a).buffer,"Comentario");
				lexema((*a).buffer, "Comentario");
                
                strcpy(inputLex[inputSizeLex],(*a).buffer);
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
      		}
			return 0;
			break;
      	case 3:
      		if(c == '*'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
      			(*a).estado = 4;
      			return 0;
      		} else {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
      			(*a).estado = 3;
      			return 0;
      		}
			return 0;
			break;
      	case 4:
			if(c == '*'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
      			(*a).estado = 4;
      			return 0;
      		}
			if (c == '/') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"Comentario Largo");
				lexema((*a).buffer, "Comentario Largo");
                
                strcpy(inputLex[inputSizeLex],(*a).buffer);
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			} else {
				printf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
      			(*a).estado = 3;
      			return 0;
			}
            
			return 0;
			break;
        default:
			return 0;
			break;
	}
}

/**
 * Automata para operadores de puntuacion (; y ,)
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataPuntuacion(automata *a , char c){
	if (isComment == 1 || isString == 1) {(*a).estado = -1;}
	//fprintf(stdout, "Puntgestado:%d char:%c\n",(*a).estado,c);
    switch((*a).estado){
        case 0:
            if(c == ';'){
                sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado=1;
                isSemiColon = 1;
                return 0;
            }
            if(c == ','){
                sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado=1;
                isComma = 1;
                return 0;
            }
			if (c == ' ' || c == '\n') {
				return 0;
			}
            return 0;
            break;
        case 1:
            if(((int)c)>=48 && ((int)c)<=57){
                //noEsPunto=1;
                return 0;
            }else{
                if (isSemiColon == 1) {
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"semi_colon");
                    //lexema((*a).buffer, "semi_colon");
                    
                    strcpy(inputLex[inputSizeLex],"semi_colon");
                    strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                } else if (isComma == 1) {
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"comma");
                    lexema((*a).buffer, "comma");
                    
                    strcpy(inputLex[inputSizeLex],"comma");
                    strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                }
				
				inputSizeLex++;
                
				fseek(fuente, -1, SEEK_CUR);
				return 1;
            }
            return 0;
            break;
		default:
			return 0;
			break;
    }
	return 0;
}

/**
 * Automata para operadores de asignacion (= += -= *= /= %=)
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataOperadoresAsignacion(automata *a, char c){
	if (isComment == 1) {(*a).estado = -1;}
	//fprintf(stdout, "Asigestado:%d char:%c\n",(*a).estado,c);
    switch((*a).estado){
        /* TODO No tiene que leer += */
		case 0:
			if(c == '*' || c == '/' || c == '%'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if(c == '='){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			if (c == '+') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 3;
				return 0;
			}
			if (c == '-') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 4;
				return 0;
			}
			if (c == ' ' || c == '\n') {
				return 0;
			}
			(*a).estado = -1;
			break;
		case 1:
			if(c == '='){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"equal");
				lexema((*a).buffer, "equal");
                
                strcpy(inputLex[inputSizeLex], "equal");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);

                inputSizeLex++;
				
                return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
		case 2:
			fprintf(stdout, "%s\t%s\n",(*a).buffer,"equal");
			lexema((*a).buffer, "equal");
            
            strcpy(inputLex[inputSizeLex], "equal");
            strcpy(inputRealLex[inputSizeLex],(*a).buffer);
            
            inputSizeLex++;
            
			fseek(fuente, -1, SEEK_CUR);
            
			return 1;
			break;
		case 3:
			if(c == '+'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"Operador de asignacion");
				lexema((*a).buffer, "Operador de asignacion");
				return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
		case 4:
			if(c == '-'){
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"Operador de asignacion");
				lexema((*a).buffer, "Operador de asignacion");
				return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
	}
	return 0;
}

/**
 * Automata para operadores de agrupacion
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataOperadoresAgrupacion(automata *a , char c) {
	if (isComment == 1) {(*a).estado = -1;}
	switch ((*a).estado) {
		case 0:
			if ((c == '(') || (c == ')') || (c == '[') || (c == ']') || (c == '{') || (c == '}')) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
                
                if (eq((*a).buffer, "(")) {
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"left_parenthesis");
                    //lexema((*a).buffer, "left_parenthesis");
                    
                    strcpy(inputLex[inputSizeLex],"left_parenthesis");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                } else if (eq((*a).buffer, ")")){
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"right_parenthesis");
                    //lexema((*a).buffer, "right_parenthesis");
                    
                    strcpy(inputLex[inputSizeLex],"right_parenthesis");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                } else if (eq((*a).buffer, "{")){
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"left_curly_bracket");
                    //lexema((*a).buffer, "left_curly_bracket");
                    
                    strcpy(inputLex[inputSizeLex],"left_curly_bracket");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                } else if (eq((*a).buffer, "}")){
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"right_curly_bracket");
                    //lexema((*a).buffer, "right_curly_bracket");
                    
                    strcpy(inputLex[inputSizeLex],"right_curly_bracket");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                } else if (eq((*a).buffer, "[")){
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"left_bracket");
                    //lexema((*a).buffer, "left_bracket");
                    
                    strcpy(inputLex[inputSizeLex],"left_bracket");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                } else if (eq((*a).buffer, "]")){
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"right_bracket");
                    //lexema((*a).buffer, "right_bracket");
                    
                    strcpy(inputLex[inputSizeLex],"right_bracket");
                    strcpy(inputRealLex[inputSizeLex], (*a).buffer);
                    
                }
                else{
                    fprintf(stdout, "%s\t%s\n",(*a).buffer,"Operador de Agrupacion");
                    //lexema((*a).buffer, "Operador de Agrupacion");
                    strcpy(inputLex[inputSizeLex],(*a).buffer);
                    strcpy(inputRealLex[inputSizeLex],(*a).buffer);
                }
				
				inputSizeLex++;
                
				return 1;
			}
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para operadores aritmeticos
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataAritmetico(automata *a , char c) {
	if (isComment == 1 || isString == 1) {(*a).estado = -1;}
	//fprintf(stdout, "Arigestado:%d char:%c\n",(*a).estado,c);
	switch ((*a).estado) {
		case 0:
			if (((c == '+') || (c == '-') || (c == '/') || (c == '%') || (c == '*')) && isComment == 0) {
				(*a).estado = 1;
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			}
			return 0;
		case 1:
			if ((isdigit(c) || c == ' ' || isalpha(c)) && isComment == 0) {
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"arith_op");
				lexema((*a).buffer, "arith_op");
                
                strcpy(inputLex[inputSizeLex],"arith_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				fseek(fuente, -1, SEEK_CUR);
				return 1;
			}
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para cadenas de caracteres
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataCadenaCaracteres(automata *a , char c) {
	//fprintf(stdout, "Cadenaestado:%d char:%c\n",(*a).estado,c);
	if (isComment == 1) {(*a).estado = -1;}
	switch ((*a).estado) {
		case 0:
			// Checa " y '
			if ((int)c == 34) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				isString = 1;
				return 0;
			}
			if ((int)c == 39) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				isString = 1;
				return 0;
			}
			if (c == ' ' || c == '\n') {
				return 0;
			}
			// Si comienza con algo mas invalido automata
			(*a).estado = -1;
			break;
		case 1:
			if ((int)c != 34) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			} else {
				// Si es " que cierra
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"Cadena de caracteres");
				lexema((*a).buffer, "Cadena de caracteres");
                
                strcpy(inputLex[inputSizeLex],"num");
				strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			}
			return 0;
			break;
		case 2:
			if ((int)c != 39) {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				return 0;
			} else {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"constant");
				lexema((*a).buffer, "constant");
                
                strcpy(inputLex[inputSizeLex],(*a).buffer);
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			}
			return 0;
			break;
		default:
			return 0;
			break;
	}
    return 0;
}

/**
 * Automata para operadores de comparacion (<,>,>=,<=,==,!=)
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataOperadoresComparacion(automata *a , char c) {
	if (isComment == 1) {(*a).estado = -1;}
	//fprintf(stdout, "Asigestado:%d char:%c\n",(*a).estado,c);
	switch ((*a).estado) {
		case 0:
			if (c == '=') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (c == '!') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (c == '>') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			if (c == '<') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			if (c == ' ' || c == '\n') {
				return 0;
			}
			// Si comienza con algo mas
			(*a).estado = -1;
			return 0;
			break;
		case 1:
			if (c == '=') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"rel_op");
				lexema((*a).buffer, "rel_op");
                
                strcpy(inputLex[inputSizeLex],"rel_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
		case 2:
			if (c == '=') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"rel_op");
				lexema((*a).buffer, "rel_op");
                
                strcpy(inputLex[inputSizeLex],"rel_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			} else {
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"rel_op");
				lexema((*a).buffer, "rel_op");
                
                strcpy(inputLex[inputSizeLex],"rel_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				fseek(fuente, -1, SEEK_CUR);
				return 1;
			}
			return 0;
			break;
		default:
			return 0;
			break;
	}
}

/**
 * Automata para operadores logicos (&&,||)
 *
 * @param automata
 * @param char
 * @return int
 **/
int automataOperadoresLogicos(automata *a , char c) {
	//fprintf(stdout, "Loggestado:%d char:%c\n",(*a).estado,c);
	if (isComment == 1) {(*a).estado = -1;}
	switch ((*a).estado) {
		case 0:
			if (c == '&') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 1;
				return 0;
			}
			if (c == '|') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				(*a).estado = 2;
				return 0;
			}
			if (c == ' ' || c == '\n') {
				return 0;
			}
			// Si comienza con algo mas
			(*a).estado = -1;
			return 0;
			break;
		case 1:
			if (c == '&') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"boolean_op");
				lexema((*a).buffer, "boolean_op");
                
                strcpy(inputLex[inputSizeLex],"boolean_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
		case 2:
			if (c == '|') {
				sprintf((*a).buffer+getLastIndex((*a).buffer), "%c", c);
				fprintf(stdout, "%s\t%s\n",(*a).buffer,"boolean_op");
				lexema((*a).buffer, "boolean_op");

                strcpy(inputLex[inputSizeLex],"boolean_op");
                strcpy(inputRealLex[inputSizeLex],(*a).buffer);
				
				inputSizeLex++;
                
				return 1;
			}
			(*a).estado = -1;
			return 0;
			break;
		default:
			return 0;
			break;
	}
}
