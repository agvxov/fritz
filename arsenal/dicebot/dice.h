#ifndef QST_DICE_H
#define QST_DICE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Roll dice in accordance with /qst/'s format.
 *
 * qst-dice-BNF: "dice+" n-dice "d" n-faces modifier
 * n-dice: signed
 * n-faces: signed
 * modifier: sign integer | %empty
 * sign: " + " | " - "
 *
 * --
 *
 * qst-output-BNF: "Rolled" integer tail modifier "(" n-dice "d" n-faces modifier ")"
 * tail: values sum
 * values: ", " integer
 * sum: " = " integer
 *
 * A newly mallocated string is returned on success.
 * NULL is returned on failure.
 */
char * qst_dice(const char * const script);

int count_digits(long long n) {
    if (n == 0) { return 1; }

    int count = 0;
    if (n < 0) {
        ++count; // for '-'
        n = -n;
    }

    while (n > 0) {
        n /= 10;
        ++count;
    }

    return count;
}

char * qst_dice(const char * const script) {
  #define ROLL ((rand() % n_sides) + 1);
    char * r = NULL;

    size_t n_dice;
    size_t n_sides;
    bool   has_modifier = false;
    long   modifier;
    bool   modifier_sign;


    /* Invalid storage is UB.
     * This includes negative input (-1) provided with unsigned variables (size_t).
     * In practice, (GNU) libc will read the input, indicate no error
     *  and creating a wraparound.
     */
    int e = sscanf(script, "dice+%zud%zu+%ld", &n_dice, &n_sides, &modifier);

    if (e != 2
    &&  e != 3) {
        return NULL;
    }

    if (n_dice  == 0
    ||  n_sides == 0) {
        return NULL;
    }

    /* qst_dice() would happily start calculating an absurdly amount of dice rolls.
     * Which is a problem when using it on a server, as it will become a CPU hog
     *  and effectively a DOS.
     * Consequently, this check is provided to restrict the rolls to a sane range.
     * This also means that overflows are impossible (on 64bits).
     */
    if (n_dice  > 1'000
    ||  n_sides > 10'000'000) {
        return NULL;
    }

    size_t max_chars = 0
        + strlen("Rolled ")
        + (n_dice * (count_digits(n_sides) + strlen(", ")))
        + ((count_digits(SIZE_MAX) + strlen(" = ")) * (n_dice > 1))
        + count_digits(n_sides) + count_digits(n_dice) + strlen(" (d)")
        + 1
    ;

    if (e == 3) {
        has_modifier  = true;
        modifier_sign = modifier > 0;
        modifier     *= modifier_sign ? 1 : -1;
        max_chars    += 0
            + (strlen(" + ") + count_digits(modifier)) * 2
        ;
    }

    r = malloc(max_chars);
    if (!r) { return NULL; }
    char * end_ptr = r;

    do {
        memcpy(r, "Rolled ", strlen("Rolled "));
        end_ptr += strlen("Rolled ");
    } while (0);

    do {
        size_t sum = 0;
        size_t roll;

        roll = ROLL;
        end_ptr += sprintf(end_ptr, "%zu", roll);
        sum += roll;

        for (size_t i = 0; i < n_dice-1; i++) {
            roll = ROLL;
            end_ptr += sprintf(end_ptr, ", %zu", roll);
            sum += roll;
        }

        if (n_dice > 1) {
            end_ptr += sprintf(end_ptr, " = %zu", sum);
        }
    } while (0);

    if (has_modifier) {
        end_ptr += sprintf(end_ptr, modifier_sign ? " + %ld" : " - %ld", modifier);
        end_ptr += sprintf(end_ptr, " (%zud%zu %c %ld)",
                    n_dice,
                    n_sides,
                    modifier_sign ? '+' : '-',
                    modifier
        );
    } else {
        end_ptr += sprintf(end_ptr, " (%zud%zu)", n_dice, n_sides);
    }

    return r;
  #undef ROLL
}

#endif
