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
    if (root->token == ASSIGN)
    {
        generateCode(root);
    }
    else if (DEBUG_MODE)
        printf("Drop statement, since it's not assignment statement\n");
}

void initReg()
{
    char *regname;
    for (int i = 0; i < MAXREG; i++)
    {
        reg[i].occupied = 0;
        reg[i].name[0] = 'r';
        reg[i].name[1] = '0' + i;
        reg[i].name[2] = '\0';
    }
}

Register *generateCode(BTNode *root)
{
    Register *retreg = NULL, *lreg, *rreg;
    int addr;
    if (root != NULL)
    {
        switch (root->token)
        {
        case ID:
            addr = getAddr(root->lexeme);
            retreg = getUnusedReg();
            MOV_REG_ADDR(retreg, addr);
            break;
        case INT:
            retreg = getUnusedReg();
            MOV_REG_INT(retreg, root->val);
            break;
        case ASSIGN:
            addr = getAddr(root->left->lexeme);
            rreg = generateCode(root->right);
            if (rreg == NULL)
                error(NULL_REGISTER);

            MOV_ADDR_REG(addr, rreg);
            returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            if (root->left->weight >= root->right->weight)
            {
                lreg = generateCode(root->left);
                rreg = generateCode(root->right);
            }
            else
            {
                rreg = generateCode(root->right);
                lreg = generateCode(root->left);
            }
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

            returnReg(rreg);
            retreg = lreg;
            break;

        default:
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
            retreg->occupied = 1;
            break;
        }
    }
    if (retreg == NULL)
        error(REG_RUNOUT);
    return retreg;
}

void returnReg(Register *reg)
{
    reg->occupied = 0;
}
