#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <poll.h>

int client_main(int argc,char* argv[]);
int c_ipv4_tcp(char* ip, char* port);
void c_ipv6_tcp(char* ip, char* port);
int c_ipv4_udp(char* ip, char* port);
int c_ipv6_udp(char* ip, char* port);
int c_uds_dgram();
int c_uds_stream();
int c_mmap(char* ip, char* port);
int c_pipe();


#define SOCKET_PATH "/tmp/file.txt"
#define BUFFER_SIZE 1024
#define FILE_NAME "file.txt"
#define FIFO_NAME "myfifo"
#define MAX_EVENTS 2 

int c_ipv4_tcp(char* ip, char* port) {
    int client_socket, err;
    struct sockaddr_in server_addr;
    FILE *fp;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to the server
    err = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (err == -1) {
        perror("connect");
        exit(1);
    }

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Send data to the server
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            exit(1);
        }
    }

    // Close the file and socket
    fclose(fp);
    close(client_socket);

    return 0;
}



void c_ipv6_tcp(char* ip, char* port) {
    int client_socket, err;
    struct sockaddr_in6 server_addr;
    char buffer[1024];
    FILE *fp;

    // Create a socket
    client_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, &server_addr.sin6_addr);
    server_addr.sin6_port = htons(atoi(port));

    // Connect to the server
    err = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        exit(1);
    }

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Send data to the server
    size_t nbytes;
    size_t nbytes2;

    while ((nbytes = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
        nbytes2 = send(client_socket, buffer, nbytes, 0);
        if (nbytes2 < 0) {
            perror("send");
            exit(1);
        }
    }

    // Close the file and socket
    fclose(fp);
    close(client_socket);
}
int c_ipv4_udp(char* ip, char* port) {
    int client_socket, err;
    struct sockaddr_in server_addr;
    FILE *fp;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Send file contents line by line
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        err = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        if (err == -1) {
            perror("sendto");
            exit(1);
        }
    }

    // Send "exit" message
    strcpy(buffer, "exit\n");
    err = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (err == -1) {
        perror("sendto");
        exit(1);
    }

    // Close the file and socket
    fclose(fp);
    close(client_socket);

    return 0;
}

int c_ipv6_udp(char* ip, char* port) {
    int client_socket, err;
    struct sockaddr_in6 server_addr;
    FILE *fp;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    inet_pton(AF_INET6, ip, &(server_addr.sin6_addr));

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Send file contents line by line
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        err = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in6));
        if (err == -1) {
            perror("sendto");
            exit(1);
        }
    }

    // Send "exit" message
    strcpy(buffer, "exit\n");
    err = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in6));
    if (err == -1) {
        perror("sendto");
        exit(1);
    }

    // Close the file and socket
    fclose(fp);
    close(client_socket);

    return 0;
}

int c_uds_dgram() {
    int client_socket, err;
    struct sockaddr_un server_addr;
    FILE *fp;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Read file contents and send them in a single datagram
    size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fp);
    if (bytes_read > 0) {
        ssize_t num_bytes = sendto(client_socket, buffer, bytes_read, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
        if (num_bytes == -1) {
            perror("sendto");
            exit(1);
        }
    }

    fclose(fp);
    close(client_socket);

    return 0;
}



int c_uds_stream() {
    int client_socket, err;
    struct sockaddr_un server_addr;
    FILE *fp;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Connect to the server
    err = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
    if (err == -1) {
        perror("connect");
        exit(1);
    }

    // Open the file
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Read file contents and send them
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        ssize_t num_bytes = send(client_socket, buffer, strlen(buffer), 0);
        if (num_bytes == -1) {
            perror("send");
            exit(1);
        }
    }

    fclose(fp);
    close(client_socket);

    return 0;
}


int c_mmap(char* ip, char* port) {
    struct sockaddr_in serv_addr;
    int sockfd, fd;
    char buffer[1024];
    ssize_t bytes_read, bytes_sent;

    // Open the file
    fd = open("file.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        close(fd);
        return -1;
    }

    // Set up the server address
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        close(fd);
        close(sockfd);
        return -1;
    }

    // Read and send the file data in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        bytes_sent = send(sockfd, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("send");
            close(fd);
            close(sockfd);
            return -1;
        }
    }

    close(fd);
    close(sockfd);
    return 0;
}


int c_pipe() {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    FILE* file;
    file = fopen("file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open named pipe");
        exit(EXIT_FAILURE);
    }
    while (!feof(file)) {
        bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        if (bytes_read > 0) {
            bytes_written = write(fd, buffer, bytes_read);
            if (bytes_written == -1) {
                perror("write");
                fclose(file);
                close(fd);
                return -1;
            }
        }
    }
    fclose(file);
    close(fd);
    return 0;
}

int client_main(int argc, char *argv[]) {
    if (argc < 7) {
        exit(1);
    }

    char message[2][1024];
    strcpy(message[0], argv[5]);
    strcpy(message[1], argv[6]);

    int client_fd;
    struct sockaddr_in server_addr;

    // Create a socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8000);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    // Send data to the server
    if (send(client_fd, message[0], strlen(message[0]), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    // Wait for a short period
    usleep(100000);

    if (send(client_fd, message[1], strlen(message[1]), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    // Close the connection
    close(client_fd);

    // Wait for a short period
    usleep(100000);
    
    //------------------------------------------------

    if (strcmp(argv[5], "ipv4") == 0 && strcmp(argv[6], "tcp") == 0) {
        c_ipv4_tcp(argv[2], argv[3]);
    } else if (strcmp(argv[5], "ipv4") == 0 && strcmp(argv[6], "udp") == 0) {
        c_ipv4_udp(argv[2], argv[3]);
    } else if (strcmp(argv[5], "ipv6") == 0 && strcmp(argv[6], "tcp") == 0) {
        c_ipv6_tcp(argv[2], argv[3]);
    } else if (strcmp(argv[5], "ipv6") == 0 && strcmp(argv[6], "udp") == 0) {
        c_ipv6_udp(argv[2], argv[3]);
    } else if (strcmp(argv[5], "mmap") == 0 && strcmp(argv[6], "filename") == 0) {
        c_mmap(argv[2], argv[3]);
    } else if (strcmp(argv[5], "pipe") == 0 && strcmp(argv[6], "filename") == 0) {
        c_pipe(argv[2], argv[3]);
    } else if (strcmp(argv[5], "uds") == 0 && strcmp(argv[6], "dgram") == 0) {
        c_uds_dgram();
    } else if (strcmp(argv[5], "uds") == 0 && strcmp(argv[6], "stream") == 0) {
        c_uds_stream();
    } else {
        exit(1);
    }

    return 0;
}
