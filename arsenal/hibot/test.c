// @BAKE gcc -o $*.out $@ -std=c23 -Wall -Wpedantic -ggdb -fsanitize=address
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "syntax-decorator.h"

int main(void) {
    syntax_c();

    do {
        const char input[] = "int f(void) { return 10; }";
        puts(syntax_highlight(input));
    } while (0);

    return 0;
}
