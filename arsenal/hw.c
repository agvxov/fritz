// @BAKE gcc -o $*.out $@ -lfcgi
#include <fcgiapp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "hw-out.sock"
const char msg[] = "Hello from C!";

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
		const char * const target = FCGX_GetParam("CHANNEL", request.envp);
		FCGX_PutStr(target, strlen(target), request.out);
		FCGX_PutStr("\n", 1, request.out);
		FCGX_PutStr(msg, sizeof(msg)-1, request.out);
        FCGX_Finish_r(&request);
    }

    return 0;
}
