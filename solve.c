#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer_parser.h"
#include "code_gen.h"
#include "helpful.h"

int main(void)
{
    /*
    freopen( "input.in" , "r" , stdin ) ;
    freopen( "output.out" , "w" , stdout ) ;
    */

    while (1)
    {
        statement();
    }
    return 0;
}
