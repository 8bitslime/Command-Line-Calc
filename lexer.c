#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include "lexer.h"

static char OPERATOR_CHARS[] = {
	'+', '-', '*', '/', '^', '=', '|', '&', '~', ':', '%', '<', '>', '?', '!'
};

static char* OPERATOR_LIST[] = {
	"++", "--",
	"+=", "-=", "*=", "/=", "^=",
	"<=", ">=",
	"+", "-", "*", "/", "^", "%", "!"
	"<", ">", "&", "|", "~"
};

static int isOperator(char c) {
	for (int i = 0; i < (sizeof(OPERATOR_CHARS) / sizeof(char)); i++) {
		if (c == OPERATOR_CHARS[i]) {
			return 1;
		}
	}
	return 0;
}

static lex_array_t *lex_alloc(int capacity) {
	lex_array_t *out = malloc(sizeof(lex_array_t) + sizeof(token_t*) * capacity);
	
	if (out == NULL) {
		printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	out->tokens = (token_t**)(((char*)out) + sizeof(lex_array_t));
	out->size = 0;
	out->capacity = capacity;
	
	return out;
}

static void lex_pushback(lex_array_t **lexArray, const char *string, int begin, int end, token_type_t type) {
	lex_array_t *array = *lexArray;
	
	if (type == _TYPE_OPERATOR_UNSPLIT) {
		for (int i = 0; i < (sizeof(OPERATOR_LIST) / sizeof(char*)); i++) {
			int opSize = strlen(OPERATOR_LIST[i]);
			if (strncmp(string + begin, OPERATOR_LIST[i], opSize) == 0) {
				lex_pushback(lexArray, string, begin, begin + opSize, TYPE_OPERATOR);
				begin += opSize;
			}
			if (begin >= end) {
				return;
			}
		}
	}
	
	if (array->size >= array->capacity) {
		int newCapacity = array->capacity * 2;
		array = realloc(array, sizeof(lex_array_t) + sizeof(token_t*) * newCapacity);
		
		if (array == NULL) {
			printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
		
		array->tokens = (token_t**)(((char*)array) + sizeof(lex_array_t));
		array->capacity = newCapacity;
		
		*lexArray = array;
	}
	
	int stringSize = end - begin;
	
	token_t *token = malloc(sizeof(token_t) + sizeof(char) * (stringSize + 1));
	
	if (token == NULL) {
		printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
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
		STRING_ESCAPE,
		OPERATOR,
		COMMENT
	};
	
	int state      = NO_STATE;
	int begin      = 0;
	char beginChar = 0;
	
	int i;
	for (i = 0; input[i]; i++) {
		char cur = input[i];
		
		loop:
		
		if (state == COMMENT) {
			if (cur == '\n' || cur == '\r') {
				state = NO_STATE;
				continue;
			}
			continue;
		} else if (cur == '#') {
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
			state = COMMENT;
			continue;
		}
		
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
			case ',':
				lex_pushback(&out, input, i, i + 1, TYPE_OPERATOR);
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
			} else if (isOperator(cur)) {
				state = OPERATOR;
			}
			
			begin = i;
			beginChar = cur;
			continue;
		} else {
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
			case STRING_ESCAPE:
				state = STRING;
				continue;
			case STRING:
				if (cur == '"' && beginChar == '"') {
					lex_pushback(&out, input, begin, i+1, TYPE_STRING_LITERAL);
					state = NO_STATE;
					continue;
				} else if (cur == '\'' && beginChar == '\'') {
					lex_pushback(&out, input, begin, i+1, TYPE_STRING_LITERAL);
					state = NO_STATE;
					continue;
				} else if (cur == '\\'){
					state = STRING_ESCAPE;
					continue;
				} else {
					continue;
				}
				break;
			case OPERATOR:
				if (isOperator(cur)) {
					continue;
				} else {
					lex_pushback(&out, input, begin, i, TYPE_OPERATOR);
					state = NO_STATE;
					goto loop;
				}
				break;
			}
			
			if (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r') {
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

void lex_free(lex_array_t *array) {
	#pragma omp parallel for
	for (int i = 0; i < array->size; i++) {
		free(array->tokens[i]);
	}
	free(array);
}

void lex_dump(const lex_array_t *array, const char *seperator) {
	for (int i = 0; i < array->size; i++) {
		printf("%s %s", array->tokens[i]->string, seperator);
	}
}