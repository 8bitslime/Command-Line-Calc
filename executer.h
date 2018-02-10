#include "parser.h"

//Returns the most simplified tree or NULL if error
//Will change the tree and free all dereferenced nodes
//If NULL is returned, the will NOT be the original one
ast_node_t *simplify_tree(ast_node_t *tree);