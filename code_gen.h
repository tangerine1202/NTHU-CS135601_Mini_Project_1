// #include "type_vars.h"

#ifndef CODE_GEN_H
#define CODE_GEN_H

void codeGenerate(BTNode *);
void initReg();
Register *generateAsmCode(BTNode *);
Register *getUnusedReg();
void setRegByInt(Register *, int);
void setRegByAddr(Register *, int);
void setRegByReg(Register *, Register *);
void setRegByRegWithOp(Register *, Register *, char *);
void setAddrByReg(int, Register *);
void returnReg(Register *);
int getAddr(char *);
char *getAddrName(int);
int getAddrVal(int);
int getAddrUnknownVal(int);
int getAddrAssigned(int);

// instruction
void MOV_REG_REG(Register *, Register *);
void MOV_REG_INT(Register *, int);
void MOV_REG_ADDR(Register *, int, char *, int, int);
void MOV_ADDR_REG(int, Register *, char *, int, int);

void ADD_REG_REG(Register *, Register *);
void SUB_REG_REG(Register *, Register *);
void MUL_REG_REG(Register *, Register *);
void DIV_REG_REG(Register *, Register *);

void AND_REG_REG(Register *, Register *);
void OR_REG_REG(Register *, Register *);
void XOR_REG_REG(Register *, Register *);

void EXIT_INSTRUCTION(int);

#endif