// @BAKE gcc -std=c23 -o hibot $@ -lfcgi -ggdb
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <fcgiapp.h>
#include "fcgiextra.h"

#define logf_notice(...)

#define switchs(s) do { const char *__ss = (s); if (0) {
#define cases(x)   } else if (strcmp(__ss, (x)) == 0) {
#define defaults   } else {
#define endswitchs } } while (0)

#define PROGRAM_NAME "hibot"
#define SOCKET_PATH  PROGRAM_NAME ".sock"

FCGX_Request fcgi;

const char * USERNAME      = NULL;
const char * MESSAGE       = NULL;
const char * INPUT_CHANNEL = NULL;
/* The channel to dump highlighting to.
 *  Not neceserally the channel we will respond to.
 */
const char * OUTPUT_CHANNEL  = NULL;

typedef enum {
	C,
	CPP,
	ASM,
	ADA,
} language_t;
language_t selected_language = C;

typedef void (*syntax_setter_t)(void);

#include "syntax.h"
#include "request.h"

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
    FCGX_PutStr(message, strlen(message), fcgi.out);
    FCGX_PutStr("\n", 1, fcgi.out);
}

static
void irc_help() {
    irc_message(INPUT_CHANNEL);

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
	irc_message("  --                  // flush all code");
	irc_message("--");
}

void flush_request(request_t * request) {
    // Ignore empty
    if (!request->buffer_head) {
        goto end;
    }

	// Message header
	irc_message(OUTPUT_CHANNEL);

    // Message body
    syntax_count = 0;
    syntax_functions[request->language]();
    for (unsigned i = 0; i < request->buffer_head; i++) {
        irc_message(syntax_highlight(request->buffer[i]));
    }

	// Message footer
	irc_message("--");

  end:
	logf_notice("Flushed message: %p", (void*)request);

    reinit_request(request);
}

static
void handle_message(const char * message_) {
	char * const message_guard = strdup(message_);
	char *       message       = message_guard;

	/* Is command */
	if (*message == '!') {
		if (!strcmp(message, "!help")) {
			irc_help();
		}

        if (!strncmp(message, "!language ", strlen("!language "))) {
            char * language_name = message + strlen("!language ");

            for (char * s = language_name; *s != '\0'; s++) { *s = toupper(*s); }

            int l = translate_language(language_name);
            if (l != -1) {
                selected_language = l;
            }
        }

		goto end;
	} else {
        // From public channels, only commands are allowed
        if (INPUT_CHANNEL[0] == '#') {
            goto end;
        }
    }

    /* Is terminator */
    if (!strcmp(message, "--")) {
        request_t * request = take_request(USERNAME);
        if (request) {
            flush_request(request);
        }
        goto end;
    }

	/* Is code */
    request_t * request = take_request(USERNAME);
    if (!request) {
        irc_message(USERNAME);
        irc_message(message_queue_full_message);
        goto end;
    }
    touch_request_timer(request);
    request->buffer[request->buffer_head++] = strdup(message);

  end:
	free(message_guard);
}

int main(void) {
    // Init internals
	syntax_functions[selected_language]();
	init_request(&request);

    // Init FCGI
    FCGX_Init();

    int sock = FCGX_OpenSocket(SOCKET_PATH, 5);
    if (sock < 0) {
        perror("FCGX_OpenSocket");
        exit(1);
    }

    FCGX_InitRequest(&fcgi, sock, 0);

    // Event loop
    while (FCGX_Accept_r(&fcgi) == 0) {
        char chan_buf[32] = {0};
        strncpy(chan_buf, FCGX_GetParam("JOINED",  fcgi.envp), sizeof(chan_buf)-1);
        for (auto s = chan_buf; *s; s++) { if (*s == ':') { *s = '\0'; break; } }

        OUTPUT_CHANNEL = chan_buf;
        INPUT_CHANNEL  = FCGX_GetParam("CHANNEL", fcgi.envp);
        USERNAME       = FCGX_GetParam("USERNAME", fcgi.envp);
        MESSAGE        = slurp_FCGX_Stream(fcgi.in);

        if (strcmp(USERNAME, PROGRAM_NAME)) { // ignore self
            handle_message(MESSAGE);
        }

        FCGX_Finish_r(&fcgi);
    }

    return 0;
}
