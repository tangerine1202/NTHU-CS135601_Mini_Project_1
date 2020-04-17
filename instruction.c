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
    printf("MOV %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}
void MOV_REG_INT(Register *reg1, int val)
{
    printf("MOV %s %d\n", reg1->name, val);
    total_clock_cycle += 10;
}
void MOV_REG_ADDR(Register *reg1, int addr2)
{
    printf("MOV %s [%d]\n", reg1->name, addr2);
    total_clock_cycle += 200;
}
void MOV_ADDR_REG(int addr1, Register *reg2)
{
    printf("MOV [%d] %s\n", addr1, reg2->name);
    total_clock_cycle += 200;
}

void ADD_REG_REG(Register *reg1, Register *reg2)
{
    printf("ADD %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}
void SUB_REG_REG(Register *reg1, Register *reg2)
{
    printf("SUB %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}
void MUL_REG_REG(Register *reg1, Register *reg2)
{
    printf("MUL %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 30;
}
void DIV_REG_REG(Register *reg1, Register *reg2)
{
    printf("DIV %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 50;
}

void OR_REG_REG(Register *reg1, Register *reg2)
{
    printf("OR %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}
void AND_REG_REG(Register *reg1, Register *reg2)
{
    printf("AND %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}
void XOR_REG_REG(Register *reg1, Register *reg2)
{
    printf("XOR %s %s\n", reg1->name, reg2->name);
    total_clock_cycle += 10;
}

void EXIT_INSTRUCTION(int exitcode)
{
    printf("EXIT %d\n", exitcode);
    total_clock_cycle += 20;
    if (DEBUG_MODE)
        printf("\ntotal clock cycle: %d\n", total_clock_cycle);
}