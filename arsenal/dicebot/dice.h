#ifndef QST_DICE_H
#define QST_DICE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdckdint.h>

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
        count++; // for '-'
        n = -n;
    }

    while (n > 0) {
        n /= 10;
        count++;
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

    int e = sscanf(script, "dice+%zud%zu+%ld", &n_dice, &n_sides, &modifier);

    size_t max_chars = 0;

    if (e != 2
    &&  e != 3) {
        return NULL;
    }

    if (n_dice == 0
    ||  n_sides == 0) {
        return NULL;
    }

    do {
        size_t intermediate = 0;
        int ee = 0
            + ckd_add(&max_chars, max_chars, strlen("Rolled "))
            + ckd_mul(&intermediate, n_dice, count_digits(n_sides) + strlen(", "))
            + ckd_add(&max_chars, max_chars, intermediate)
            + ckd_add(&max_chars, max_chars, count_digits(SIZE_MAX) * (n_dice > 1) + strlen(" = "))
            + ckd_add(&max_chars, max_chars, strlen(" (d)") + 1)
            + ckd_add(&max_chars, max_chars, count_digits(n_sides))
            + ckd_add(&max_chars, max_chars, count_digits(n_dice))
        ;

        if (e == 3) {
            has_modifier  = true;
            modifier_sign = modifier > 0;
            modifier     *= modifier_sign ? 1 : -1;

            ee += ckd_add(&max_chars, max_chars, (strlen(" + ") + count_digits(modifier)) * 2);
        }

        if (ee) {
            return strdup("Overflow");
        }
    } while (0);

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
