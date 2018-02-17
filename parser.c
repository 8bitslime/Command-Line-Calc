#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parser.h"

typedef struct stack_data_u {
	operator_type_t op;
	ast_node_t *node;
} stack_data_t;

typedef struct stack_s {
	stack_data_t *data;
	int size;
	int capacity;
} stack_t;

static stack_t *stack_alloc(int capacity) {
	stack_t *out = malloc(sizeof(stack_t) + sizeof(stack_data_t) * capacity);
	
	if (out == NULL) {
		printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	out->data = (stack_data_t*)&out[1];
	out->size = 0;
	out->capacity = capacity;
	
	return out;
}

static stack_data_t stack_get_top(stack_t *stack) {
	if (stack->size == 0) {
		return (stack_data_t) {NOP, NULL};
	} else {
		return stack->data[stack->size - 1];
	}
}

static stack_data_t stack_pop(stack_t *stack) {
	if (stack->size == 0) {
		return (stack_data_t) {NOP, NULL};
	} else {
		return stack->data[--stack->size];
	}
}

static void stack_push(stack_t **stack_ptr, stack_data_t op) {
	stack_t *stack = *stack_ptr;
	
	if (stack->size >= stack->capacity) {
		int newSize = stack->capacity * 2;
		
		stack = realloc(stack, sizeof(stack_t) + sizeof(stack_data_t) * newSize);
		
		if (stack == NULL) {
			printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
		
		stack->data = (stack_data_t*)&stack[1];
		stack->capacity = newSize;
		
		*stack_ptr = stack;
	}
	
	stack->data[stack->size++] = op;
}

static int operator_get_precedence(operator_type_t op) {
	switch (op) {
	case ADD:
	case SUB:
		return 1;
	case MUL:
	case DIV:
		return 2;
	case POW:
		return 3;
	case FUNC:
		return 4;
	case NOP:
	default:
		return 0;
	}
}

static operator_type_t operator_parse_string(const char *string) {
	//TODO: lookup table for operators and associativity
	if (string == NULL) {
		return NOP;
	} else if (strncmp(string, "+", 1) == 0) {
		return ADD;
	} else if (strncmp(string, "-", 1) == 0) {
		return SUB;
	} else if (strncmp(string, "*", 1) == 0) {
		return MUL;
	} else if (strncmp(string, "/", 1) == 0) {
		return DIV;
	}
	
	return NOP;
}

static ast_node_t *ast_alloc_node(int numChildren) {
	ast_node_t *out = malloc(sizeof(ast_node_t) + sizeof(ast_node_t*) * numChildren);
	
	if (out == NULL) {
		printf("Memory allocation failed in %s on line %d\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	out->children = (ast_node_t**)&out[1];
	out->numChildren = numChildren;
	
	for (int i = 0; i < numChildren; i++) {
		out->children[i] = NULL;
	}
	
	return out;
}

static ast_node_t *ast_build_node(token_t *token) {
	ast_node_t *out = NULL;
	
	switch (token->type) {
	case TYPE_NUMBER_LITERAL:
		out = ast_alloc_node(0);
		out->type = NODE_VALUE;
		out->data.value = (number_t){atof(token->string)};
		break;
	
	case TYPE_OPERATOR:
		//TODO: check operator arguments
		out = ast_alloc_node(2);
		out->data.op = operator_parse_string(token->string);
		out->type = NODE_OPERATOR;
		break;
	
	case TYPE_L_PAREN:
		out = ast_alloc_node(0);
		out->type = NODE_L_PAREN;
		break;
		
	default:
		break;
	}
	
	if (out != NULL) {
		out->token = token;
	}
	
	return out;
}

ast_node_t *ast_build_tree(lex_array_t *tokens) {
	stack_t *op_stack   = stack_alloc(16);
	stack_t *expr_stack = stack_alloc(16);
	
	for (int i = 0; i < tokens->size; i++) {
		token_t *token = tokens->tokens[i];
		
		//TODO: check if vector/array literal [1, 2, 3]
		//TODO: check if function or code block
		
		if (token->type == TYPE_L_PAREN) {
			stack_data_t paren_push = {
				L_PAREN,
				NULL
			};
			stack_push(&op_stack, paren_push);
			continue;
		} else if (token->type == TYPE_R_PAREN) {
			while (stack_get_top(op_stack).op != L_PAREN && stack_get_top(op_stack).op != NOP) {
				stack_data_t op_stack_data = stack_pop(op_stack);
				ast_node_t *top_op_stack = op_stack_data.node;
				ast_node_t *expr1 = stack_pop(expr_stack).node;
				ast_node_t *expr0 = stack_pop(expr_stack).node;
				
				//implicit leading 0
				//TODO: binary operators only (maybe?)
				if (expr0 == NULL) {
					top_op_stack->numChildren = 1;
					if (op_stack_data.op == SUB) {
						expr1->data.value.real *= -1;
					}
					top_op_stack->children[0] = expr1;
				} else {
					top_op_stack->children[0] = expr0;
					top_op_stack->children[1] = expr1;
				}
				
				stack_data_t push = {
					op_stack_data.op,
					top_op_stack
				};
				stack_push(&expr_stack, push);
			}
			if (stack_get_top(op_stack).op == L_PAREN) {
				stack_pop(op_stack);
			} else {
				//TODO: error handeling or let it slide?
			}
			continue;
			
		}
		
		ast_node_t *node = ast_build_node(token);
		
		stack_data_t stack_top = {
			NOP,
			node
		};
		
		switch (node->type) {
		case NODE_VALUE:
			stack_push(&expr_stack, stack_top);
			break;
		
		case NODE_OPERATOR:
			{
				stack_top.op = node->data.op;
				int op = operator_get_precedence(stack_top.op);
				
				operator_type_t op_top_stack_type = stack_get_top(op_stack).op;
				int op_top_stack = operator_get_precedence(op_top_stack_type);
				
				while (op_top_stack >= op) {
					ast_node_t *top_op_stack = stack_pop(op_stack).node;
					ast_node_t *expr1 = stack_pop(expr_stack).node;
					ast_node_t *expr0 = stack_pop(expr_stack).node;
					
					if (expr0 == NULL) {
						top_op_stack->numChildren = 1;
						if (op_top_stack_type == SUB) {
							expr1->data.value.real *= -1;
						}
						top_op_stack->children[0] = expr1;
					} else {
						top_op_stack->children[0] = expr0;
						top_op_stack->children[1] = expr1;
					}
					
					stack_data_t push = {
						op_top_stack_type,
						top_op_stack
					};
					stack_push(&expr_stack, push);
					
					operator_type_t op_top_stack_type = stack_get_top(op_stack).op;
					op_top_stack = operator_get_precedence(op_top_stack_type);
				}
				
				stack_push(&op_stack, stack_top);
				break;
			}
			
		default:
			free(node);
			continue;
		}
	}
	
	while (op_stack->size > 0) {
		operator_type_t op_top_stack_type = stack_get_top(op_stack).op;
		
		ast_node_t *top_op_stack = stack_pop(op_stack).node;
		ast_node_t *expr1 = stack_pop(expr_stack).node;
		ast_node_t *expr0 = stack_pop(expr_stack).node;
		
		if (expr0 == NULL) {
			top_op_stack->numChildren = 1;
			if (op_top_stack_type == SUB) {
				expr1->data.value.real *= -1;
			}
			top_op_stack->children[0] = expr1;
		} else {
			top_op_stack->children[0] = expr0;
			top_op_stack->children[1] = expr1;
		}
		
		stack_data_t push = {
			op_top_stack_type,
			top_op_stack
		};
		stack_push(&expr_stack, push);
	}
	
	ast_node_t *root = stack_pop(expr_stack).node;
	
	free(op_stack);
	free(expr_stack);
	
	return root;
}

void ast_free(ast_node_t *tree) {
	if (tree == NULL) {
		return;
	} 
	
	#pragma omp parallel for
	for (int i = 0; i < tree->numChildren; i++) {
		if (tree->children[i] != NULL) {
			ast_free(tree->children[i]);
		}
	}
	
	free(tree);
}