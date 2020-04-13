#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "header.h"
#include "code_gen.h"

Register reg[MAXREG];

void initReg()
{
    char *regname;
    for (int i = 0; i < MAXREG; i++)
    {
        reg[i].id = i;
        reg[i].val = 0;
        reg[i].used = 0;
        reg[i].name[0] = 'r';
        reg[i].name[1] = '0' + i;
        reg[i].name[2] = '\0';
    }
}

Register *getUnusedReg()
{
    Register *retreg = NULL;
    for (int i = 0; i < MAXREG; i++)
    {
        if (!reg[i].used)
        {
            retreg = &(reg[i]);
            break;
        }
    }
    if (retreg == NULL)
        codeGenError(REG_RUNOUT);
    return retreg;
}

void returnReg(Register *reg)
{
    reg->val = 0;
    reg->used = 0;
}

int getAddr(char *str)
{
    int i = 0, retaddr;
    while (i < sbcount)
    {
        if (strcmp(str, sbtable[i].name) == 0)
        {
            retaddr = i * 4;
            break;
        }
        else
            i++;
    }
    return retaddr;
}

int getAddrVal(int addr)
{
    return sbtable[addr / 4].val;
}

Register *codeGenerate(BTNode *root)
{
    Register *retreg = NULL, *lreg, *rreg;
    int addr;
    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:
            addr = getAddr(root->lexeme);
            retreg = getUnusedReg();
            debug_MOV_REG_ADDR(retreg, addr, sbtable[addr / 4].name, getAddrVal(addr));
            retreg->val = getAddrVal(addr);
            retreg->used = 1;
            break;
        // TODO: improve `code generate` oper int int condition
        case INT:
            retreg = getUnusedReg();
            debug_MOV_REG_INT(retreg, root->val);
            retreg->val = root->val;
            retreg->used = 1;
            break;
        case ASSIGN:
            rreg = codeGenerate(root->right);
            addr = getAddr(root->left->lexeme);
            debug_MOV_ADDR_REG(addr, rreg, sbtable[addr / 4].name, getAddrVal(addr));
            returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            rreg = codeGenerate(root->right); // ALL: do right recursion first can save reg useage
            lreg = codeGenerate(root->left);
            if (strcmp(root->lexeme, "+") == 0)
                debug_ADD_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "-") == 0)
                debug_SUB_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "*") == 0)
                debug_MUL_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "/") == 0)
                debug_DIV_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "|") == 0)
                debug_OR_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "&") == 0)
                debug_AND_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "^") == 0)
                debug_XOR_REG_REG(lreg, rreg);
            returnReg(rreg);
            retreg = lreg;
            break;
        default:
            retreg = NULL;
        }
    }
    return retreg;
}

void codeGenError(CodeGen_ErrorType errorNum)
{
    switch (errorNum)
    {
    case REG_RUNOUT:
        fprintf(stderr, "Run out of register\n");
        break;
    }
    exit(0);
}
