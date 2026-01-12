// @BAKE gcc -o $*.out $@ -lfcgi
/* Send back exit order on request.
 * Useful for automated tests where.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcgiapp.h>

#define SOCKET_PATH "exit-out.sock"

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
		FCGX_PutStr("!exit\n", strlen("!exit\n"), request.out);
        FCGX_Finish_r(&request);
    }

    return 0;
}
