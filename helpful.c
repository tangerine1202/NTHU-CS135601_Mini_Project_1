#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

void evaluate(BTNode *root)
{
    if (DEBUG_MODE)
    {
        printf("-------------------\n");
        printf("%d\n", evaluateTree(root));
        printPrefix(root);
        printf("\n");
        printf("\n");
    }
    if (DEBUG_MODE)
    {
        printf("Code Generate:\n");
        codeGenerate(root);
        printf("\n");
    }
    else
    {
        codeGenerate(root);
    }

    freeTree(root);
}

int evaluateTree(BTNode *root)
{
    int retval = 0, lv, rv;
    if (root != NULL)
    {
        switch (root->data)
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
                retval = setval(root->left->lexeme, rv);
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
    case DEBUG_FACROT_ORANDXOR:
        fprintf(stderr, "or/and/xor exists in factor\n");
        break;
    case REG_RUNOUT:
        fprintf(stderr, "Run out of register\n");
        break;
    case DIV_BY_ZERO:
        fprintf(stderr, "Divided by zero\n");
        break;
    case VAR_UNASSIGNED:
        fprintf(stderr, "Value of Var unassigned\n");
        break;
    }
    exit(0);
}