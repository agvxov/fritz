// @BAKE gcc -o $*.out $@ -std=c23 -ggdb -Wall -Wpedantic -fsanitize=address,undefined
#include "dice.h"

int puts_wrapper(char * s) {
    if (s == NULL) {
        puts("(NULL)");
        return 0;
    }

    auto r = puts(s);
    free(s);
    return r;
}

signed main(void) {
    puts_wrapper(qst_dice(""));                    // invalid: empty string input
    puts_wrapper(qst_dice("dice+"));               // invalid: empty spec after prefix
    puts_wrapper(qst_dice("dice+0d6"));            // invalid: zero dice
    puts_wrapper(qst_dice("dice+1d0"));            // invalid: zero-sided die
    puts_wrapper(qst_dice("dice+-1d6"));           // invalid: negative dice count
    puts_wrapper(qst_dice("dice+1d-6"));           // invalid: negative sides
    puts_wrapper(qst_dice("dice+1d6"));            // simple: single die
    puts_wrapper(qst_dice("dice+2d6+3"));          // simple: multiple dice + positive modifier
    puts_wrapper(qst_dice("dice+2d6+-3"));         // simple: multiple dice + negative modifier
    puts_wrapper(qst_dice("dice+10d4-5"));         // edge: malformed negative modifier
    puts_wrapper(qst_dice("dice+1000d6"));         // stress: many dice
    puts_wrapper(qst_dice("dice+2147483647d1"));   // stress: huge count (overflow/limits)
    puts_wrapper(qst_dice("dice+1d2147483647"));   // stress: huge sides (overflow/limits)
    puts_wrapper(qst_dice("dice+1000d10000000"));  // stress: huge sides (overflow/limits)

    return 0;
}
