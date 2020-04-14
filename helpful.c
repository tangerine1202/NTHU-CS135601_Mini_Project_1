#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "semantizer.h"
#include "code_gen.h"
#include "helpful.h"

int DEBUG_MODE = 1;

void evaluate(BTNode *root)
{
    Value *semantic_val = NULL;

    if (DEBUG_MODE)
    {
        // Value of statement ('#' for unknown, 'null' for assign)
        printf("\nVal: ");
        char buf[10];
        semantic_val = semantize(root);
        if (semantic_val == NULL)
            printf("null");
        else
            printf("%s",
                   semantic_val->unknown_val ? "#" : itoa(semantic_val->val, buf, 10));

        printf("\n\n");

        // Statement Prefix representation
        printf("Prefix representation:\n");
        printPrefix(root);
        printf("\n\n");

        // Tree view
        printf("Tree view:\n");
        printTree(root, 0);
        printf("\n");

        // TODO: accelerator
        // accelerator(root);

        // Asm Code
        codeGenerate(root);
        printf("\n");
        printf("-------------------\n");
    }

    else
    {
        /* 
            fn sematizer(BTNode *) 
            # Basic
            - check assigned
            - check multiple assign
            - check div_by_zero (calculate value)
            - combine evaluate() features
                - debuging output 
        */

        semantic_val = semantize(root);

        /* TODO: 
            fn codeGenerator(BTNode *)
            # Basic
            - doesn't calculate value
            - drop expr without '='
            - add weight priority features
            
            ## Key Idea
            - left hand side is x,y,z
                - generate code and store in r0,r1,r2
                - store rootNode
            -left hand side is other vars
                - store rootNode in Symbol
            
            - 

            # Implement 
            - r0, r1, r2 for x,y,z
            - r3~r6 (at least four register) dynamic register
            - r7 unused / dynamic register

            # Advance 
            - count Symbol used times, and calculate useful cache

            # Deprecated idea
            ## save nodeTree in Symbol
                - left hand side is x,y,z
                    two choose:
                    1. 
                        - generate code immediately (WRONG!)
                            1. x = y+z
                            4. a = x+12 (suspend)
                            2. x = 3
                            3. y = a + x (or x + a)
                        - store assign->right
                        - free `assign` and `assign->left` node 

                    2. symbol->node = assignNode
                
                - left hand side is other vars
                    - store `assign->right` and free `assign` and `assign->left` node
        */

        codeGenerate(root);
    }
    if (semantic_val != NULL)
        free(semantic_val);
    freeTree(root);
}

int evaluateTree(BTNode *root)
{
    int retval = 0, lv, rv;
    if (root != NULL)
    {
        switch (root->token)
        {
        case ID:
        case INT:
            retval = root->val;
            break;

        case ASSIGN:
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            if (strcmp(root->lexeme, "+") == 0)
                retval = lv + rv;
            else if (strcmp(root->lexeme, "-") == 0)
                retval = lv - rv;
            else if (strcmp(root->lexeme, "*") == 0)
                retval = lv * rv;
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (rv == 0)
                    error(DIV_BY_ZERO);
                else
                    retval = lv / rv;
            }
            else if (strcmp(root->lexeme, "|") == 0)
                retval = lv | rv;
            else if (strcmp(root->lexeme, "&") == 0)
                retval = lv & rv;
            else if (strcmp(root->lexeme, "^") == 0)
                retval = lv ^ rv;
            else if (strcmp(root->lexeme, "=") == 0)
                retval = setSbVal(root->left->lexeme, rv);
            break;
        default:
            retval = 0;
        }
    }
    return retval;
}

/* print a tree by pre-order. */
void printPrefix(BTNode *root)
{
    if (root != NULL)
    {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}

void printTree(BTNode *root, int level)
{
    if (root != NULL)
    {
        for (int i = 0; i < level; i++)
            printf("  ");
        switch (root->token)
        {
        case UNKNOWN:
            printf("[UNKNOWN]");
            break;
        case END:
            printf("[END]");
            break;
        case INT:
            printf("[INT]");
            break;
        case ID:
            printf("[ID]");
            break;
        case ORANDXOR:
            printf("[ORANDXOR]");
            break;
        case ADDSUB:
            printf("[ADDSUB]");
            break;
        case MULDIV:
            printf("[MULDIV]");
            break;
        case ASSIGN:
            printf("[ASSIGN]");
            break;
        case LPAREN:
            printf("[LPAREN]");
            break;
        case RPAREN:
            printf("[RPAREN]");
            break;
        case ENDFILE:
            printf("[ENDFILE]");
            break;
        }
        printf("%s\n", root->lexeme);
        printTree(root->left, level + 1);
        printTree(root->right, level + 1);
    }
}

/* clean a tree.*/
void freeTree(BTNode *root)
{
    if (root != NULL)
    {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

void error(ErrorType errorNum)
{
    if (DEBUG_MODE)
    {
        switch (errorNum)
        {
        case MISPAREN:
            fprintf(stderr, "Mismatched parenthesis\n");
            break;
        case NOTNUMID:
            fprintf(stderr, "Number or identifier expected\n");
            break;
        case NOTFOUND:
            fprintf(stderr, "%s not defined\n", getLexeme());
            break;
        case RUNOUT:
            fprintf(stderr, "Out of memory\n");
            break;
        case FACROT_ORANDXOR:
            fprintf(stderr, "or/and/xor exists in factor\n");
            break;
        case REG_RUNOUT:
            fprintf(stderr, "Run out of register\n");
            break;
        case DIV_BY_ZERO:
            fprintf(stderr, "Divided by zero\n");
            break;
        case VAR_UNASSIGNED:
            printf("Variable unassigned\n");
            break;
        case CANT_GET_ADDR:
            printf("Can't get id addr from name\n");
            break;
        case UNEXPECT_TOKENTYPE:
            printf("Unexpect TokenType\n");
            break;
        case WRONG_ADDR:
            printf("Wrong Address\n");
            break;
        case NULL_REGISTER: // maybe deprecated
            printf("Null register\n");
            break;
        case NULL_VALUE:
            printf("Calculate with NULL value\n");
            break;
        }
    }
    EXIT_INSTRUCTION(1);
    exit(0);
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

char *getAddrName(int addr)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    return sbtable[i].name;
}

int getAddrUnknownVal(int addr)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    return sbtable[i].unknown_val;
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
