#include <stdio.h>
#include "executer.h"

int main (int argc, char **argv) {
	
	lex_array_t *tokens = lex("1+2*3-3");
	ast_node_t *tree    = ast_build_tree(tokens);
	ast_node_t *simple  = simplify_tree(tree);
	
	if (simple != NULL) {
		printf("%f", simple->data.value.real);
	}
	
	lex_free(tokens);
	ast_free(tree);
	
	return 0;
}