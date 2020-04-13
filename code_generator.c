#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

Register reg[MAXREG];

void codeGenerate(BTNode *root, int debug)
{
    initReg();
    generateAsmCode(root, debug);
}

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

Register *generateAsmCode(BTNode *root, int debug)
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
            MOV_REG_ADDR(retreg, addr, sbtable[addr / 4].name, getAddrVal(addr), debug);
            retreg->val = getAddrVal(addr);
            retreg->used = 1;
            break;
        // TODO: improve `code generate` oper int int condition
        case INT:
            retreg = getUnusedReg();
            MOV_REG_INT(retreg, root->val, debug);
            retreg->val = root->val;
            retreg->used = 1;
            break;
        case ASSIGN:
            rreg = generateAsmCode(root->right, debug);
            addr = getAddr(root->left->lexeme);
            MOV_ADDR_REG(addr, rreg, sbtable[addr / 4].name, getAddrVal(addr), debug);
            returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            // note: reg useage depend on right/left rercursion
            rreg = generateAsmCode(root->right, debug);
            lreg = generateAsmCode(root->left, debug);
            if (strcmp(root->lexeme, "+") == 0)
                ADD_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "-") == 0)
                SUB_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "*") == 0)
                MUL_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "/") == 0)
                DIV_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "|") == 0)
                OR_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "&") == 0)
                AND_REG_REG(lreg, rreg, debug);
            else if (strcmp(root->lexeme, "^") == 0)
                XOR_REG_REG(lreg, rreg, debug);
            returnReg(rreg);
            retreg = lreg;
            break;
        default:
            retreg = NULL;
        }
    }
    return retreg;
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
