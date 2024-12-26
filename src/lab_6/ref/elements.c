
#include <string.h>
#define MAX_ID_LEN 32
#define MAX_STACK_LEN 250

typedef enum Relation 
{ None=' ', Before='<', Together='=', After='>', Dual='%' } Relation;

typedef struct Symbol
{
	char symbol;
	Relation rel;
	char id[MAX_ID_LEN+1];
	int value;
} Symbol;

typedef struct Stack
{
	Symbol w[MAX_STACK_LEN];
	int top;
} Stack;

typedef struct Rule
{
	char left;
	char * right;
} Rule;

const char alpabet[] = "LSETMIC=+-/*();#";

Rule rules[] = 
{
	{'_', "#L#"},
	{'S', "I=E;"},
	{'E', "E+T"}, {'E', "E-T"}, {'E', "T"}
	/* ... */
};

char matrix[][] = 
{
/*     LSETMIC=+-/*();#  */ 
/*L*/	" <   %         =",
/*S*/	"     <         <"
			/* ... */
};

int charIndex(char c)
{
	char *t;
	t = strchr(alpabet, c);
	if (t)
		return t-alpabet;
	else
		Error("Unknown symbol"); /* print error and exit programm */
}

Relation f(char x, char y)
{
	/* find relation in matrix by indexes of symbols x and y */
}

int g() /* -1 = ERROR: rule not found */
{
	/* find rule index in rules for stack top subsiquence between < and > */
}

/* функция свертки (в стеке) */

/* функция семантики */

	n = g(y);
	...
	switch (n)
	{
		case 1: /* E+T */
			z.value = e.value + t.value; /* e and t - symbols from stack,
														 z - new symbol that will pushed to stack */
			break;
		/* ... */
	}

