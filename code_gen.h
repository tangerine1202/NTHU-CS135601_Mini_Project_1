#ifndef CODE_GEN_H
#define CODE_GEN_H

#define MAXREG 8

typedef struct
{
    int id;
    int val;
    int used;
    char name[3]; // r0, r1, r2...
} Register;

extern Register reg[MAXREG];

void codeGenerate(BTNode *);
void initReg();
Register *generateAsmCode(BTNode *);
Register *getUnusedReg();
void returnReg(Register *);
int getAddr(char *);
int getAddrVal(int);
int getAddrAssigned(int);
void setAddrAssigned(int);

// instruction
void MOV_REG_REG(Register *, Register *);
void MOV_REG_INT(Register *, int);
void MOV_REG_ADDR(Register *, int, char *, int);
void MOV_ADDR_REG(int, Register *, char *, int);

void ADD_REG_REG(Register *, Register *);
void SUB_REG_REG(Register *, Register *);
void MUL_REG_REG(Register *, Register *);
void DIV_REG_REG(Register *, Register *);

void AND_REG_REG(Register *, Register *);
void OR_REG_REG(Register *, Register *);
void XOR_REG_REG(Register *, Register *);

void EXIT_instruction(int);

// (deprecated pure instruction)
/*
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
*/

#endif