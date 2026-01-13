#ifndef REQUEST_H
#define REQUEST_H

const int          message_timeout            = 10;
const char * const message_queue_full_message = "Air space too crowded in this area.";

// As defined in the official specification for IRC
#define IRC_MESSAGE_MAX 512

typedef struct {
	char * user;
	language_t language;
	time_t expiry;
	char * buffer[IRC_MESSAGE_MAX];
	unsigned int buffer_head;
} request_t;

request_t request;

static inline
void init_request(request_t * request) {
    *request = (request_t) {
        .user        = NULL,
        .language    = selected_language,
        .expiry      = 0,
        .buffer_head = 0,
    };
}

static inline
void reinit_request(request_t * request) {
	free(request->user);
    for (long i = 0; i < request->buffer_head; i++) {
        free(request->buffer[i]);
    }
    init_request(request);
}

static inline
void touch_request_timer(request_t * const request) {
    request->expiry = time(NULL) + message_timeout;
}

static inline
bool is_expired_request(const request_t * const request) {
    return request->expiry < time(NULL);
}

request_t * take_request(const char * const user) {
    if (is_expired_request(&request)) {
        reinit_request(&request);
    }

    if (request.user) {
        // same user
        if (!strcmp(request.user, user)) {
            return &request;
        // in use
        } else {
            return NULL;
        }
    }

    // new user
	request.user = strdup(user);

	return &request;
}

#endif
