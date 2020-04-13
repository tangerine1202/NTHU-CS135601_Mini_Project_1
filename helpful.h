#ifndef HELPFUL_H
#define HELPFUL_H

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

extern int DEBUG_MODE;

void evaluate(BTNode *);
int evaluateTree(BTNode *);
void printPrefix(BTNode *);
void printTree(BTNode *, int);
void freeTree(BTNode *);
void error(ErrorType);

#endif