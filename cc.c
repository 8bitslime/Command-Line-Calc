#include <stdio.h>
#include "executer.h"

int main (int argc, char **argv) {
	
	lex_array_t *tokens = lex("(1+2)*3");
	ast_node_t *tree    = ast_build_tree(tokens);
	
	if (simplify_tree(tree) != NULL) {
		printf("%f", tree->data.value.real);
	}
	
	lex_free(tokens);
	ast_free(tree);
	
	return 0;
}