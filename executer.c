#include <stdlib.h>
#include <stdio.h>

#include "executer.h"

//TODO: error string

ast_node_t *simplify_tree(ast_node_t *tree) {
	
	if (tree == NULL) {
		return NULL;
	}
	
	if (tree->type == NODE_OPERATOR) {
		ast_node_t *first_child = simplify_tree(tree->children[0]);
		
		if (first_child == NULL) {
			return NULL;
		}
		
		double accum;
		if (first_child->type == NODE_VALUE) {
			//TODO: make number_t
			accum = first_child->data.value.real;
			free(first_child);
		}
		
		//#pragma omp parallel for
		for (int i = 1; i < tree->numChildren; i++) {
			ast_node_t *child = simplify_tree(tree->children[i]);
			
			if (child == NULL) {
				return NULL;
			}
			
			if (child->type == NODE_VALUE) {
				switch (tree->data.op) {
				case ADD:
					accum += child->data.value.real;
					break;
				case SUB:
					accum -= child->data.value.real;
					break;
				case MUL:
					accum *= child->data.value.real;
					break;
				case DIV:
					accum /= child->data.value.real;
					break;
				default:
					break;
				}
				free(child);
			}
			//TODO: variables
		}
		
		tree->numChildren = 0;
		tree->type = NODE_VALUE;
		tree->data.value = (number_t){accum};
	}
	
	return tree;
}