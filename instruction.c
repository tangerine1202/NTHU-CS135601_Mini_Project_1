#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"

void MOV_REG_REG(Register *reg1, Register *reg2)
{
    printf("MOV %s %s\n", reg1->name, reg2->name);
}
void MOV_REG_INT(Register *reg1, int val)
{
    printf("MOV %s %d\n", reg1->name, val);
}
void MOV_REG_ADDR(Register *reg1, int addr2)
{
    printf("MOV %s [%d]\n", reg1->name, addr2);
}
void MOV_ADDR_REG(int addr1, Register *reg2)
{
    printf("MOV [%d] %s\n", addr1, reg2->name);
}

void ADD_REG_REG(Register *reg1, Register *reg2)
{
    printf("ADD %s %s\n", reg1->name, reg2->name);
}
void SUB_REG_REG(Register *reg1, Register *reg2)
{
    printf("SUB %s %s\n", reg1->name, reg2->name);
}
void MUL_REG_REG(Register *reg1, Register *reg2)
{
    printf("MUL %s %s\n", reg1->name, reg2->name);
}
void DIV_REG_REG(Register *reg1, Register *reg2)
{
    printf("DIV %s %s\n", reg1->name, reg2->name);
}

void OR_REG_REG(Register *reg1, Register *reg2)
{
    printf("OR %s %s\n", reg1->name, reg2->name);
}
void AND_REG_REG(Register *reg1, Register *reg2)
{
    printf("AND %s %s\n", reg1->name, reg2->name);
}
void XOR_REG_REG(Register *reg1, Register *reg2)
{
    printf("XOR %s %s\n", reg1->name, reg2->name);
}

void EXIT_instruction(int exitcode)
{
    printf("EXIT %d\n", exitcode);
}

/* ---------- debug: instruction ---------- */
void debug_MOV_REG_REG(Register *reg1, Register *reg2)
{
    printf("MOV %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_MOV_REG_INT(Register *reg1, int val)
{
    printf("MOV %s(%d)\t%d\n", reg1->name, reg1->val, val);
}
void debug_MOV_REG_ADDR(Register *reg1, int addr2, char *addrname, int addrval)
{
    printf("MOV %s(%d)\t%s[%d](%d)\n", reg1->name, reg1->val, addrname, addr2, addrval);
}
void debug_MOV_ADDR_REG(int addr1, Register *reg2, char *addrname, int addrval)
{
    printf("MOV %s[%d](%d)\t%s(%d)\n", addrname, addr1, addrval, reg2->name, reg2->val);
}

void debug_ADD_REG_REG(Register *reg1, Register *reg2)
{
    printf("ADD %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_SUB_REG_REG(Register *reg1, Register *reg2)
{
    printf("SUB %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_MUL_REG_REG(Register *reg1, Register *reg2)
{
    printf("MUL %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_DIV_REG_REG(Register *reg1, Register *reg2)
{
    printf("DIV %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}

void debug_OR_REG_REG(Register *reg1, Register *reg2)
{
    printf("OR %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_AND_REG_REG(Register *reg1, Register *reg2)
{
    printf("AND %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}
void debug_XOR_REG_REG(Register *reg1, Register *reg2)
{
    printf("XOR %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
}