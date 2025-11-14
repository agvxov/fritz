// @BAKE gcc -o $*.out $@ -lfcgi
#include <fcgiapp.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_PATH "hw-c.sock"

int main(void) {
    FCGX_Request request;

    FCGX_Init();

    int sock = FCGX_OpenSocket(SOCKET_PATH, 5);
    if (sock < 0) {
        perror("FCGX_OpenSocket");
        exit(1);
    }

    FCGX_InitRequest(&request, sock, 0);

    while (FCGX_Accept_r(&request) == 0) {
		const char buffer[] = "Hello from C!";
		FCGX_PutStr(buffer, sizeof(buffer)-1, request.out);
        FCGX_Finish_r(&request);
    }

    return 0;
}
