
#include "server.c"
#include "client.c"
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc < 5) {
        main_chat(argc, argv);
    } else {
        if (strcmp(argv[1], "-c") == 0) {
            client_main(argc, argv);
        } else if (strcmp(argv[1], "-s") == 0) {
            server_main(argc, argv);
        }
    }
    return 0;
}
