#ifndef HELPFUL_H
#define HELPFUL_H

void evaluate(BTNode *);
int evaluateTree(BTNode *);
void printPrefix(BTNode *);
void printTree(BTNode *, int);
void freeTree(BTNode *);
void error(ErrorType);

int getAddr(char *);
char *getAddrName(int);
int getAddrVal(int);
int getAddrUnknownVal(int);
int getAddrAssigned(int);

int max(int, int);
int min(int, int);

inline void charswap(char *, char *);
char *reverse(char *, int, int);
char *itoa(int, char *, int);

#endif