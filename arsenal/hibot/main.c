// @BAKE gcc -o hibot $@ -lfcgi -fsanitize=address -ggdb
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <fcgiapp.h>

#define logf_notice(...)

#define switchs(s) do { const char *__ss = (s); if (0) {
#define cases(x)   } else if (strcmp(__ss, (x)) == 0) {
#define defaults   } else {
#define endswitchs } } while (0)

#define PROGRAM_NAME "hibot"
#define SOCKET_PATH  PROGRAM_NAME ".sock"

FCGX_Request request;

const char * CHANNEL  = NULL;
const char * USERNAME = NULL;
const char * MESSAGE  = NULL;

#define DEFAULT_LANGUAGE C

typedef enum {
	C,
	CPP,
	ASM,
	ADA,
} language_t;

typedef void (*syntax_setter_t)(void);

#include "syntax.h"
#include "timer.h"

syntax_setter_t syntax_functions[] = {
	[C]   = &syntax_c,
	[CPP] = &syntax_cpp,
	[ASM] = &syntax_fasm,
	[ADA] = &syntax_ada,
};

static
language_t translate_language(const char * const language) {
    switchs (language)
        cases ("C")        return C;
        cases ("C++")      return CPP;
        cases ("CPP")      return CPP;
        cases ("ASM")      return ASM;
        cases ("FASM")     return ASM;
        cases ("ASSEMBLY") return ASM;
        cases ("ADA")      return ADA;
    endswitchs;
	return -1;
}

static inline
void irc_message(const char * const message) {
    FCGX_PutStr(message, strlen(message), request.out);
    FCGX_PutStr("\n", 1, request.out);
}

static
void irc_help() {
    irc_message(USERNAME);

	irc_message(PROGRAM_NAME " "
#include "version.inc"
				);
	irc_message(PROGRAM_NAME " is a code highlighting IRC bot."
					" You may direct message it with your code or commands."
				);
	irc_message("Syntax:");
	irc_message("  !help               // print help");
	irc_message("  !<language>         // set language for next message");
	irc_message("  <code>              // echo this code");
	irc_message("  !<language> <code>  // set language and echo code");
	irc_message("  !--                 // flush all code");
	irc_message("--");
}

void flush_request(request_t * request) {
	setitimer(ITIMER_REAL, NULL, NULL);
    if (!request_queue_head) {
        return;
    }

	// Message header
	irc_message(USERNAME);

    // Message body
    syntax_count = 0;
    syntax_functions[request->language]();
    for (unsigned i = 0; i < request->buffer_head; i++) {
        irc_message(syntax_highlight(request->buffer[i]));
    }

	// Message footer
	irc_message("--");

	logf_notice("Flushed message: %p (%d)", (void*)request, request_queue_head);

    drop_request(request);
}

static
void event_privmsg(const char * message_) {
	char * const message_guard = strdup(message_);
	char *       message       = message_guard;
	char * terminator;
	int is_code = 1;

	/* Is command */
	if (*message == '!') {
		if (!strcmp(message, "!help")) {
			irc_help();
			goto END;
		}
		/* */
		terminator = message;
		while (*terminator != ' ') {
			if (*terminator == '\0') {
				is_code = 0;
				break;
			}
			++terminator;
		}
		*terminator = '\0';
        /* */
        {
            request_t * request = take_request(USERNAME);

            if (!strcmp(message, "!--")) {
                if (request) {
                    flush_request(request);
                }
                goto END;
            }

            /* get language */
            for (char * s = message + 1; *s != '\0'; s++) {
                *s = toupper(*s);
            }
            int l = translate_language(message + 1);
            message = terminator + 1;
            if (l != -1) {
                request->language = l;
            }
        }
	}

	/* Is code */
	if (is_code) {
		request_t * request = take_request(USERNAME);
		if (!request) {
			irc_message(USERNAME);
            irc_message(message_queue_full_message);
			goto END;
		}

		touch_request_timer(request);
		request->buffer[request->buffer_head++] = strdup(message);
	}

  END:
	free(message_guard);
}

char * slurp_FCGX_Stream(FCGX_Stream * stream) {
    char * r = NULL;

    size_t len = 0;
    size_t cap = 4096;
    char buf[4096];
    r = (char*)malloc(cap * sizeof(char));
    if (!r) { return r; }

    while (true) {
        size_t bytes = FCGX_GetStr(buf, sizeof(buf), stream);

        if (bytes > 0) {
            while (len + bytes > cap) {
                cap *= 2;
                r = realloc(r, cap);
            }
            memcpy(r + len, buf, bytes);
            len += bytes;
        }

        if (FCGX_HasSeenEOF(stream)) { break; }
    }

    r[len] = '\0';

    return r;
}

int main(void) {
    // Init internals
	syntax_functions[DEFAULT_LANGUAGE]();

	for (unsigned int i = 0; i < message_queue_size; i++) {
		request_queue[i] = &request_queue__[i];
		init_request(request_queue[i]);
	}

	signal(SIGALRM, on_request_timeout);

    // Init FCGI
    FCGX_Init();

    int sock = FCGX_OpenSocket(SOCKET_PATH, 5);
    if (sock < 0) {
        perror("FCGX_OpenSocket");
        exit(1);
    }

    FCGX_InitRequest(&request, sock, 0);

    // Event loop
    while (FCGX_Accept_r(&request) == 0) {
        CHANNEL  = FCGX_GetParam("CHANNEL",  request.envp);
        USERNAME = FCGX_GetParam("USERNAME", request.envp);
        MESSAGE  = slurp_FCGX_Stream(request.in);

        event_privmsg(MESSAGE);

        FCGX_Finish_r(&request);
    }

    return 0;
}
