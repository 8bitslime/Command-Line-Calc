#ifndef TYPES_H
#define TYPES_H

typedef double real;

typedef enum {
	NOP = 0,
	ADD,
	SUB,
	MUL,
	DIV,
	POW,
	OPEN_PAREN,
	CLOSE_PAREN,
	_UNARY_OPS,
	SIN,
	COS,
	TAN
} operation_t;

typedef struct infix_s {
	real val;
	operation_t op;
	struct infix_s *next;
} infix_t;

#endif