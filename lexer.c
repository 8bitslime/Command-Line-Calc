#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include "lexer.h"

int main() {
	
	FILE *file;
	
	file = fopen("lexer.c", "rb");
	
	if (file == NULL) {
		printf("file couldn't open!\n");
		exit(EXIT_FAILURE);
	}
	
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = malloc(length + 1);
	
	if (buffer == NULL) {
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	
	fread(buffer, 1, length, file);
	fclose(file);
	
	buffer[length] = 0;
	
	lex_array_t *array = lex(buffer);
	
	//for (int i = 0; i < array->size; i++) {
	//	printf("%s\n", array->tokens[i]->string);
	//}
	
	return 0;
}

static lex_array_t *lex_alloc(int capacity) {
	lex_array_t *out = malloc(sizeof(lex_array_t) + sizeof(token_t*) * capacity);
	
	if (out == NULL) {
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	
	out->tokens = (token_t**)(((char*)out) + sizeof(lex_array_t));
	out->size = 0;
	out->capacity = capacity;
	
	return out;
}

static void lex_pushback(lex_array_t **lexArray, const char *string, int begin, int end, token_type_t type) {
	lex_array_t *array = *lexArray;
	
	if (array->size >= array->capacity) {
		int newCapacity = array->capacity * 2;
		array = realloc(array, sizeof(lex_array_t) + sizeof(token_t*) * newCapacity);
		
		if (array == NULL) {
			printf("Memory allocation failed at line: %d\n", __LINE__);
			exit(EXIT_FAILURE);
		}
		
		array->tokens = (token_t**)(((char*)array) + sizeof(lex_array_t));
		array->capacity = newCapacity;
		
		*lexArray = array;
	}
	
	int stringSize = end - begin;
	
	token_t *token = malloc(sizeof(token_t) + sizeof(char) * (stringSize + 1));
	
	if (token == NULL) {
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	
	token->type = type;
	token->string = ((char*)token) + sizeof(token_t);
	
	for (int i = 0; i < stringSize; i++) {
		token->string[i] = tolower(string[i + begin]);
	}
	token->string[stringSize] = 0;
	
	array->tokens[array->size] = token;
	array->size++;
}

lex_array_t *lex(const char *input) {
	lex_array_t *out = lex_alloc(16);
	
	enum {
		NO_STATE = 0,
		NAME,
		NUMBER,
		NUMBER_W_DECIMAL,
		NUMBER_HEX,
		STRING,
		OPERATOR
	};
	
	int state      = NO_STATE;
	int begin      = 0;
	char beginChar = 0;
	
	int i;
	for (i = 0; input[i]; i++) {
		char cur = input[i];
		
		loop:
		if (state == NO_STATE) {
			switch (cur) {
			case '(':
				lex_pushback(&out, input, i, i + 1, TYPE_L_PAREN);
				continue;
			case ')':
				lex_pushback(&out, input, i, i + 1, TYPE_R_PAREN);
				continue;
			case '{':
				lex_pushback(&out, input, i, i + 1, TYPE_L_CURLY);
				continue;
			case '}':
				lex_pushback(&out, input, i, i + 1, TYPE_R_CURLY);
				continue;
			case '[':
				lex_pushback(&out, input, i, i + 1, TYPE_L_BRACKET);
				continue;
			case ']':
				lex_pushback(&out, input, i, i + 1, TYPE_R_BRACKET);
				continue;
			}
			
			if (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r') {
				continue;
			} else if ((cur >= 'a' && cur <= 'z') || (cur >= 'A' && cur <= 'Z') || cur == '_') {
				state = NAME;
			} else if (cur >= '0' && cur <= '9') {
				state = NUMBER;
			} else if (cur == '.') {
				state = NUMBER_W_DECIMAL;
			} else if (cur == '"' || cur == '\'') {
				state = STRING;
			} else if (cur == '+' || cur == '-' || cur == '*' || cur == '/' || cur == '^' || cur == '=') {
				state = OPERATOR;
			}
			
			begin = i;
			beginChar = cur;
			continue;
		} else {
			if (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r' && state != STRING) {
				token_type_t type;
				switch (state) {
				case NUMBER:
				case NUMBER_W_DECIMAL:
				case NUMBER_HEX:
					type = TYPE_NUMBER_LITERAL;
					break;
				case STRING:
					type = TYPE_STRING_LITERAL;
					break;
				case OPERATOR:
					type = TYPE_OPERATOR;
					break;
				case NAME:
					type = TYPE_IDENTIFIER;
					break;
				}
				lex_pushback(&out, input, begin, i, type);
				state = NO_STATE;
				continue;
			}
			
			switch (state) {
			case NAME:
				if ((cur >= 'a' && cur <= 'z') || (cur >= 'A' && cur <= 'Z') || cur == '_' || (cur >= '0' && cur <= '9')) {
					continue;
				} else {
					lex_pushback(&out, input, begin, i, TYPE_IDENTIFIER);
					state = NO_STATE;
					goto loop;
				}
				break;
			case NUMBER:
				if (cur >= '0' && cur <= '9') {
					continue;
				} else if (tolower(cur) == 'x' && begin == (i - 1)) {
					state = NUMBER_HEX;
					continue;
				} else if (cur == '.') {
					state = NUMBER_W_DECIMAL;
					continue;
				} else {
					lex_pushback(&out, input, begin, i, TYPE_NUMBER_LITERAL);
					state = NO_STATE;
					goto loop;
				}
				break;
			case NUMBER_W_DECIMAL:
				if (cur >= '0' && cur <= '9') {
					continue;
				} else {
					lex_pushback(&out, input, begin, i, TYPE_NUMBER_LITERAL);
					state = NO_STATE;
					goto loop;
				}
				break;
			case STRING:
				if (cur == '"' && beginChar == '"' && input[i-1] != '\\') {
					lex_pushback(&out, input, begin, i, TYPE_STRING_LITERAL);
					state = NO_STATE;
					continue;
				} else {
					continue;
				}
				break;
			case OPERATOR:
				if (cur == '+' || cur == '-' || cur == '*' || cur == '/' || cur == '^' || cur == '=') {
					continue;
				} else {
					lex_pushback(&out, input, begin, i, TYPE_OPERATOR);
					state = NO_STATE;
					goto loop;
				}
				break;
			}
		}
	}
	
	if (state != NO_STATE) {
		token_type_t type;
		switch (state) {
		case NUMBER:
		case NUMBER_W_DECIMAL:
		case NUMBER_HEX:
			type = TYPE_NUMBER_LITERAL;
			break;
		case STRING:
			type = TYPE_STRING_LITERAL;
			break;
		case OPERATOR:
			type = TYPE_OPERATOR;
			break;
		case NAME:
			type = TYPE_IDENTIFIER;
			break;
		}
		lex_pushback(&out, input, begin, i, type);
	}
	
	return out;
}