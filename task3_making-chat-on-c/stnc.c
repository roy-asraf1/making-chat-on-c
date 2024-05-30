#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void handle_client(int client_sock, int argc) {
    char buffer[BUFFER_SIZE]; // meake buffer
    ssize_t read_size;

    while ((read_size = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) { //got the size of the buffer
        buffer[read_size] = '\0';
        if (argc==4)
            printf("server: ");
        else
        {
            printf("client: ");
        }
        printf("%s", buffer);
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) { //to start the program in the terminal
    if (argc < 3) {
        printf("Usage:\n");
        printf("  Client: stnc -c IP PORT\n");
        printf("  Server: stnc -s PORT\n");
        return 1;
    }
    
    int sock;
    struct sockaddr_in addr;
    bool is_client = strcmp(argv[1], "-c") == 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[is_client ? 3 : 2]));

    if (is_client) {
        addr.sin_addr.s_addr = inet_addr(argv[2]);
        printf("client is ready: \n");
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) { //if the connect didnt sucsess
            perror("connect");
            return 1;
        }
    }
    else {
        addr.sin_addr.s_addr = INADDR_ANY;
        printf("server is ready: \n"); 
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) { //if the bind between the port the ip didnt sucess
            perror("bind");
            return 1;
        }
        if (listen(sock, 1) == -1) {
            perror("listen");
            return 1;
        }
        sock = accept(sock, NULL, NULL); // start the accept function
        printf("please enter exit to get out from the chat!");

    }

    if (fork() == 0) {
        handle_client(sock,argc);

    } 
    else
    {
        char buffer[BUFFER_SIZE];
        ssize_t read_size;

        while ((read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
            buffer[read_size] = '\0'; // Ensure null termination for string comparison

            // Check if the user typed "exit"
            if (strcmp(buffer, "exit\n") == 0) {
                printf("client leaves the chat...\n");
                exit(0); // Exit the program
            }

            send(sock, buffer, read_size, 0);
        }

        shutdown(sock, SHUT_WR);
        
        }

    return 0;
}
    












