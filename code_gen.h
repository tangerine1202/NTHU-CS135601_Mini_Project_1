// #include "type_vars.h"

#ifndef CODE_GEN_H
#define CODE_GEN_H

void codeGenerate(BTNode *);
void initReg();
Register *generateCode(BTNode *);
Register *getUnusedReg();
void returnReg(Register *);

// instruction
void MOV_REG_REG(Register *, Register *);
void MOV_REG_INT(Register *, int);
void MOV_REG_ADDR(Register *, int);
void MOV_ADDR_REG(int, Register *);

void ADD_REG_REG(Register *, Register *);
void SUB_REG_REG(Register *, Register *);
void MUL_REG_REG(Register *, Register *);
void DIV_REG_REG(Register *, Register *);

void AND_REG_REG(Register *, Register *);
void OR_REG_REG(Register *, Register *);
void XOR_REG_REG(Register *, Register *);

void EXIT_INSTRUCTION(int);

#endif