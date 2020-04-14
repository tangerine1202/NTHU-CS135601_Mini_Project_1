/* 
### Reg Cache
- add lexeme
- add eraseable
- find cache
    - [1] used cache
    - [0] find unused reg
        - [0] erase eraseable reg
*/

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
        reg[i].used = 0;
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
        switch (root->data)
        {
        case ID:
            addr = getAddr(root->lexeme);
            if (!getAddrAssigned(addr))
                error(VAR_UNASSIGNED);
            retreg = getUnusedReg();
            MOV_REG_ADDR(retreg, addr, sbtable[addr / 4].name, getAddrVal(addr));
            setReg(retreg, getAddrVal(addr));
            break;
        // TODO: improve `code generate` oper int int condition
        case INT:
            retreg = getUnusedReg();
            MOV_REG_INT(retreg, root->val);
            setReg(retreg, root->val);
            break;
        case ASSIGN:
            addr = getAddr(root->left->lexeme); //FIXME: will `a + 5 = 12` exist in testcase ?
            rreg = generateAsmCode(root->right);
            MOV_ADDR_REG(addr, rreg, sbtable[addr / 4].name, getAddrVal(addr));
            setAddr(addr, rreg->val);
            retreg = rreg;
            // returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            // note: reg useage depend on left/right rercursion
            if (root->left->weight > root->right->weight)
            {
                lreg = generateAsmCode(root->left);
                rreg = generateAsmCode(root->right);
            }
            else
            {
                rreg = generateAsmCode(root->right);
                lreg = generateAsmCode(root->left);
            }
            if (strcmp(root->lexeme, "+") == 0)
            {
                ADD_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val + rreg->val);
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                SUB_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val - rreg->val);
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                MUL_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val * rreg->val);
            }
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (rreg->val == 0)
                    error(DIV_BY_ZERO);
                else
                {
                    DIV_REG_REG(lreg, rreg);
                    setReg(lreg, lreg->val / rreg->val);
                }
            }
            else if (strcmp(root->lexeme, "|") == 0)
            {
                OR_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val | rreg->val);
            }
            else if (strcmp(root->lexeme, "&") == 0)
            {
                AND_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val & rreg->val);
            }
            else if (strcmp(root->lexeme, "^") == 0)
            {
                XOR_REG_REG(lreg, rreg);
                setReg(lreg, lreg->val ^ rreg->val);
            }
            returnReg(rreg);
            retreg = lreg;
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
        if (!(reg[i].used))
        {
            retreg = &(reg[i]);
            break;
        }
    }
    if (retreg == NULL)
        error(REG_RUNOUT);
    return retreg;
}

void setReg(Register *reg, int val)
{
    reg->val = val;
    reg->used = 1;
}

void returnReg(Register *reg)
{
    reg->val = 0;
    reg->used = 0;
}

int getAddr(char *str)
{
    int i = 0, retaddr = 0;
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
    if (i >= sbcount)
        error(CANT_GET_ADDR);
    return retaddr;
}

int getAddrVal(int addr)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    return sbtable[i].val;
}

int getAddrAssigned(int addr)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    return sbtable[i].assigned;
}

void setAddr(int addr, int val)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    sbtable[i].val = val;
    sbtable[i].assigned = 1;
}