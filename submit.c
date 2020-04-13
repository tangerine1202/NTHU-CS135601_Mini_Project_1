#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* --------------- declaration --------------- */

/* ----- define vars ----- */
#define MAXLEN 256
#define TBLSIZE 64
#define MAXREG 8

/* ----- typedef ----- */

typedef enum
{
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    DEBUG_FACROT_ORANDXOR,
    REG_RUNOUT,
    DIV_BY_ZERO,
    VAR_UNASSIGNED,
} ErrorType;

typedef enum
{
    UNKNOWN,
    END,
    INT,
    ID,
    ORANDXOR,
    ADDSUB,
    MULDIV,
    ASSIGN,
    LPAREN,
    RPAREN,
    ENDFILE
} TokenSet;

typedef struct
{
    char name[MAXLEN];
    int val;
    int assigned;
} Symbol;

typedef struct _Node
{
    char lexeme[MAXLEN];
    TokenSet data;
    int val;
    struct _Node *left, *right;
} BTNode;

typedef struct
{
    int id;
    int val;
    int used;
    char name[3]; // r0, r1, r2...
} Register;

/* ----- global vars -----*/
int DEBUG_MODE = 0;
TokenSet lookahead = UNKNOWN;
char lexeme[MAXLEN];
Symbol sbtable[TBLSIZE];
int sbcount = 0;
Register reg[MAXREG];

/* ----- fn: lexer_parser.h ----- */

BTNode *factor(void);
BTNode *term(void);
BTNode *term_tail(BTNode *);
BTNode *expr(void);
BTNode *expr_tail(BTNode *);
void statement(void);
char *getLexeme(void);
TokenSet getToken(void);
void advance(void);
int match(TokenSet);
BTNode *makeNode(TokenSet, const char *);
int getval(void);
int setval(char *, int);

/* ----- fn: helpful.h ----- */

void evaluate(BTNode *);
int evaluateTree(BTNode *);
void printPrefix(BTNode *);
void freeTree(BTNode *);
void error(ErrorType);

/* -----fn: code_gen.h ----- */

void codeGenerate(BTNode *);
void initReg();
Register *generateAsmCode(BTNode *);
Register *getUnusedReg();
void returnReg(Register *);
int getAddr(char *);
int getAddrVal(int);
int getAddrAssigned(int);
void setAddrAssigned(int);

// instruction
void MOV_REG_REG(Register *, Register *);
void MOV_REG_INT(Register *, int);
void MOV_REG_ADDR(Register *, int, char *, int);
void MOV_ADDR_REG(int, Register *, char *, int);

void ADD_REG_REG(Register *, Register *);
void SUB_REG_REG(Register *, Register *);
void MUL_REG_REG(Register *, Register *);
void DIV_REG_REG(Register *, Register *);

void AND_REG_REG(Register *, Register *);
void OR_REG_REG(Register *, Register *);
void XOR_REG_REG(Register *, Register *);

void EXIT_INSTRUCTION(int);

/* --------------- end declaration ---------------*/

/* --------------- definition ---------------- */

/* ----- solve.c -----*/

int main(void)
{
    // if (!DEBUG_MODE)
    // {
    //     freopen("sb_input.txt", "r", stdin);
    //     freopen("sb_output.txt", "w", stdout);
    // }

    // init sbtable as unassigned
    for (int i = 0; i < TBLSIZE; i++)
        sbtable[i].assigned = 0;

    // initReg
    initReg();

    // init xyz
    for (int i = 0; i < 3; i++)
    {
        char c[2];
        c[0] = (char)('x' + i);
        c[1] = '\0';
        strcpy(sbtable[sbcount].name, c);
        sbtable[sbcount].val = 0;
        sbtable[sbcount].assigned = 1;
        // TODO: optimize
        // read x,y,z first
        MOV_REG_ADDR(getUnusedReg(), sbcount * 4, c, 0);
        sbcount++;
    }

    while (1)
    {
        statement();
    }
    return 0;
}

/* ----- lexer_parser.c ----- */

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

/* ----- helpful.c ----- */

void evaluate(BTNode *root)
{
    if (DEBUG_MODE)
    {
        printf("\n%d\n", evaluateTree(root));
        printPrefix(root);
        printf("\n");
        printf("\n");
    }
    if (DEBUG_MODE)
    {
        printf("Code Generate:\n");
        codeGenerate(root);
        printf("\n");
        printf("-------------------\n");
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
    if (DEBUG_MODE)
    {
        switch (errorNum)
        {
        case MISPAREN:
            printf("Mismatched parenthesis\n");
            break;
        case NOTNUMID:
            printf("Number or identifier expected\n");
            break;
        case NOTFOUND:
            printf("%s not defined\n", getLexeme());
            break;
        case RUNOUT:
            printf("Out of memory\n");
            break;
        case DEBUG_FACROT_ORANDXOR:
            printf("or/and/xor exists in factor\n");
            break;
        case REG_RUNOUT:
            printf("Run out of register\n");
            break;
        case DIV_BY_ZERO:
            printf("Divided by zero\n");
            break;
        case VAR_UNASSIGNED:
            printf("Value of Var unassigned\n");
            break;
        }
    }
    EXIT_INSTRUCTION(1);
    exit(0);
}

/* ----- code_generator.c ----- */

void codeGenerate(BTNode *root)
{
    initReg();
    generateAsmCode(root);
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
            retreg->val = getAddrVal(addr);
            retreg->used = 1;
            break;
        // TODO: improve `code generate` oper int int condition
        case INT:
            retreg = getUnusedReg();
            MOV_REG_INT(retreg, root->val);
            retreg->val = root->val;
            retreg->used = 1;
            break;
        case ASSIGN:
            addr = getAddr(root->left->lexeme);
            rreg = generateAsmCode(root->right);
            MOV_ADDR_REG(addr, rreg, sbtable[addr / 4].name, getAddrVal(addr));
            setAddrAssigned(addr);
            returnReg(rreg);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            // note: reg useage depend on right/left rercursion
            rreg = generateAsmCode(root->right);
            lreg = generateAsmCode(root->left);
            if (strcmp(root->lexeme, "+") == 0)
                ADD_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "-") == 0)
                SUB_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "*") == 0)
                MUL_REG_REG(lreg, rreg);
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (rreg->val == 0)
                    error(DIV_BY_ZERO);
                else
                    DIV_REG_REG(lreg, rreg);
            }
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
        error(REG_RUNOUT);
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

int getAddrAssigned(int addr)
{
    return sbtable[addr / 4].assigned;
}

void setAddrAssigned(int addr)
{
    sbtable[addr / 4].assigned = 1;
}

/* ----- instruction.c ----- */

void MOV_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("MOV %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("MOV %s %s\n", reg1->name, reg2->name);
}
void MOV_REG_INT(Register *reg1, int val)
{
    if (DEBUG_MODE)
        printf("MOV %s(%d)\t%d\n", reg1->name, reg1->val, val);
    else
        printf("MOV %s %d\n", reg1->name, val);
}
void MOV_REG_ADDR(Register *reg1, int addr2, char *addrname, int addrval)
{
    if (DEBUG_MODE)
        printf("MOV %s(%d)\t%s[%d](%d)\n", reg1->name, reg1->val, addrname, addr2, addrval);
    else
        printf("MOV %s [%d]\n", reg1->name, addr2);
}
void MOV_ADDR_REG(int addr1, Register *reg2, char *addrname, int addrval)
{
    if (DEBUG_MODE)
        printf("MOV %s[%d](%d)\t%s(%d)\n", addrname, addr1, addrval, reg2->name, reg2->val);
    else
        printf("MOV [%d] %s\n", addr1, reg2->name);
}

void ADD_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("ADD %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("ADD %s %s\n", reg1->name, reg2->name);
}
void SUB_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("SUB %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("SUB %s %s\n", reg1->name, reg2->name);
}
void MUL_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("MUL %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("MUL %s %s\n", reg1->name, reg2->name);
}
void DIV_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("DIV %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("DIV %s %s\n", reg1->name, reg2->name);
}

void OR_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("OR %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("OR %s %s\n", reg1->name, reg2->name);
}
void AND_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("AND %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("AND %s %s\n", reg1->name, reg2->name);
}
void XOR_REG_REG(Register *reg1, Register *reg2)
{
    if (DEBUG_MODE)
        printf("XOR %s(%d)\t%s(%d)\n", reg1->name, reg1->val, reg2->name, reg2->val);
    else
        printf("XOR %s %s\n", reg1->name, reg2->name);
}

void EXIT_INSTRUCTION(int exitcode)
{
    printf("EXIT %d\n", exitcode);
}
