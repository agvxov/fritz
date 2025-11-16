#ifndef FCGIEXTRA
#define FCGIEXTRA

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

#endif
