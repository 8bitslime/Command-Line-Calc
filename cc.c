#include <stdio.h>
#include "executer.h"

int main (int argc, char **argv) {
	
	lex_array_t *tokens = lex("300 + 3");
	ast_node_t *tree    = ast_build_tree(tokens);
	
	lex_dump(tokens, "\n");
	
	if (simplify_tree(tree) != NULL) {
		//printf("%i\n", tree->data.op);
		printf("%f", tree->data.value.real);
	} else {
		printf("o shit");
	}
	
	lex_free(tokens);
	ast_free(tree);
	
	return 0;
}