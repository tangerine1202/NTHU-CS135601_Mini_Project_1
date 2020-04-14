#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "struct_vars.h"
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

int DEBUG_MODE = 1;

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
