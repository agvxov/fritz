#ifndef SYNTAX_H
#define SYNTAX_H
#include <stdbool.h>
#include <string.h>

// E.g: + - * / 0 1
typedef struct {
    const char * chars;
    const char * hl_start;
    const char * hl_end;
} char_group_t;

// E.g: short int long
typedef struct {
    const char * const * keywords;
    const char * hl_start;
    const char * hl_end;
} keyword_group_t;

// E.g: " \" "
typedef struct {
    const char * start;
    const char * end;
    const char * escape;
    const char * hl_start;
    const char * hl_end;
} region_t;

/* Used to determine word boundaries,
 *  so that partial matches may not result in highlighting.
 * This also means that keywords may only contain characters
 *  which are listed here.
 * XXX: i believe unicode should work out of pure accident
 */
const char * const word_characters =
  #ifdef SYNTAX_WORD_CHARACTERS
    SYNTAX_WORD_CHARACTERS
  #else
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "_-"
  #endif
;

extern int syntax_init(void);
extern int syntax_deinit(void);
extern int syntax_define_chars(const char * chars, const char * hl_start, const char * hl_end);
extern int syntax_define_keywords(const char * const * keywords, const char * hl_start, const char * hl_end);
extern int syntax_define_region(const char * start, const char * end, const char * escape, const char * hl_start, const char * hl_end);
extern size_t syntax_max_memory_requirement(size_t input_len);
extern void syntax_highlight_string(char * const destination, const char * const source, const size_t destination_size);

/* Char groups are optimized such that if multiple chars of the same group follow each other,
 *  only one hl_start and hl_end will be used.
 */

/* When faced with a keyword only partially fitting,
 * there are only a few options:
 *   1) cut indiscriminately -> risk partial escape sequences forming eldritch outputs
 *   2) cut the keyword -> effectively highlight partial matches
 *   3) insert hl_start and discard the keyword or hl_end -> produce puzzling output that feels like a bug
 *   4) threat the operation as atomic -> waste buffer space, make strlen(destination) != sizeof(destination)-1
 * We found option 4 the least problematic.
 *
 * Char groups truncate as if every character was its own keyword,
 *  memory optimization is considered as a second thought.
 * I.e. atleast one and as many chars as possible will be placed between hl_start and hl_end,
 *  or there won't be a trace at all.
 *
 * We apply the same logic for region starts themselves, however not the contents of a region.
 * That is, for example a truncated string will render as a unterminated string.
 * This behaviour is consistent with the string actually missing a termination.
 */
#endif

#ifdef SYNTAX_IMPLEMENTATION
#ifndef SYNTAX_DEFINITION_MAX
#  define SYNTAX_DEFINITION_MAX 16
#endif
static char_group_t char_groups[SYNTAX_DEFINITION_MAX];
static int char_groups_empty_top;
static keyword_group_t keyword_groups[SYNTAX_DEFINITION_MAX];
static int keyword_groups_empty_top;
static region_t regions[SYNTAX_DEFINITION_MAX];
static int regions_empty_top;

int syntax_init(void) {
    char_groups_empty_top    = 0;
    keyword_groups_empty_top = 0;
    regions_empty_top        = 0;

    return 0;
}

int syntax_deinit(void) {
    return 0;
}

int syntax_define_chars(
  const char * chars,
  const char * hl_start,
  const char * hl_end
) {
    if (char_groups_empty_top >= SYNTAX_DEFINITION_MAX
    ||  !chars) {
        return 1;
    }

    char_groups[char_groups_empty_top].chars = chars;
    char_groups[char_groups_empty_top].hl_start = hl_start;
    char_groups[char_groups_empty_top].hl_end   = hl_end;
    ++char_groups_empty_top;

    return 0;
}

int syntax_define_keywords(
  const char * const * keywords,
  const char * hl_start,
  const char * hl_end
) {
    if (keyword_groups_empty_top >= SYNTAX_DEFINITION_MAX
    ||  !keywords) {
        return 1;
    }

    keyword_groups[keyword_groups_empty_top].keywords = keywords;
    keyword_groups[keyword_groups_empty_top].hl_start = hl_start;
    keyword_groups[keyword_groups_empty_top].hl_end   = hl_end;
    ++keyword_groups_empty_top;

    return 0;
}

int syntax_define_region(
  const char * start,
  const char * end,
  const char * escape,
  const char * hl_start,
  const char * hl_end
) {
    if (regions_empty_top >= SYNTAX_DEFINITION_MAX
    ||  !start) {
        return 1;
    }

    regions[regions_empty_top].start    = start;
    regions[regions_empty_top].end      = end;
    regions[regions_empty_top].escape   = escape;
    regions[regions_empty_top].hl_start = hl_start;
    regions[regions_empty_top].hl_end   = hl_end;
    ++regions_empty_top;

    return 0;
}

/* Given an input length and the current highlighting,
 *  return the worst case scenario for the required buffer's size.
 */
size_t syntax_max_memory_requirement(
  size_t input_len
) {
    size_t r = input_len;

    for (int i = 0; i < keyword_groups_empty_top; i++) {
        size_t start_len = strlen(keyword_groups[i].hl_start);
        size_t end_len   = strlen(keyword_groups[i].hl_end);
        for (const char * const * w = keyword_groups[i].keywords; *w != NULL; w++) {
            size_t l = strlen(*w);
            l = (input_len / l)
              * (l + start_len + end_len)
            ;
            if (r < l) {
                r = l;
            }
        }
    }

    for (int i = 0; i < regions_empty_top; i++) {
        size_t l = strlen(regions[i].start)
                 + strlen(regions[i].end)
        ;
        l = (input_len / l)
          * (l + strlen(regions[i].hl_start) + strlen(regions[i].end))
        ;
        if (r < l) {
            r = l;
        }
    }

    return r;
}

static
int _syntax_destination_append(
  char * * destination,
  size_t * remaining,
  const char * source,
  size_t len
) {
    if (*remaining == 0 || len >= *remaining) {
        return 1;
    }

    memcpy(*destination, source, len);
    *destination += len;
    *remaining -= len;
    return 0;
}

// Core function
void syntax_highlight_string(
  char * const destination,
  const char * const source,
  const size_t destination_size
) {
    if (destination      == NULL
    ||  source           == NULL
    ||  destination_size == 0) {
        return;
    }

    char * out       = destination;
    const char * s   = source;
    size_t remaining = destination_size;
    bool in_word = false;

    while (*s) {
        bool matched = false;

        // Regions
        for (int i = 0; i < regions_empty_top; i++) {
            const size_t start_len = strlen(regions[i].start);
            if (start_len == 0) {
                continue;
            }

            if (strncmp(s, regions[i].start, start_len) != 0) {
                continue;
            }

            const size_t end_len    = regions[i].end      ? strlen(regions[i].end)    : 0;
            const size_t escape_len = regions[i].escape   ? strlen(regions[i].escape) : 0;
            const char * hl_start   = regions[i].hl_start ? regions[i].hl_start       : "";
            const char * hl_end     = regions[i].hl_end   ? regions[i].hl_end         : "";

            char * const saved_out = out;
            if (_syntax_destination_append(&out, &remaining, hl_start, strlen(hl_start))
            ||  _syntax_destination_append(&out, &remaining, regions[i].start, start_len)) {
                out = saved_out;
                goto done;
            }

            s += start_len;

            while (*s) {
                if (escape_len != 0
                &&  strncmp(s, regions[i].escape, escape_len) == 0) {
                    if (_syntax_destination_append(&out, &remaining, regions[i].escape, escape_len)) {
                        goto done;
                    }
                    s += escape_len;

                    if (*s) {
                        if (_syntax_destination_append(&out, &remaining, s, 1)) {
                            goto done;
                        }
                        ++s;
                    }
                    continue;
                }

                if (end_len != 0
                &&  strncmp(s, regions[i].end, end_len) == 0) {
                    if (_syntax_destination_append(&out, &remaining, regions[i].end, end_len)
                    ||  _syntax_destination_append(&out, &remaining, hl_end, strlen(hl_end))) {
                        goto done;
                    }
                    s += end_len;
                    matched = true;
                    break;
                }

                if (_syntax_destination_append(&out, &remaining, s, 1)) {
                    goto done;
                }
                ++s;
            }

            if (matched == false) {
                if (_syntax_destination_append(&out, &remaining, hl_end, strlen(hl_end))) {
                    goto done;
                }
            }

            matched = true;
            break;
        }

        if (matched) {
            in_word = false;
            continue;
        }

        // Keywords
        if (strchr(word_characters, (unsigned char)*s) != NULL) {
            if (in_word) {
                goto keyword_end;
            }
            in_word = true;

            const char * word_end = s;
            while (*word_end
            &&     strchr(word_characters, (unsigned char)*word_end) != NULL) {
                ++word_end;
            }

            const size_t word_len = (size_t)(word_end - s);

            for (int i = 0; i < keyword_groups_empty_top && !matched; i++) {
                const char * const * w = keyword_groups[i].keywords;
                if (w == NULL) {
                    continue;
                }

                for (; *w != NULL; w++) {
                    const size_t kw_len = strlen(*w);
                    if (kw_len == word_len && memcmp(s, *w, word_len) == 0) {
                        const char * hl_start = keyword_groups[i].hl_start ? keyword_groups[i].hl_start : "";
                        const char * hl_end   = keyword_groups[i].hl_end   ? keyword_groups[i].hl_end   : "";

                        char * const saved_out = out;
                        if (_syntax_destination_append(&out, &remaining, hl_start, strlen(hl_start))
                        ||  _syntax_destination_append(&out, &remaining, *w, word_len)
                        ||  _syntax_destination_append(&out, &remaining, hl_end, strlen(hl_end))) {
                            out = saved_out;
                            goto done;
                        }

                        s = word_end;
                        matched = true;
                        break;
                    }
                }
            }
        } else {
            in_word = false;
        }

        if (matched) {
            continue;
        }

      keyword_end:

        // Char
        for (int i = 0; i < char_groups_empty_top; i++) {
            const char * chars = char_groups[i].chars;
            if (chars == NULL) {
                continue;
            }

            if (strchr(chars, (unsigned char)*s) == NULL) {
                continue;
            }

            const char * run_end = s;
            while (*run_end && strchr(chars, (unsigned char)*run_end) != NULL) {
                ++run_end;
            }

            const char * hl_start = char_groups[i].hl_start ? char_groups[i].hl_start : "";
            const char * hl_end   = char_groups[i].hl_end   ? char_groups[i].hl_end   : "";

            const size_t hl_start_len = strlen(hl_start);
            const size_t hl_end_len   = strlen(hl_end);

            if (remaining <= hl_start_len + hl_end_len + 1) {
                continue;
            }

            size_t max_payload = remaining - 1 - hl_start_len - hl_end_len;
            size_t run_len = (size_t)(run_end - s);
            size_t emit_len = run_len < max_payload ? run_len : max_payload;

            if (emit_len == 0) {
                continue;
            }

            if (_syntax_destination_append(&out, &remaining, hl_start, hl_start_len)
            ||  _syntax_destination_append(&out, &remaining, s, emit_len)
            ||  _syntax_destination_append(&out, &remaining, hl_end, hl_end_len)) {
                goto done;
            }

            s += emit_len;
            matched = true;
            break;
        }

        if (matched) {
            continue;
        }

        // Regular text
        if (_syntax_destination_append(&out, &remaining, s, 1)) {
            goto done;
        }
        ++s;
    }

  done:
    if (remaining > 0) {
        *out = '\0';
    } else if (destination_size > 0) {
        destination[destination_size - 1] = '\0';
    }
}
#endif
