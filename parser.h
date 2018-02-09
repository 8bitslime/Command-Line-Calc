#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum ast_node_type_e {
	NODE_UNKNOWN = 0,
	NODE_VALUE,
	NODE_OPERATOR,
	NODE_IDENTIFIER_FUNC,
	NODE_IDENTIFIER_VAR,
	NODE_STRING, //Probably not going to use this one
	NODE_L_PAREN
} ast_node_type_t;

typedef union number_u {
	double real;
	//TODO: more vectors, arrays, whatever
} number_t;

typedef struct ast_node_s {
	ast_node_type_t type;
	number_t value;
	token_t *token;
	struct ast_node_s **children;
	int numChildren;
} ast_node_t;

ast_node_t *ast_build_tree(lex_array_t *tokens);
void ast_free(ast_node_t *tree);

#endif