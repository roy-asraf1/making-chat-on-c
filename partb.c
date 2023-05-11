#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/sha.h>


//#include <openssl/sha.h>

#define BUFFER_SIZE 1024
#define CHUNK_SIZE (100 * 1024 * 1024)




void handle_client(int client_sock, int argc) {
    char buffer[BUFFER_SIZE]; // make buffer
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

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    char *type = NULL;
    char *param = NULL;
    int perform_test = 0;
    char *ip = NULL;
    char *port = NULL;
    int quiet_mode = 0;
    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-s") == 0)
        {
            if (i+1 >= argc) 
            {
                printf("Error: port not specified.\n");
                return 1;
            }
            port = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0) 
        {
            perform_test = 1;
        } 
        else if (strcmp(argv[i], "-q") == 0) {
            quiet_mode = 1;
        }
    }

    // Validate command-line arguments
    if (port == NULL) 
    {
        printf("Error: port not specified.\n");
        return 1;
    }
    if (!perform_test) 
    {
        printf("Error: -p option not specified.\n");
        return 1;
    }
        // Generate data
    char *data = malloc(CHUNK_SIZE);
    if (data == NULL) {
        perror("malloc");
        return 1;
    }


    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        data[i] = rand() % 256;
    }

    // need to add the preformance of checksum
    // Perform the test of ipv4 (tcp&udp)
    if (strcmp(type, "ipv4") == 0) 
    {
        if (strcmp(param, "tcp") == 0)
        {
        // Perform IPv4 TCP test

        void ipv4_tcp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc,char *argv);
        print_test_result("ipv4_tcp", "ms", time, quiet_mode);

        }
        else if (strcmp(param, "udp") == 0) 
        {
        // Perform IPv4 UDP test
            void ipv4_udp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("ipv4_udp", "ms", time, quiet_mode);
        }
    } 
    // Perform the test of ipv6 (tcp&udp)
    else if (strcmp(type, "ipv6") == 0)
    {
        if (strcmp(param, "tcp") == 0) 
        {
            // Perform IPv6 TCP test
            void ipv6_tcp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("ipv6_tcp", "ms", time, quiet_mode);
        } 
        else if (strcmp(param, "udp") == 0) 
        {
            // Perform IPv6 UDP test
            void ipv6_udp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("ipv6_udp", "ms", time, quiet_mode);
        }
    } 
    // Perform the test of uds dgram and uds stream
    else if (strcmp(type, "uds") == 0) 
    {
        if (strcmp(param, "dgram") == 0) 
        {
            // Perform UDS datagram test
            void uds_dgram(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("uds_dgram", "ms", time, quiet_mode);
        }
        else if (strcmp(param, "stream") == 0) 
        {
            // Perform UDS stream test
            void uds_stream(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("uds_stream", "ms", time, quiet_mode);
        }
    } 

    else if (strcmp(type, "mmap") == 0) 
    {
        int fd = open(param, O_RDONLY);
        if (fd == -1) {
            printf("Error: could not open file %s: %s\n", param, strerror(errno));
            return 1;
        }
        struct stat st;
        if (fstat(fd, &st) == -1) {
            printf("Error: could not stat file %s: %s\n", param, strerror(errno));
            close(fd);
            return 1;
        }
        if (!S_ISREG(st.st_mode)) {
            printf("Error: file %s is not a regular file.\n", param);
            close(fd);
            return 1;
        }
        void *mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mem == MAP_FAILED) {
            printf("Error: could not map file %s: %s\n", param, strerror(errno));
            close(fd);
            return 1;
        }
        // Perform mmap test
        void mmapfileName(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
        print_test_result("mmap", "ms", time, quiet_mode);
        munmap(mem, st.st_size);
        close(fd);
    } 

    
    else if (strcmp(type, "pipe") == 0) 
    {
        if (strcmp(param, "r") == 0)
        {
            int fd = open(type, O_RDONLY);
            if (fd == -1) {
                printf("Error: could not open pipe %s for reading: %s\n", param, strerror(errno));
                return 1;
            }
            // Perform pipe read test
            // ...
            double time = 0.0;  // Replace with actual time
            print_test_result("pipe_r", "ms", time, quiet_mode);
            close(fd);
        } 
        else if (strcmp(param, "w") == 0) 
        {
            int fd = open(type, O_WRONLY);
            if (fd == -1) 
            {
                printf("Error: could not open pipe %s for writing: %s\n", param, strerror(errno));
                return 1;
            }
            // Perform pipe write test
            void pipe_w(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv);
            print_test_result("pipe_w", "ms", time, quiet_mode);
            close(fd);
        } 
        else {
            printf("Error: invalid parameter for pipe: %s\n", param);
            return 1;
        }
    } else 
    {
        printf("Error: invalid type: %s\n", type);
        return 1;
    }

    return 0;
}



void ipv4_tcp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){
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
        printf("please enter exit to get out from the chat! \n");

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
        clock_t start_time = clock();
        shutdown(sock, SHUT_WR);
        clock_t end_time = clock();
            // Report the result
        double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    }
}
    
    
void ipv4_udp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){
    int sock;
    struct sockaddr_in addr;
    bool is_client = strcmp(argv[1], "-c") == 0;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[is_client ? 3 : 2]));

    if (is_client) {
        addr.sin_addr.s_addr = inet_addr(argv[2]);
        printf("client is ready: \n");
    }
    else {
        addr.sin_addr.s_addr = INADDR_ANY;
        printf("server is ready: \n");
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) { //if the bind between the port the ip didnt sucess
            perror("bind");
            return 1;
        }
        printf("please enter exit to get out from the chat! \n");
    }

    char buffer[BUFFER_SIZE];
    ssize_t read_size;

    while ((read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        buffer[read_size] = '\0'; // Ensure null termination for string comparison

        // Check if the user typed "exit"
        if (strcmp(buffer, "exit\n") == 0) {
            printf("client leaves the chat...\n");
            exit(0); // Exit the program
        }
        if (is_client) {
            sendto(sock, buffer, read_size, 0, (struct sockaddr *)&addr, sizeof(addr));
        }
        else {
            socklen_t len = sizeof(addr);
            sendto(sock, buffer, read_size, 0, (struct sockaddr *)&addr, len);
        }
    }
}

void ipv6_tcp(char *type, char *param, int perform_test, char *ip, char *port, int quiet_mode, int argc, char *argv[]) {
    // Generate data
    char *data = malloc(CHUNK_SIZE);
    if (data == NULL) {
        perror("malloc");
        return;
    }

    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        data[i] = rand() % 256;
    }

    // Compute checksum
    unsigned char checksum[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, data, CHUNK_SIZE);
    SHA256_Final(checksum, &sha256_ctx);

    // Create socket
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        free(data);
        return;
    }

    // Set socket options
    int optval = 1;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));

    // Create address structure
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    int status = getaddrinfo(ip, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        close(sockfd);
        free(data);
        return;
    }

    // Connect to server
    clock_t start_time = clock();
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sockfd);
        freeaddrinfo(res);
        free(data);
        return;
    }
    double elapsed_time = ((double) (clock() - start_time)) / CLOCKS_PER_SEC * 1000; // convert to milliseconds

    // Send data
    int sent_bytes = send(sockfd, data, CHUNK_SIZE, 0);
    if (sent_bytes < 0) {
        perror("send");
        close(sockfd);
        freeaddrinfo(res);
        free(data);
        return;
    }

    // Receive data
    char recv_buffer[BUFFER_SIZE];
    int received_bytes = 0;
    while (received_bytes < SHA256_DIGEST_LENGTH) {
        int bytes = recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
        if (bytes < 0) {
            perror("recv");
            close(sockfd);
            freeaddrinfo(res);
            free(data);
            return;
        }
        memcpy(checksum + received_bytes, recv_buffer, bytes);
        received_bytes += bytes;
    }
    if (!quiet_mode)
    {
    printf("IPv6 TCP test:\n");
    printf("  Data size: %d bytes\n", CHUNK_SIZE);
    printf("  Elapsed time: %.3f ms\n", elapsed_time);
    }

    // Cleanup
    close(sockfd);
    freeaddrinfo(res);
    free(data);
} 
        
void print_test_result(char *test_name, char *unit, double time, int quiet_mode) {
    if (!quiet_mode) {
        printf("%s: %.3f %s\n", test_name, time, unit);
    }
}

void ipv6_udp(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

    // Generate data
    char *data = malloc(CHUNK_SIZE);
    if (data == NULL) {
        perror("malloc");
        return;
    }
    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        data[i] = rand() % 256;
    }

    // Generate checksum
    unsigned char checksum[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, data, CHUNK_SIZE);
    SHA256_Final(checksum, &sha256_ctx);

    // Create socket
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return;
    }

    // Enable both IPv4 and IPv6 connections
    int val = 1;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val)) == -1) {
        perror("setsockopt");
        return;
    }

    // Get address information for the server
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(ip, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        return;
    }

    // Send data to server
    clock_t start_time = clock();
    ssize_t sent_size = sendto(sockfd, data, CHUNK_SIZE, 0, res->ai_addr, res->ai_addrlen);
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;

    if (sent_size == -1) {
        perror("sendto");
        return;
    }

    // Receive checksum from server
    unsigned char received_checksum[SHA256_DIGEST_LENGTH];
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    ssize_t recv_size = recvfrom(sockfd, received_checksum, SHA256_DIGEST_LENGTH, 0, (struct sockaddr *)&hints, &addr_len);
    if (recv_size == -1) {
        perror("recvfrom");
        return;
    }

    // Verify checksum
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, received_checksum, SHA256_DIGEST_LENGTH);
    SHA256_Final(received_checksum, &sha256_ctx);
    if (memcmp(checksum, received_checksum, SHA256_DIGEST_LENGTH) != 0) {
        fprintf(stderr, "Checksums do not match!\n");
    }

    // Report result
    if (!quiet_mode){
            print_test_result("ipv6_udp", "ms", elapsed_time, quiet_mode);

    // Clean up
    freeaddrinfo(res);
    close(sockfd);
    free(data);

    }
}
void uds_dgram(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

}
void uds_stream(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

}
void mmapfileName(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

}
void pipe_r(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

}
void pipe_w(char *type ,char *param , int perform_test, char *ip, char *port,int quiet_mode,int argc, char *argv){

}
    
