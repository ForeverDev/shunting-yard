#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum token_type {
	
	UNKNOWN		= 0x00,
	ADD			= 0x01,
	SUB			= 0x02,
	MUL			= 0x03,
	DIV			= 0x04,
	OPENPAR		= 0x05,
	CLOSEPAR	= 0x06,
	NUMBER		= 0x07

} token_type;

typedef struct token {
	
	token_type 	type;
	char		word[32];

} token;

typedef struct expression_state {
	
	// tokens to be inserted from when we lex
	token 		tok[1024];
	token*		tok_ptr;

	// operator stack, ops_ptr points to the top of the stack
	token 		ops[1024];
	token*		ops_ptr;

	// rpn stack, rpn_ptr points to the top of the stack
	token 		rpn[1024];
	token* 		rpn_ptr;

	// stack for parsing RPN
	double		ret[1024];
	double*		ret_ptr;

} expression_state;

// this array holds the prescendence of the operators (we define it in main)
unsigned int pres[64];

int main(int argc, char** argv) {

	printf("\n");
	
	pres[ADD] = 1;
	pres[SUB] = 1;
	pres[MUL] = 2;
	pres[DIV] = 2;
	
	expression_state* E = malloc(sizeof(expression_state));

	while (1) {

		char 	expr[1024];
		char* 	expr_ptr = expr;

		E->tok_ptr = E->tok;
		E->ops_ptr = E->ops;
		E->rpn_ptr = E->rpn;
		E->ret_ptr = E->ret;
	
		fgets(expr, 1024, stdin);
	
		// first, parse the expression into tokens
		while (*expr_ptr) {

			// c is the current char being parsed
			char c = *expr_ptr++;
			// buf is a general purpose buffer that we may
			// or may not use
			char buf[1024];
			// bp is a pointer to buf that we use to insert characters
			char* bp = buf;
			// t is the token to be added to the token array in E
			token t;
			if (c == 32) continue;
			if (isdigit(c)) {
				t.type = NUMBER;
				*bp++ = c;
				while (isdigit((c = *expr_ptr++)) || c == '.') {
					*bp++ = c;
				}
				*bp = 0;
				expr_ptr--;
				strcpy(t.word, buf);
			} else if (ispunct(c)) {
				t.word[0] = c;
				t.word[1] = 0;
				switch (c) {
					case '(':
						t.type = OPENPAR;
						break;
					case ')':
						t.type = CLOSEPAR;
						break;
					case '+':
						t.type = ADD;
						break;
					case '-':
						t.type = SUB;
						break;
					case '*':
						t.type = MUL;
						break;
					case '/':
						t.type = DIV;
						break;
					default:
						// TODO throw some sort of error saying unidentifiable token
						t.type = UNKNOWN;
						break;
				}
			}
			*++E->tok_ptr = t;
		}
		
		// now convert to rpn
		for (token* i = E->tok + 1; i < E->tok_ptr; i++) {
			switch (i->type) {
				case UNKNOWN:
					break;
				case NUMBER:
					*++E->rpn_ptr = *i;
					break;
				case OPENPAR:
					*++E->ops_ptr = *i;
					break;
				case CLOSEPAR:
					while (E->ops_ptr != E->ops && E->ops_ptr->type != OPENPAR) {
						*++E->rpn_ptr = *E->ops_ptr--;
					}
					E->ops_ptr--;
					break;
				case ADD:
				case SUB:
				case MUL:
				case DIV:
				{
					unsigned int p = pres[i->type];	
					while (E->ops_ptr != E->ops && E->ops_ptr->type != OPENPAR && p <= pres[E->ops_ptr->type]) {
						*++E->rpn_ptr = *E->ops_ptr--;
					}
					*++E->ops_ptr = *i;
					break;
				}
				default:
					break;
			}
		}
		for (token* i = E->ops_ptr; i > E->ops; i--) {
			*++E->rpn_ptr = *i;
		}

		// now parse the rpn for the expression result
		for (token* i = E->rpn + 1; i <= E->rpn_ptr; i++) {
			double a, b;
			switch (i->type) {
				case NUMBER:
					*++E->ret_ptr = atof(i->word);
					break;
				case ADD:
				case SUB:
				case MUL:
				case DIV:
					b = *E->ret_ptr--;
					a = *E->ret_ptr--;
					switch (i->type) {
						case ADD:
							*++E->ret_ptr = a + b; 
							break;
						case SUB:
							*++E->ret_ptr = a - b; 
							break;
						case MUL:
							*++E->ret_ptr = a * b; 
							break;
						case DIV:
							*++E->ret_ptr = a / b; 
							break;
					}
					break;
				default:
					break;
			}
		}

		printf("= %f\n\n", *E->ret_ptr);
	}

	return 0;
}
