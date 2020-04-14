#ifndef STRUCT_VARS_H
#define STRUCT_VARS_H

#define MAXLEN 256
#define TBLSIZE 64
#define MAXREG 8

typedef enum
{
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    DEBUG_FACROT_ORANDXOR,
    REG_RUNOUT,
    DIV_BY_ZERO,
    VAR_UNASSIGNED,
    CANT_GET_ADDR,
    UNEXPECT_TOKENTYPE,
    WRONG_ADDR,
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
    int assigned;
} Symbol;

typedef struct _Node
{
    char lexeme[MAXLEN];
    TokenSet data;
    int val;
    int weight;
    struct _Node *left, *right;
} BTNode;

typedef struct
{
    char name[3]; // r0, r1, r2...
    int val;
    int used;
} Register;

extern int DEBUG_MODE;

extern TokenSet lookahead;
extern char lexeme[MAXLEN];
extern Symbol sbtable[TBLSIZE];
extern int sbcount;
extern Register reg[MAXREG];

#endif