#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "semantizer.h"
#include "code_gen.h"
#include "helpful.h"

/* 
    fn sematizer(BTNode *) 
    # Basic
    - check assigned
    - check multiple assign
    - check div_by_zero (calculate value)
    - combine evaluate() features
        - debuging output 
*/

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
                error(LEFT_SIDE_OF_ASSIGN_IS_NOT_ID);

            rval = semantize(root->right);
            if (rval == NULL)
                error(NULL_VALUE);

            setAddrVal(addr, rval);

            updateNodeWeight(root);
            free(rval);
            break;
        case ADDSUB:
        case ORANDXOR:
        case MULDIV:
            lval = semantize(root->left);
            rval = semantize(root->right);
            if (lval == NULL || rval == NULL)
                error(NULL_VALUE);

            calculateValWithOp(lval, rval, root->lexeme);
            retval = lval;

            // fn: shortcutOp2Int(BTNode *root, int val)
            // shortcut op node to int node
            if (retval->unknown_val == 0)
                shortcutOpNodeToIntNode(root, retval->val);

            updateNodeWeight(root);

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
        node->weight = 1;
    else if (node->left == NULL || node->right == NULL)
        error(NULL_NODE);
    else if (node->left->weight == node->right->weight)
        node->weight = node->left->weight + 1;
    else
        node->weight = max(node->left->weight, node->right->weight);
}

void shortcutOpNodeToIntNode(BTNode *root, int val)
{
    root->token = INT;
    root->weight = 1;
    root->val = val;

    // free subNode
    freeTree(root->left);
    root->left = NULL;
    freeTree(root->right);
    root->right = NULL;
}

void calculateValWithOp(Value *lval, Value *rval, char *op)
{
    if (strcmp(op, "/") == 0 && rval->unknown_val == 0 && rval->val == 0)
        error(DIV_BY_ZERO);

    else if (lval->unknown_val && rval->unknown_val)
        lval->unknown_val = 1;

    // unknown to known condition
    else if (lval->unknown_val)
    {
        if ((strcmp(op, "*") == 0 && rval->val == 0) || (strcmp(op, "&") == 0 && rval->val == 0))
        {
            lval->unknown_val = 0;
            lval->val = 0;
        }
        else
            lval->unknown_val = 1;
    }
    else if (rval->unknown_val)
    {
        if ((strcmp(op, "*") == 0 && lval->val == 0) || (strcmp(op, "&") == 0 && lval->val == 0))
        {
            lval->unknown_val = 0;
            lval->val = 0;
        }
        else
            lval->unknown_val = 1;
    }

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
