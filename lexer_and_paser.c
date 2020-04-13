#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

TokenSet lookahead = UNKNOWN;
char lexeme[MAXLEN];
Symbol sbtable[TBLSIZE];
int sbcount = 0;

int getval(void)
{
    int i, retval, found;

    if (match(INT))
        retval = atoi(getLexeme());
    else if (match(ID))
    {
        i = 0;
        found = 0;
        retval = 0;
        while (i < sbcount && !found)
        {
            if (strcmp(getLexeme(), sbtable[i].name) == 0)
            {
                retval = sbtable[i].val;
                found = 1;
                break;
            }
            else
                i++;
        }
        if (!found)
        {
            if (sbcount < TBLSIZE)
            {
                strcpy(sbtable[sbcount].name, getLexeme());
                sbtable[sbcount].val = 0;
                sbcount++;
            }
            else
                error(RUNOUT);
        }
    }
    return retval;
}

int setval(char *str, int val)
{
    int i, retval;
    i = 0;
    while (i < sbcount)
    {
        if (strcmp(str, sbtable[i].name) == 0)
        {
            sbtable[i].val = val;
            retval = val;
            break;
        }
        else
            i++;
    }
    return retval;
}

/* create a node without any child.*/
BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode *)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

TokenSet getToken(void)
{
    /*
    - set `lexeme[]`
    - return Token
    */
    int i;
    char c;

    while ((c = fgetc(stdin)) == ' ' || c == '\t')
        ; // 忽略空白字元

    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+' || c == '-')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    }
    else if (c == '|' || c == '&' || c == '^')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ORANDXOR;
    }
    else if (c == '*' || c == '/')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n')
    {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=')
    {
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(')
    {
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')')
    {
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_')
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_')
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    }
    else if (c == EOF)
    {
        return ENDFILE;
    }
    else
    {
        return UNKNOWN;
    }
}

void advance(void)
{
    lookahead = getToken();
}

int match(TokenSet token)
{
    if (lookahead == UNKNOWN)
        advance();
    return token == lookahead;
}

char *getLexeme(void)
{
    return lexeme;
}

/* factor := INT | ID | ID ASSIGN expr | ADD_SUB INT | ADD_SUB ID | LPAREN expr RPAREN */
BTNode *factor(void)
{
    BTNode *retp = NULL;
    char tmpstr[MAXLEN];

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        retp->val = getval();
        advance();
    }
    else if (match(ID))
    {
        BTNode *left = makeNode(ID, getLexeme());
        left->val = getval();
        strcpy(tmpstr, getLexeme()); // FIXME: why copy lexeme of ID variable ?
        advance();
        if (match(ASSIGN))
        {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
        else
        {
            retp = left;
        }
    }
    else if (match(ADDSUB))
    {
        strcpy(tmpstr, getLexeme());
        advance();
        if (match(ID) || match(INT))
        {
            retp = makeNode(ADDSUB, tmpstr);
            if (match(ID))
                retp->right = makeNode(ID, getLexeme());
            else
                retp->right = makeNode(INT, getLexeme());
            retp->right->val = getval();
            retp->left = makeNode(INT, "0");
            retp->left->val = 0;
            advance();
        }
        else
        {
            error(NOTNUMID);
        }
    }
    else if (match(ORANDXOR)) //TODO: Does this condition exist?
    {
        error(DEBUG_FACROT_ORANDXOR);
    }
    else if (match(LPAREN))
    {
        advance();
        retp = expr();
        if (match(RPAREN))
        {
            advance();
        }
        else
        {
            error(MISPAREN);
        }
    }
    else
    {
        error(NOTNUMID);
    }
    return retp;
}

/* term := factor term_tail */
BTNode *term(void)
{
    BTNode *node;

    node = factor();

    return term_tail(node);
}

/* term_tail := MUL_DIV factor term_tail|NiL */
BTNode *term_tail(BTNode *left)
{
    BTNode *node;

    if (match(MULDIV))
    {
        node = makeNode(MULDIV, getLexeme());
        advance();

        node->left = left;
        node->right = factor();

        return term_tail(node);
    }
    else
        return left;
}

/* expr := term expr_tail */
BTNode *expr(void)
{
    BTNode *node;

    node = term();

    return expr_tail(node);
}

/* expr_tail := ADD_SUB_AND_OR_XOR term expr_tail | NiL */
BTNode *expr_tail(BTNode *left)
{
    BTNode *node;

    if (match(ADDSUB))
    {
        node = makeNode(ADDSUB, getLexeme());
        advance();

        node->left = left;
        node->right = term();

        return expr_tail(node);
    }
    else if (match(ORANDXOR))
    {
        node = makeNode(ORANDXOR, getLexeme());
        advance();

        node->left = left;
        node->right = term();

        return expr_tail(node);
    }
    else
        return left;
}

/* statement := END | expr END */
void statement(void)
{
    BTNode *retp;

    if (match(ENDFILE))
    {
        // TODO: optimize
        // write x,y,z to r0,r1,r2
        for (int i = 0; i < 3; i++)
        {
            char c[2];
            c[0] = (char)('x' + i);
            c[1] = '\0';
            MOV_REG_ADDR(&(reg[i]), getAddr(c), c, getAddrVal(getAddr(c)));
            sbcount++;
        }

        EXIT_INSTRUCTION(0);
        exit(0);
    }
    else if (match(END))
    {

        advance();
    }
    else
    {
        retp = expr();
        if (match(END))
        {
            evaluate(retp);

            advance();
        }
    }
}
