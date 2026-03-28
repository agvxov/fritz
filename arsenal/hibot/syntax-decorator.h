#define SYNTAX_IMPLEMENTATION
#define SYNTAX_DEFINITION_MAX 20
#include "syntax.h"

/* Control codes */
#define IRC_RST       "\x0F"
#define IRC_BOLD      "\x02"
#define IRC_ITALIC    "\x1D"
#define IRC_UNDERLINE "\x1F"
#define IRC_REVERSE   "\x16"
#define IRC_COLOR     "\x03"
/* Foreground colors (mIRC standard) */
#define IRC_WHITE       "\x03" "00"
#define IRC_BLACK       "\x03" "01"
#define IRC_BLUE        "\x03" "02"
#define IRC_GREEN       "\x03" "03"
#define IRC_RED         "\x03" "04"
#define IRC_BROWN       "\x03" "05"
#define IRC_PURPLE      "\x03" "06"
#define IRC_ORANGE      "\x03" "07"
#define IRC_YELLOW      "\x03" "08"
#define IRC_LIGHT_GREEN "\x03" "09"
#define IRC_CYAN        "\x03" "10"
#define IRC_LIGHT_CYAN  "\x03" "11"
#define IRC_LIGHT_BLUE  "\x03" "12"
#define IRC_PINK        "\x03" "13"
#define IRC_GRAY        "\x03" "14"
#define IRC_LIGHT_GRAY  "\x03" "15"

static const char * const digits = {
    "0123456789"
};

void syntax_c(void) {
    static const char * const type[] = {
        "void",
        "bool",
        "char",
        "int",
        "float",
        "double",
        "size_t",
        "FILE",
        "char8_t",
        "char16_t",
        "char32_t",
        "wchar_t",
    };
    static const char * const group[] = {
        "template",
        "class",
        "decltype",
        "typeid",
        "typename",
    };
    static const char * const group[] = {
        "enum",
        "struct",
        "typedef",
        "union",
    };
    static const char * const value[] = {
        "true",
        "false",
        "TRUE",
        "FALSE",
        "nullptr",
        "NULL",
    };
    static const char * const specifier[] = {
        "extern",
        "static",
        "inline",
        "const",
        "auto",
        "signed",
        "unsigned",
        "short",
        "long",
    };
    static const char * const control[] = {
        "break",
        "continue",
        "goto",
        "return",
    };
    static const char * const jump[] = {
        "if",
        "else",
        "while",
        "do",
        "for",
        "default",
        "case",
        "error",
        "end",
        "done",
    };
    static const char * const special[] = {
        "sizeof",
        "alignas",
        "alignof",
        "restrict",
        "static_assert",
        "thread_local",
        "typeof",
        "typeof_unqual",
        "volatile",
        "asm",
        "fortran",
    };
    syntax_deinit();
    syntax_init();

    syntax_define_chars("+-*/%=&|^~!<>?:", IRC_RED, IRC_RST);
    syntax_define_chars(".;()[]{}", IRC_YELLOW, IRC_RST);
    syntax_define_chars(digits, IRC_CYAN, IRC_RST);
    syntax_define_keywords(type, IRC_BLUE, IRC_RST);
    syntax_define_keywords(group, IRC_LIGHT_GREEN, IRC_RST);
    syntax_define_keywords(value, IRC_PURPLE, IRC_RST);
    syntax_define_keywords(specifier, IRC_LIGHT_GREEN, IRC_RST);
    syntax_define_keywords(jump, IRC_BROWN, IRC_RST);
    syntax_define_keywords(special, IRC_ORANGE, IRC_RST);
    syntax_define_region("\"", "\"", "\\", IRC_GREEN, IRC_RST);
    syntax_define_region("'", "'", "\\", IRC_GREEN, IRC_RST);
    syntax_define_region("#", "", "\n", IRC_GRAY, IRC_RST);
    syntax_define_region("//", "\"", "\n", IRC_GRAY, IRC_RST);
    syntax_define_region("/*", "", "*/", IRC_GRAY, IRC_RST);
}

void syntax_cpp(void) {
    syntax_c();

    static const char * const logic[] = {
        "and",
        "and_eq",
        "bitand",
        "bitor",
        "not",
        "not_eq",
        "or",
        "or_eq",
        "xor",
        "xor_eq ",
    };
    static const char * const exception[] = {
        "try",
        "catch",
        "noexcept",
        "throw",
    };
    static const char * const atomic[] = {
        "atomic_cancel",
        "atomic_commit",
        "atomic_noexcept",
    };
    static const char * const cast[] = {
        "const_cast",
        "dynamic_cast",
        "reinterpret_cast",
        "static_cast",
    };
    static const char * const constant[] = {
        "consteval",
        "constexpr",
        "constinit",
    };
    static const char * const coroutine[] = {
        "co_await",
        "co_return",
        "co_yield",
    };
    static const char * const allocation[] = {
        "new",
        "delete",
    };
    static const char * const contract[] = {
        "compl",
        "pre",
        "post",
        "contract_assert",
        "transaction_safe",
        "transaction_safe_dynamic",
    };
    static const char * const protection[] = {
        "friend",
        "private",
        "protected",
        "public",
    };
    static const char * const extra_special[] = {
        "trivially_relocatable_if_eligible",
        "replaceable_if_eligible",
    };
    static const char * const bullshit1[] = {
        "using",
        "namespace",
        "import",
        "export",
        "module",
        "requires",
    };
    static const char * const bullshit2[] = {
        "this",
        "final",
        "virtual",
        "override",
        "concept",
        "explicit",
        "mutable",
        "operator",
        "reflexpr",
        "synchronized",
    };

    syntax_define_chars(logic, IRC_RED, IRC_RST);
    syntax_define_chars(exception, IRC_YELLOW, IRC_RST);
    syntax_define_chars(atomic, IRC_GREEN, IRC_RST);
    syntax_define_chars(cast, IRC_PINK, IRC_RST);
    syntax_define_chars(constant, IRC_BLUE, IRC_RST);
    syntax_define_chars(coroutine, IRC_LIGHT_CYAN, IRC_RST);
    syntax_define_chars(allocation, IRC_YELLOW, IRC_RST);
    syntax_define_chars(contract, IRC_ORANGE, IRC_RST);
    syntax_define_chars(protection, IRC_LIGHT_GREEN, IRC_RST);
    syntax_define_chars(extra_special, IRC_ORANGE, IRC_RST);
    syntax_define_chars(bullshit1, IRC_PURPLE, IRC_RST);
    syntax_define_chars(bullshit2, IRC_RED, IRC_RST);
}

void syntax_fasm(void) {
    syntax_c(); // XXX xolatile, fix this, i'm begging you
}

void syntax_ada(void) {
    syntax_c(); // XXX xolatile, fix this, i'm begging you
}

char * syntax_highlight(const char * s) {
  #define BUFFER_SIZE 1024
    static char buffer[BUFFER_SIZE];

    syntax_highlight_string(buffer, s, BUFFER_SIZE);

    return buffer;
  #undef BUFFER_SIZE
}
