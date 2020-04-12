#ifndef CODE_GEN_H
#define CODE_GEN_H

#define MAXREG 8

typedef enum
{
    REG_RUNOUT,
} CodeGen_ErrorType;

typedef struct
{
    int id;
    int val;
    int used;
    char name[3]; // r0, r1, r2...
} Register;

extern Register reg[MAXREG];

void init_reg();
Register *get_unused_reg();
void return_reg(Register *);
int getAddr(char *);
int getAddrVal(int);
Register *codeGenerate(BTNode *);
void code_gen_error(CodeGen_ErrorType);

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

void EXIT_instruction(int);

// debug instruction
// instruction
void debug_MOV_REG_REG(Register *, Register *);
void debug_MOV_REG_INT(Register *, int);
void debug_MOV_REG_ADDR(Register *, int, char *, int);
void debug_MOV_ADDR_REG(int, Register *, char *, int);

void debug_ADD_REG_REG(Register *, Register *);
void debug_SUB_REG_REG(Register *, Register *);
void debug_MUL_REG_REG(Register *, Register *);
void debug_DIV_REG_REG(Register *, Register *);

void debug_AND_REG_REG(Register *, Register *);
void debug_OR_REG_REG(Register *, Register *);
void debug_XOR_REG_REG(Register *, Register *);

#endif