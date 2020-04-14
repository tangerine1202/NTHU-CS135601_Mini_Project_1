// #include "type_vars.h"

#ifndef LEXER_PARSER_H
#define LEXER_PARSER_H

/*
Something like Python
>> y = 2
>> z = 2
>> x = 3*y + 4/(2*z)

*/

/*
the only type: integer
everything is an expression

	statement 	:= END | expr END
	expr      	:= term expr_tail
	expr_tail 	:= ADD_SUB_AND_OR_XOR term expr_tail | NiL
	term      	:= factor term_tail
	term_tail 	:= MUL_DIV factor term_tail|NiL
	factor    	:= INT | ADD_SUB INT | ADD_SUB ID | ID ASSIGN expr| ID | LPAREN expr RPAREN
                        (for negative number e.g. (-1), (-3*2))
*/

int getValFromLexeme(void);
int setSbVal(char *, int);
BTNode *factor(void);
BTNode *term(void);
BTNode *term_tail(BTNode *);
BTNode *expr(void);
BTNode *expr_tail(BTNode *);
void statement(void);

TokenSet getToken(void);
char *getLexeme(void);
void advance(void);
int match(TokenSet);
BTNode *makeNode(TokenSet, const char *);

#endif