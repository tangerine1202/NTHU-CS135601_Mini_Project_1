#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "semantizer.h"
#include "code_gen.h"
#include "helpful.h"

int DEBUG_MODE = 1;
int reg_needed;
int last_clock_cycle = 0;

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
            printf("null\n");
        else
            printf("%s\n",
                   semantic_val->unknown_val ? "#" : itoa(semantic_val->val, buf, 10));

        reg_needed = root->weight;
        printf("Reg needed: %d\n", reg_needed);
        printf("\n");

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

        // clock cycle
        printf("total clock cycle: %d\n", total_clock_cycle - last_clock_cycle);
        last_clock_cycle = total_clock_cycle;

        printf("-------------------\n");
    }

    else
    {
        semantic_val = semantize(root);

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
        case NULL_NODE:
            printf("Read null Node\n");
            break;
        case LEFT_SIDE_OF_ASSIGN_IS_NOT_ID:
            printf("Left hand side of assign is not ID\n");
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

int max(int a, int b)
{
    return a > b ? a : b;
}
int min(int a, int b)
{
    return a < b ? a : b;
}

// inline function to swap two numbers
inline void charswap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

// function to reverse buffer[i..j]
char *reverse(char *buffer, int i, int j)
{
    while (i < j)
        charswap(&buffer[i++], &buffer[j--]);

    return buffer;
}

// Iterative function to implement itoa() function in C
char *itoa(int value, char *buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32)
        return buffer;

    // consider absolute value of number
    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
            buffer[i++] = 65 + (r - 10);
        else
            buffer[i++] = 48 + r;

        n = n / base;
    }

    // if number is 0
    if (i == 0)
        buffer[i++] = '0';

    // If base is 10 and value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10)
        buffer[i++] = '-';

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}