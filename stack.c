/*
 *  stack.c
 *  Analizador Sintactico
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define INT_DIGITS 600

typedef struct node {
    int value;
    struct node *next;
} Node;

typedef struct stack {
    Node *first;
	int size;
} Stack;



void initStack(Stack *s) {
    s->first = 0;
	s->size = 0;
}

int isEmpty(Stack *s) {
    if (s->first == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int push(Stack *s, int ele) {
	Node *nuevo = (Node *) malloc(sizeof(Node));
	
	if (nuevo == 0) {
		return FALSE;
	}
	nuevo->value = ele;
	nuevo->next = 0;
	
	if (isEmpty(s)) {
		s->first = nuevo;
	} else {
		nuevo->next = s->first;
		s->first = nuevo;
	}
	s->size++;
	return TRUE;
}

int top(Stack *s) {
	if (isEmpty(s)) {
		return -1;
	} else {
		return s->first->value;
	}
}

int pop(Stack *s) {
	if (isEmpty(s)) {
		return -1;
	} else {
		int aux = s->first->value;
		Node *tmp = s->first;
		if (s->first->next == 0) {
			s->first = 0;
		} else {
			s->first = tmp->next;
		}
		free(tmp);
		s->size--;
		return aux;
	}
}

void clear(Stack *s) {
	while (pop(s) != -1){}
	s->size = 0;
}

void despliega(Stack *s, int *pila) {
	int aux;
	Node *tmp = s->first;
	for (aux = s->size - 1 ; aux >= 0; aux--) {
		pila[aux] = tmp->value;
		tmp = tmp->next;
	}
}


char *itoaC(i)
int i;
{
	// Room for INT_DIGITS digits, - and '\0' //
	static char buf[INT_DIGITS + 2];
	char *p = buf + INT_DIGITS + 1;	//points to terminating '\0'
	if (i >= 0) {
		do {
			*--p = '0' + (i % 10);
			i /= 10;
		} while (i != 0);
		return p;
	}
	else {			// i < 0 
		do {
			*--p = '0' - (i % 10);
			i /= 10;
		} while (i != 0);
		*--p = '-';
	}
	return p;
}