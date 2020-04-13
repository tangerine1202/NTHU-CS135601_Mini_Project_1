#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

int main(void)
{

    // freopen("input.txt", "r", stdin);
    // freopen("output.txt", "w", stdout);

    while (1)
    {
        statement();
    }
    return 0;
}
