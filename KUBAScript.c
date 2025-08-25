#include "mpc.h"
#include <stdio.h>

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif



typedef struct {
	int type;
	long num;
	int err;
} lval;


enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };


lval lval_num(long x) {
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}

lval lval_err(int x) {
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}

void lval_print(lval v) {
	switch (v.type)
	{
	case LVAL_NUM:
		printf("%li", v.num);
		break;

	case LVAL_ERR:
		if (v.err == LERR_DIV_ZERO) {
			printf("Error: DIvision by zero.");
		}
		if (v.err == LERR_BAD_OP) {
			printf("Error: Invalid operator.");
		}
		if (v.err == LERR_DIV_ZERO) {
			printf("Error: Invalid number.");
		}
		break;
	
	default:
		break;
	}
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }



long power(long x, long y) {
	long multiplier = x;
	while (y != 1) {
		x *= multiplier;
		y--;
	}
	return x;
}



lval evaluate_op(lval x, char* op, lval y) {

	if (x.type == LVAL_ERR) { return x; }
	if (y.type == LVAL_ERR) { return y; }

	if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
	if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
	if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
	if (strcmp(op, "/") == 0) {
		return y.num == 0
		? lval_err(LERR_DIV_ZERO)
		: lval_num(x.num / y.num);
	}
	if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
	if (strcmp(op, "^") == 0) { return lval_num(power(x.num, y.num)); }
	return lval_err(LERR_BAD_OP);
}



lval evaluate(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) {
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno !=  ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}
	
	char* op = t->children[1]->contents;

	lval x = evaluate(t->children[2]);

	int i = 3;
	while (strstr(t->children[i]->tag, "expr")) {
		x = evaluate_op(x, op, evaluate(t->children[i]));
		i++;
	}	
	return x;
}

long count_leaves(mpc_ast_t* t) {
	long x = 0;
	if (strstr(t->tag, "number") || strstr(t->tag, "operator")) {
		x += 1;
		return x;
	}
	x += 2;

	int i = 3;
	while (strstr(t->children[i]->tag, "expr")) {
		x += count_leaves(t->children[i]);
		i++;
	}	
	return x;
}


int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");	
	mpc_parser_t* Operator = mpc_new("operator");	
	mpc_parser_t* Expr = mpc_new("expr");	
	mpc_parser_t* Lispy = mpc_new("lispy");	
	
	mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+[.]?[0-9]?/ ;                   \
      operator : '+' | '-' | '*' | '/' | '%' | '^' ;      \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);



	puts("\n---------------------------");
	puts("Welcome to KUBAScript");
	puts("Lispy Version 0.0.0.0.2");
	puts("Press Ctrl+c to exit\n");
	
	while (1) {
		char* input = readline("lispy> ");
		add_history(input);
		
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			long leaves = count_leaves(r.output);
			printf("number of leaves: %li\n", leaves);
			lval result = evaluate(r.output);
			printf("result: ");
			lval_println(result);
			mpc_ast_delete(r.output);
		}
		else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		free(input);
	}
	
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	
	return 0;
}

