#ifndef LEXER_H
#define LEXER_H

typedef enum token_type_e {
	TYPE_UNKNOWN = 0,
	
	TYPE_IDENTIFIER,
	TYPE_OPERATOR,
	
	TYPE_NUMBER_LITERAL,
	TYPE_STRING_LITERAL,
	
	
	TYPE_L_PAREN,
	TYPE_R_PAREN,
	
	TYPE_L_CURLY,
	TYPE_R_CURLY,
	
	TYPE_L_BRACKET,
	TYPE_R_BRACKET,
} token_type_t;

typedef struct token_s {
	token_type_t type;
	char *string;
} token_t;

typedef struct lex_array_s {
	token_t **tokens;
	int size;
	int capacity;
} lex_array_t;

lex_array_t *lex(const char *input);
void lex_free(lex_array_t *array);

#endif