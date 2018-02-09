#include "parser.h"

int main (int argc, char **argv) {
	
	lex_array_t *tokens = lex("1+2*3");
	ast_node_t *tree = ast_build_tree(tokens);
	
	lex_free(tokens);
	ast_free(tree);
	
	return 0;
}