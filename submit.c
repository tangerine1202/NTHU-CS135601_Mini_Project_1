#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* --------------- declaration --------------- */

/* ----- define vars ----- */

#define MAXLEN 256
#define TBLSIZE 64
#define MAXREG 8

typedef enum
{
    MISPAREN,
    NOTNUMID,
    NOTFOUND,
    RUNOUT,
    FACROT_ORANDXOR,
    REG_RUNOUT,
    DIV_BY_ZERO,
    VAR_UNASSIGNED,
    CANT_GET_ADDR,
    UNEXPECT_TOKENTYPE,
    WRONG_ADDR,
    NULL_VALUE,
    NULL_NODE,
    NULL_REGISTER, // maybe deprecate
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
    int val;
    int unknown_val;
} Value;

typedef struct _Node
{
    char lexeme[MAXLEN];
    TokenSet token;
    int val;
    int weight;
    struct _Node *left, *right;
} BTNode;

typedef struct
{
    char name[MAXLEN];
    int val;
    int unknown_val;
    int assigned;
} Symbol;

typedef struct
{
    char name[3]; // r0, r1, r2...
    int val;
    int unknown_val;
    int occupied;
} Register;

int DEBUG_MODE;

TokenSet lookahead = UNKNOWN;
char lexeme[MAXLEN];
Symbol sbtable[TBLSIZE];
int sbcount = 0;
Register reg[MAXREG];

/* ----- fn: lexer_parser.h ----- */

int getValFromLexeme(void);
int setSbVal(char *, int);
BTNode *factor(void);
BTNode *term(void);
BTNode *term_tail(BTNode *);
BTNode *expr(void);
BTNode *expr_tail(BTNode *);
void statement(void);

TokenSet getToken(void);
char *getLexeme(void);
void advance(void);
int match(TokenSet);
BTNode *makeNode(TokenSet, const char *);

/* ----- fn: helpful.h ----- */

void evaluate(BTNode *);
int evaluateTree(BTNode *);
void printPrefix(BTNode *);
void printTree(BTNode *, int);
void freeTree(BTNode *);
void error(ErrorType);

int getAddr(char *);
char *getAddrName(int);
int getAddrVal(int);
int getAddrUnknownVal(int);
int getAddrAssigned(int);

int max(int, int);
int min(int, int);

void charswap(char *, char *);
char *reverse(char *, int, int);
char *itoa(int, char *, int);

/* ----- fn: semantizer.h ----- */

Value *semantize(BTNode *);
Value *makeValueNode(int val, int unknonw_val);
void updateNodeWeight(BTNode *);
void calculateValWithOp(Value *, Value *, char *);
void setAddrVal(int, Value *);

/* ----- fn: code_gen.h ----- */

void codeGenerate(BTNode *);
void initReg();
Register *generateAsmCode(BTNode *);
Register *getUnusedReg();
void setRegByInt(Register *, int);
void setRegByAddr(Register *, int);
void setRegByReg(Register *, Register *);
void setRegByRegWithOp(Register *, Register *, char *);
void setAddrByReg(int, Register *);
void returnReg(Register *);

// instruction
void MOV_REG_REG(Register *, Register *);
void MOV_REG_INT(Register *, int);
void MOV_REG_ADDR(Register *, int, char *, int, int);
void MOV_ADDR_REG(int, Register *, char *, int, int);

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
    // freopen("input.txt", "r", stdin);
    // freopen("output.txt", "w", stdout);

    // init sbtable as unassigned
    for (int i = 0; i < TBLSIZE; i++)
    {
        sbtable[i].unknown_val = 1;
        sbtable[i].assigned = 0;
    }

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
        sbtable[sbcount].unknown_val = 1;
        sbtable[sbcount].assigned = 1;
        sbcount++;
    }

    while (1)
    {
        statement();
    }
    return 0;
}

/* ----- lexer_parser.c ----- */

int getValFromLexeme(void)
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

int setSbVal(char *str, int val)
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

/* factor := INT | ID | ID ASSIGN expr | ADD_SUB INT | ADD_SUB ID | LPAREN expr RPAREN */
BTNode *factor(void)
{
    BTNode *retp = NULL;
    char tmpstr[MAXLEN];

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        retp->val = getValFromLexeme();
        advance();
    }
    else if (match(ID))
    {
        BTNode *left = makeNode(ID, getLexeme());
        left->val = getValFromLexeme();
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
            retp->right->val = getValFromLexeme();
            retp->left = makeNode(INT, "0");
            retp->left->val = 0;
            advance();
        }
        else if (match(LPAREN))
        {
            retp = makeNode(ADDSUB, tmpstr);
            advance();
            retp->right = expr();
            retp->left = makeNode(INT, "0");
            retp->left->val = 0;
            if (match(RPAREN))
                advance();
            else
                error(MISPAREN);
        }
        else
            error(NOTNUMID);
    }
    else if (match(ORANDXOR))
    {
        error(FACROT_ORANDXOR);
    }
    else if (match(LPAREN))
    {
        advance();
        retp = expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    }
    else
        error(NOTNUMID);
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
        /* TODO: warp into
            fn codeGenerator() 
        */

        // TODO: optimize
        // write x,y,z to r0,r1,r2
        for (int i = 0; i < 3; i++)
        {
            char c[2];
            c[0] = (char)('x' + i);
            c[1] = '\0';
            MOV_REG_ADDR(&(reg[i]), getAddr(c), c, getAddrVal(getAddr(c)), getAddrUnknownVal(getAddr(c)));
            sbcount++;
        }

        /* warp end */

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
char *getLexeme(void)
{
    return lexeme;
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

BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode *)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->token = tok;
    node->val = 0;
    node->weight = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* ----- helpful.c ----- */

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
void charswap(char *x, char *y)
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

/* ----- semantizer.c ----- */

Value *semantize(BTNode *root)
{
    Value *retval = NULL, *lval, *rval;
    int addr;
    if (root != NULL)
    {
        switch (root->token)
        {
        case ID:
            addr = getAddr(root->lexeme);
            if (!getAddrAssigned(addr))
                error(VAR_UNASSIGNED);

            updateNodeWeight(root);
            retval = makeValueNode(getAddrVal(addr), getAddrUnknownVal(addr));
            break;
        case INT:
            updateNodeWeight(root);
            retval = makeValueNode(root->val, 0);
            break;
        case ASSIGN:
            // Multiple assign is illegal.
            if (root->left->token == ID)
                addr = getAddr(root->left->lexeme);
            else
                error(CANT_GET_ADDR);

            rval = semantize(root->right);
            if (rval == NULL)
                error(NULL_VALUE);

            updateNodeWeight(root);

            setAddrVal(addr, rval);
            free(rval);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            if (root->left->weight > root->right->weight)
            {
                lval = semantize(root->left);
                rval = semantize(root->right);
            }
            else
            {
                rval = semantize(root->right);
                lval = semantize(root->left);
            }
            if (lval == NULL || rval == NULL)
                error(NULL_VALUE);

            updateNodeWeight(root);

            calculateValWithOp(lval, rval, root->lexeme);
            retval = lval;
            free(rval);
            break;
        default:
            error(UNEXPECT_TOKENTYPE);
            break;
        }
    }
    return retval;
}

Value *makeValueNode(int val, int unknonw_val)
{
    Value *retval = (Value *)malloc(sizeof(Value));
    retval->val = val;
    retval->unknown_val = unknonw_val;
    return retval;
}

void updateNodeWeight(BTNode *node)
{
    int lw, rw;
    if (node->left == NULL && node->right == NULL)
        node->weight = 0;
    else if (node->left == NULL || node->right == NULL)
        error(NULL_NODE);
    else if (node->left->weight == node->right->weight)
        node->weight = node->left->weight + 1;
    else
        node->weight = max(node->left->weight, node->right->weight);
}

void calculateValWithOp(Value *lval, Value *rval, char *op)
{
    if (rval->unknown_val == 0 && rval->val == 0)
        error(DIV_BY_ZERO);

    else if (lval->unknown_val || rval->unknown_val)
        lval->unknown_val = 1;
    else if (strcmp(op, "+") == 0)
        lval->val = lval->val + rval->val;
    else if (strcmp(op, "-") == 0)
        lval->val = lval->val - rval->val;
    else if (strcmp(op, "*") == 0)
        lval->val = lval->val * rval->val;
    else if (strcmp(op, "/") == 0)
        lval->val = lval->val / rval->val;
    else if (strcmp(op, "|") == 0)
        lval->val = lval->val | rval->val;
    else if (strcmp(op, "&") == 0)
        lval->val = lval->val & rval->val;
    else if (strcmp(op, "^") == 0)
        lval->val = lval->val ^ rval->val;
}

void setAddrVal(int addr, Value *valnode)
{
    int i = addr / 4;
    if (i < 0 || i >= sbcount)
        error(WRONG_ADDR);
    sbtable[i].val = valnode->val;
    sbtable[i].unknown_val = valnode->unknown_val;
    sbtable[i].assigned = 1;
}

/* ----- code_generator.c ----- */

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
            if (root->left->weight >= root->right->weight)
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

/* ----- instruction.c ----- */

void MOV_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MOV %s %s\n", reg1->name, reg2->name);
}
void MOV_REG_INT(Register *reg1, int val)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%d\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10), val);
    else
        fprintf(stdout, "MOV %s %d\n", reg1->name, val);
}
void MOV_REG_ADDR(Register *reg1, int addr2, char *addrname, int addrval, int addrunknown)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s(%s)\t%s[%d](%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               addrname, addr2, addrunknown ? "#" : itoa(addrval, buf, 10));
    else
        fprintf(stdout, "MOV %s [%d]\n", reg1->name, addr2);
}
void MOV_ADDR_REG(int addr1, Register *reg2, char *addrname, int addrval, int addrunknown)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MOV %s[%d](%s)\t%s(%s)\n",
               addrname, addr1, addrunknown ? "#" : itoa(addrval, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MOV [%d] %s\n", addr1, reg2->name);
}

void ADD_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("ADD %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "ADD %s %s\n", reg1->name, reg2->name);
}
void SUB_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("SUB %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "SUB %s %s\n", reg1->name, reg2->name);
}
void MUL_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("MUL %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "MUL %s %s\n", reg1->name, reg2->name);
}
void DIV_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("DIV %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "DIV %s %s\n", reg1->name, reg2->name);
}

void OR_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("OR %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "OR %s %s\n", reg1->name, reg2->name);
}
void AND_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("AND %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "AND %s %s\n", reg1->name, reg2->name);
}
void XOR_REG_REG(Register *reg1, Register *reg2)
{
    char buf[10];
    if (DEBUG_MODE)
        printf("XOR %s(%s)\t%s(%s)\n",
               reg1->name, reg1->unknown_val ? "#" : itoa(reg1->val, buf, 10),
               reg2->name, reg2->unknown_val ? "#" : itoa(reg2->val, buf, 10));
    else
        fprintf(stdout, "XOR %s %s\n", reg1->name, reg2->name);
}

void EXIT_INSTRUCTION(int exitcode)
{
    printf("EXIT %d\n", exitcode);
}