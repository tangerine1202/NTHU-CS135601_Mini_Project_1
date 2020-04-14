#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

void MOV_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MOV %s %s\n", reg1->name, reg2->name);
}
void MOV_REG_INT(Register *reg1, int val)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%d\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10), val);
    else
        fprintf(stdout, "MOV %s %d\n", reg1->name, val);
}
void MOV_REG_ADDR(Register *reg1, int addr2, char *addrname, int addrval, int addrunknown)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%s[%d](%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               addrname, addr2, addrunknown ? "#" : itoa(addrval, buf, 10));
    else
        fprintf(stdout, "MOV %s [%d]\n", reg1->name, addr2);
}
void MOV_ADDR_REG(int addr1, Register *reg2, char *addrname, int addrval, int addrunknown)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s[%d](%s)\t%s(%s)\n",
               addrname, addr1, addrunknown ? "#" : itoa(addrval, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MOV [%d] %s\n", addr1, reg2->name);
}

void ADD_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("ADD %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "ADD %s %s\n", reg1->name, reg2->name);
}
void SUB_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("SUB %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "SUB %s %s\n", reg1->name, reg2->name);
}
void MUL_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MUL %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MUL %s %s\n", reg1->name, reg2->name);
}
void DIV_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("DIV %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "DIV %s %s\n", reg1->name, reg2->name);
}

void OR_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("OR %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "OR %s %s\n", reg1->name, reg2->name);
}
void AND_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("AND %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "AND %s %s\n", reg1->name, reg2->name);
}
void XOR_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("XOR %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "XOR %s %s\n", reg1->name, reg2->name);
}

void EXIT_INSTRUCTION(int exitcode)
{
    printf("EXIT %d\n", exitcode);
}

/* ---------- pure instruction ---------- */
/*
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
*/