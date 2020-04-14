#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

Register reg[MAXREG];

void codeGenerate(BTNode *root)
{
    // TODO: optimize by cache reg (don't need to initReg everytime)
    initReg();
    generateAsmCode(root);
}

void initReg()
{
    char *regname;
    for (int i = 0; i < MAXREG; i++)
    {
        reg[i].val = 0;
        reg[i].unknown_val = 1;
        reg[i].occupied = 0;
        reg[i].name[0] = 'r';
        reg[i].name[1] = '0' + i;
        reg[i].name[2] = '\0';
    }
}

Register *generateAsmCode(BTNode *root)
{
    Register *retreg = NULL, *lreg, *rreg;
    int addr;
    if (root != NULL)
    {
        switch (root->token)
        {
        case ID:
            addr = getAddr(root->lexeme);
            if (!getAddrAssigned(addr))
                error(VAR_UNASSIGNED);
            retreg = getUnusedReg();
            MOV_REG_ADDR(retreg, addr, getAddrName(addr), getAddrVal(addr), getAddrUnknownVal(addr));
            setRegByAddr(retreg, addr);
            break;
        // TODO: improve `code generate` oper int int condition
        case INT:
            retreg = getUnusedReg();
            MOV_REG_INT(retreg, root->val);
            setRegByInt(retreg, root->val);
            break;
        case ASSIGN:
            // Multiple assign is illegal.
            addr = getAddr(root->left->lexeme);
            rreg = generateAsmCode(root->right);
            if (rreg == NULL)
                error(NULL_REGISTER);

            MOV_ADDR_REG(addr, rreg, getAddrName(addr), getAddrVal(addr), getAddrUnknownVal(addr));
            setAddrByReg(addr, rreg);
            returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            lreg = generateAsmCode(root->left);
            rreg = generateAsmCode(root->right);
            if (strcmp(root->lexeme, "+") == 0)
                ADD_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "-") == 0)
                SUB_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "*") == 0)
                MUL_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "/") == 0)
                DIV_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "|") == 0)
                OR_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "&") == 0)
                AND_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "^") == 0)
                XOR_REG_REG(lreg, rreg);

            setRegByRegWithOp(lreg, rreg, root->lexeme);
            retreg = lreg;
            returnReg(rreg);
            break;
        default:
            error(UNEXPECT_TOKENTYPE);
            break;
        }
    }
    return retreg;
}

Register *getUnusedReg()
{
    Register *retreg = NULL;
    for (int i = 0; i < MAXREG; i++)
    {
        if (!(reg[i].occupied))
        {
            retreg = &(reg[i]);
            break;
        }
    }
    if (retreg == NULL)
        error(REG_RUNOUT);
    return retreg;
}

void setRegByInt(Register *reg, int val)
{
    reg->val = val;
    reg->unknown_val = 0;
    reg->occupied = 1;
}

void setRegByAddr(Register *reg, int addr)
{
    Symbol *sb = &(sbtable[addr / 4]);
    reg->val = sb->val;
    reg->unknown_val = sb->unknown_val;
    reg->occupied = 1;
}

void setRegByReg(Register *reg1, Register *reg2)
{
    reg1->val = reg2->val;
    reg1->unknown_val = reg2->unknown_val;
    reg1->occupied = 1;
}

void setRegByRegWithOp(Register *reg1, Register *reg2, char *op)
{
    if (reg1->unknown_val || reg2->unknown_val)
        reg1->unknown_val = 1;
    else if (strcmp(op, "+") == 0)
        reg1->val = reg1->val + reg2->val;
    else if (strcmp(op, "-") == 0)
        reg1->val = reg1->val - reg2->val;
    else if (strcmp(op, "*") == 0)
        reg1->val = reg1->val * reg2->val;
    else if (strcmp(op, "/") == 0)
        reg1->val = reg1->val / reg2->val;
    else if (strcmp(op, "|") == 0)
        reg1->val = reg1->val | reg2->val;
    else if (strcmp(op, "&") == 0)
        reg1->val = reg1->val & reg2->val;
    else if (strcmp(op, "^") == 0)
        reg1->val = reg1->val ^ reg2->val;
    reg->occupied = 1;
}

void setAddrByReg(int addr, Register *reg)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    sbtable[i].val = reg->val;
    sbtable[i].unknown_val = reg->unknown_val;
    sbtable[i].assigned = 1;
}

void returnReg(Register *reg)
{
    reg->val = 0;
    reg->unknown_val = 1;
    reg->occupied = 0;
}
