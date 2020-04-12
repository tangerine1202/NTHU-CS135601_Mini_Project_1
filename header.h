#ifndef HEADER_H
#define HEADER_H

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

#define MAXLEN 256
#define TBLSIZE 64

typedef enum
{
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    DEBUG_FACROT_ORANDXOR
} ErrorType;
typedef enum
{
    UNKNOWN,
    END,
    INT,
    ID,
    ORANDXOR,
    ADDSUB,
    MULDIV,
    ASSIGN,
    LPAREN,
    RPAREN,
    ENDFILE
} TokenSet;

typedef struct
{
    char name[MAXLEN];
    int val;
} Symbol;

typedef struct _Node
{
    char lexeme[MAXLEN];
    TokenSet data;
    int val;
    struct _Node *left, *right;
} BTNode;

// TokenSet lookahead = UNKNOWN;
// char lexeme[MAXLEN];

BTNode *factor(void);
BTNode *term(void);
BTNode *term_tail(BTNode *);
BTNode *expr(void);
BTNode *expr_tail(BTNode *);
void statement(void);
char *getLexeme(void);
TokenSet getToken(void);
void advance(void);
void error(ErrorType);
int match(TokenSet);
int evaluateTree(BTNode *);
void printPrefix(BTNode *);
void freeTree(BTNode *);
BTNode *makeNode(TokenSet, const char *);
int getval(void);
int setval(char *, int);

#endif