#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "types.h"

void calcString(void);
void emptyStack(void);
void freeString(void);
int postfixToInfix(const char *expression);

static infix_t *stack = NULL;
static infix_t *infixString = NULL;

int main(int argc, char **argv) {
	
	char input[1024];
	
	if (argc <= 1) {
		printf("Command line Calculator shell mode.\nType 'exit' to quit.\n");
		while (1) {
			printf(">>");
			if (scanf("%[^\n]%*c", input)) {
				if (strcmp(input, "exit") == 0) {
					break;
				}
				
				postfixToInfix(input);
				emptyStack();
				
				if (infixString == NULL) {
					freeString();
					continue;
				}
				
				calcString();
				
				printf("%lf\n", infixString->val);
				freeString();
			}
		}
		return 0;
	}
	
	for (int i = 1; i < argc; i++) {
		sprintf(input, "%s %s", input, argv[i]);
	}
	
	postfixToInfix(input);
	
	emptyStack();
	
	if (infixString == NULL) {
		return 0;
	}
	
	calcString();
	
	printf("%lf\n", infixString->val);
	freeString();
	
	return 0;
}

void calcString() {
	infix_t *ptr = infixString;
	
	while (ptr != NULL) {
		
		if (ptr->op != NOP) {
			if (ptr == infixString) {
				break;
			} else {
				ptr = infixString;
				continue;
			}
		}
		
		infix_t *ptr2 = ptr->next;
		
		if (ptr2 != NULL) {
			
			infix_t *ptr3 = ptr2->next;
			
			if (ptr3 != NULL) {
				if (ptr2->op != NOP) {
					if (ptr == infixString) {
						break;
					}
					ptr = ptr3;
					continue;
				}
				
				if (ptr3->op == NOP) {
					if (ptr3->next == NULL) {
						break;
					}
					ptr = ptr2;
					continue;
				}
				
				switch (ptr3->op) {
					case ADD:
						ptr->val += ptr2->val;
						break;
					case SUB:
						ptr->val -= ptr2->val;
						break;
					case MUL:
						ptr->val *= ptr2->val;
						break;
					case DIV:
						ptr->val /= ptr2->val;
						break;
					case POW:
						ptr->val = pow(ptr->val, ptr2->val);
						break;
				}
				
				ptr->next = ptr3->next;
				ptr = infixString;
				
				free(ptr2);
				free(ptr3);
				continue;
			} else if (ptr == infixString) {
				break;
			} else {
				ptr = infixString;
				continue;
			}
		} else if (ptr == infixString) {
			break;
		} else {
			ptr = infixString;
			continue;
		}
	}
}

void emptyStack(void) {
	infix_t *i = stack;
	while (i != NULL) {
		infix_t *stringEnd = infixString;
		for (; stringEnd->next != NULL; stringEnd = stringEnd->next);
		stringEnd->next = i;
		i = i->next;
		stringEnd->next->next = NULL;
	}
	stack = NULL;
}

void freeString(void) {
	while (infixString != NULL) {
		infix_t *next = infixString->next;
		free(infixString);
		infixString = next;
	}
}

void parseString(char *string) {
	infix_t *next = calloc(sizeof(infix_t), 1);
	
	for (int i = 0; string[i]; i++) {
		string[i] = (char)tolower(string[i]);
	}
	
	if ((*string >= '0' && *string <= '9') || *string == '.') {
		next->val = atof(string);
	} else if (strcmp(string, "e") == 0) {
		next->val = 2.71828182845904523536;
	} else if (strcmp(string, "pi") == 0) {
		next->val = 3.14159265358979323846;
	} else if (*string == '+') {
		next->op = ADD;
	} else if (*string == '-') {
		next->op = SUB;
		if (infixString == NULL) {
			parseString("0.0");
		}
	} else if (*string == '*') {
		next->op = MUL;
	} else if (*string == '/') {
		next->op = DIV;
	} else if (*string == '^') {
		next->op = POW;
	} else {
		free(next);
		return;
	}
	
	if (!next) {
		return;
	}
	
	if (next->op == NOP) {
		if (infixString != NULL) {
			infix_t *i;
			for (i = infixString; i->next != NULL; i = i->next);
			i->next = next;
		} else {
			infixString = next;
			return;
		}
	} else {
		if (infixString == NULL) {
			parseString("0.0");
		}
		if (stack == NULL) {
			stack = next;
			return;
		}
		
		int stackPres = 1 + (stack->op == MUL || stack->op == DIV) + (stack->op == POW) * 2;
		int nextPres = 1 + (next->op == MUL || next->op == DIV) + (next->op == POW) * 2;
		
		if (stackPres >= nextPres) {
			
			infix_t *i;
			for (i = infixString; i->next != NULL; i = i->next);
			i->next = stack;
			stack = stack->next;
			
			if (stack == NULL) {
				stack = next;
				return;
			}
		}
		
		next->next = stack;
		stack = next;
	}
}

int postfixToInfix(const char *expression) {
	
	enum states {
		NO_STATE = 0,
		NUMBER,
		OPERATION,
		NAME
	};
	
	int state = NO_STATE;
	char stateSymbol = 0;
	int stateBegin;
	
	char token[128] = {0};
	
	int i = 0;
	for (; expression[i] > 0; i++) {
		
		char cur = expression[i];
		
		top:
		
		if (state == NO_STATE) {
			
			if ((cur >= '0' && cur <= '9') || cur == '.') {
				state = NUMBER;
				stateBegin = i;
				continue;
			} else if (cur == '+' || cur == '-' || cur == '*' || cur == '/' || cur == '^') {
				state = OPERATION;
				stateBegin = i;
				continue;
			} else if ((cur >= 'a' && cur <= 'z') || (cur >= 'A' && cur <= 'Z')) {
				state = NAME;
				stateBegin = i;
				continue;
			} else {
				continue;
			}
			
		} else {
			
			if (cur == ' ') {
				strncpy(token, expression + stateBegin, i - stateBegin);
				token[i - stateBegin] = 0;
				state = NO_STATE;
				parseString(token);
				continue;
			}
			
			switch (state) {
				case NUMBER:
					if ((cur >= '0' && cur <= '9') || cur == '.') {
						continue;
					} else {
						//end number
						strncpy(token, expression + stateBegin, i - stateBegin);
						token[i - stateBegin] = 0;
						state = NO_STATE;
						parseString(token);
						goto top;
					}
					break;
				case OPERATION:
					strncpy(token, expression + stateBegin, i - stateBegin);
					token[i - stateBegin] = 0;
					state = NO_STATE;
					parseString(token);
					goto top;
					break;
				case NAME:
					if ((cur >= 'a' && cur <= 'z') || (cur >= 'A' && cur <= 'Z')) {
						continue;
					} else {
						strncpy(token, expression + stateBegin, i - stateBegin);
						token[i - stateBegin] = 0;
						state = NO_STATE;
						parseString(token);
						goto top;
					}
					break;
			}
			
		}
	}
	
	if (state != NO_STATE) {
		strncpy(token, expression + stateBegin, i - stateBegin);
		token[i - stateBegin] = 0;
		state = NO_STATE;
		parseString(token);
	}
	
	return 0;
}