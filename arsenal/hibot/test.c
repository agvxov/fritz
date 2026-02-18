// @BAKE gcc -o $*.out $@ -std=c23 -Wall -Wpedantic -ggdb
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "syntax.h"

int main(void) {
    syntax_c();

    do {
        const char input[] = "static";
        puts(syntax_highlight(input));
    } while (0);

    return 0;
}
