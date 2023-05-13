#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <poll.h>
#include <math.h>    
#include <time.h>    
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#define MAX_EVENTS 1 
#define SOCKET_PATH "/tmp/file.txt"
#define BUFFER_SIZE 1024
#define FILE_NAME "file.txt"
#define FIFO_NAME "myfifo"


int server_main(int argc,char* argv[]);
long s_ipv4_tcp(char* port);
long s_ipv6_tcp(char* port);
long s_ipv4_udp(char* port);
long s_ipv6_udp(char* port);
long s_uds_dgram();
long s_uds_stream();
long s_mmap(char* port);
long s_pipe();
unsigned short compute_checksum(const void* buffer, size_t length);

unsigned short compute_checksum(const void* buffer, size_t length) {
    unsigned long sum = 0;
    const unsigned short* buf = buffer;
    while (length > 1) {
        sum += *buf++;
        length -= 2;
    }
    if (length > 0) {
        sum += *((unsigned char*)buf);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)~sum;
}


long s_ipv4_tcp(char* port) {
    int server_socket, client_socket, err;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), '\0', 8);

    err = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(server_socket, 1);
    if (err == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    client_addr_len = sizeof(struct sockaddr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    ssize_t totalBytesRead = 0;
    ssize_t bytesRead;

    while (1) {
        bytesRead = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead == 0)
            break;
        else if (bytesRead == -1) {
            perror("recv");
            close(client_socket);
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        totalBytesRead += bytesRead;
        memset(buffer, 0, sizeof(buffer));
    }

    gettimeofday(&end_time, NULL);
    long start_usec = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_usec = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_usec - start_usec;

    close(client_socket);
    close(server_socket);

    return time;
}

long s_ipv6_tcp(char *port) {
    int server_socket, client_socket, err;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    server_addr.sin6_addr = in6addr_any;

    err = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in6));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(server_socket, 1);
    if (err == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    client_addr_len = sizeof(struct sockaddr_in6);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    ssize_t totalBytesRead = 0;
    ssize_t bytesRead;

    while (1) {
        bytesRead = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0)
            break;
        totalBytesRead += bytesRead;
        memset(buffer, 0, sizeof(buffer));
    }

    gettimeofday(&end_time, NULL);
    long start_usec = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_usec = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_usec - start_usec;

    close(client_socket);
    close(server_socket);

    return time;
}



long s_ipv4_udp(char* port) {
    int socket_fd, err;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), '\0', 8);

    err = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int checksum = 0;
    ssize_t totalBytesRead = 0;

    while (1) {
        client_addr_len = sizeof(struct sockaddr);
        ssize_t bytesRead = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytesRead == -1) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        if (strcmp(buffer, "exit\n") == 0) {
            break;
        }
        totalBytesRead += bytesRead;
        checksum += compute_checksum(buffer, bytesRead);
        memset(buffer, 0, sizeof(buffer));
    }

    gettimeofday(&end_time, NULL);
    long start_usec = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_usec = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_usec - start_usec;

    close(socket_fd);

    return time;
}



long s_ipv6_udp(char* port) {
    int socket_fd, err;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    server_addr.sin6_addr = in6addr_any;

    err = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in6));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[MAX_EVENTS];
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int checksum = 0;
    while (1) {
        int ret = poll(fds, MAX_EVENTS, -1);
        if (ret == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (fds[0].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            client_addr_len = sizeof(struct sockaddr_in6);
            err = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
            checksum += err;
            if (err == -1) {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
            if (strcmp(buffer, "exit\n") == 0) {
                break;
            }
        }
    }

    gettimeofday(&end_time, NULL);
    long start_sec = start_time.tv_sec;
    long start_usec = start_time.tv_usec;
    long end_sec = end_time.tv_sec;
    long end_usec = end_time.tv_usec;
    long start_time_micros = start_sec * 1000000 + start_usec;
    long end_time_micros = end_sec * 1000000 + end_usec;
    long time = end_time_micros - start_time_micros;

    close(socket_fd);
    return time;
}

long s_uds_dgram() {
    int socket_fd, err;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    err = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int checksum = 0;
    ssize_t totalBytesRead = 0;

    while (1) {
        client_addr_len = sizeof(struct sockaddr_un);
        ssize_t bytesRead = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytesRead == -1) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        if (strcmp(buffer, "exit\n") == 0) {
            break;
        }
        totalBytesRead += bytesRead;
        checksum += compute_checksum(buffer, bytesRead);
        memset(buffer, 0, sizeof(buffer));
    }

    gettimeofday(&end_time, NULL);
    long start_usec = start_time.tv_sec * 1000000 + start_time.tv_usec;
    long end_usec = end_time.tv_sec * 1000000 + end_time.tv_usec;
    long time = end_usec - start_usec;

    close(socket_fd);
    unlink(SOCKET_PATH);

    return time;
}

long s_uds_stream() {
    int server_socket, client_socket, err;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    err = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un));
    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(server_socket, 1);
    if (err == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    client_addr_len = sizeof(struct sockaddr_un);
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    ssize_t totalBytesRead = 0;
    ssize_t bytesRead;
    while ((bytesRead = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        totalBytesRead += bytesRead;
        memset(buffer, 0, sizeof(buffer));
    }
    if (bytesRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&end_time, NULL);
    long start_sec = start_time.tv_sec;
    long start_usec = start_time.tv_usec;
    long end_sec = end_time.tv_sec;
    long end_usec = end_time.tv_usec;
    long start_time_micros = start_sec * 1000000 + start_usec;
    long end_time_micros = end_sec * 1000000 + end_usec;
    long time = end_time_micros - start_time_micros;

    close(client_socket);
    close(server_socket);

    return time;
}

long s_mmap(char* port) {
    struct sockaddr_in serv_addr, client_addr;
    char buffer[BUFFER_SIZE];
    struct pollfd fds[1];
    int timeout = 5000;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int bite = 0;

    while (1) {
        int ret = poll(fds, 1, timeout);
        if (ret == -1) {
            perror("poll");
            close(sockfd);
            return -1;
        } else if (ret == 0) {
            break;
        } else {
            if (fds[0].revents & POLLIN) {
                memset(buffer, 0, BUFFER_SIZE);
                socklen_t clientlen = sizeof(client_addr);
                int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &clientlen);
                bite += n;
                if (n < 0) {
                    perror("recvfrom");
                    close(sockfd);
                    return -1;
                }
            }
        }
    }

    gettimeofday(&end_time, NULL);

    long start_sec = start_time.tv_sec;
    long start_usec = start_time.tv_usec;
    long end_sec = end_time.tv_sec;
    long end_usec = end_time.tv_usec;
    long start_time_micros = start_sec * 1000000 + start_usec;
    long end_time_micros = end_sec * 1000000 + end_usec;
    long time = end_time_micros - start_time_micros;

    close(sockfd);

    return time;
}

long s_pipe() {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    mkfifo(FIFO_NAME, 0666);
    fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open named pipe");
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen("received_file.txt", "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int checksum = 0;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        checksum += bytes_read;
        fwrite(buffer, 1, bytes_read, file);
    }

    gettimeofday(&end_time, NULL);

    long start_sec = start_time.tv_sec;
    long start_usec = start_time.tv_usec;
    long end_sec = end_time.tv_sec;
    long end_usec = end_time.tv_usec;
    long start_time_micros = start_sec * 1000000 + start_usec;
    long end_time_micros = end_sec * 1000000 + end_usec;
    long time = end_time_micros - start_time_micros;

    fclose(file);
    close(fd);
    unlink(FIFO_NAME);

    return time;
}


int server_main(int argc, char *argv[]) {
    if (argc < 5) {
        exit(1);
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char *port = argv[2];

    char buffer[2][1024];

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);

    // Bind the socket to the server address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept a client connection
    addr_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Receive message1 from the client
    ssize_t num_bytes = recv(client_fd, buffer[0], sizeof(buffer[0]) - 1, 0);
    if (num_bytes == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buffer[0][num_bytes] = '\0';

    // Receive message2 from the client
    num_bytes = recv(client_fd, buffer[1], sizeof(buffer[1]) - 1, 0);
    if (num_bytes == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buffer[1][num_bytes] = '\0';

    // Close the connection
    close(client_fd);
    close(server_fd);

    //-----------------------------------------------------------------

    long time = 0;
    char *protocol = NULL;

    if (strcmp(buffer[0], "ipv4") == 0 && strcmp(buffer[1], "tcp") == 0) {
        time = s_ipv4_tcp(port);
        protocol = "ipv4_tcp";
    } else if (strcmp(buffer[0], "ipv4") == 0 && strcmp(buffer[1], "udp") == 0) {
        time = s_ipv4_udp(port);
        protocol = "ipv4_udp";
    } else if (strcmp(buffer[0], "ipv6") == 0 && strcmp(buffer[1], "tcp") == 0) {
        time = s_ipv6_tcp(port);
        protocol = "ipv6_tcp";
    } else if (strcmp(buffer[0], "ipv6") == 0 && strcmp(buffer[1], "udp") == 0) {
        time = s_ipv6_udp(port);
        protocol = "ipv6_udp";
    } else if (strcmp(buffer[0], "mmap") == 0 && strcmp(buffer[1], "filename") == 0) {
        time = s_mmap(port);
        protocol = "mmap";
    } else if (strcmp(buffer[0], "pipe") == 0 && strcmp(buffer[1], "filename") == 0) {
        time = s_pipe();
        protocol = "pipe";
     } else if (strcmp(buffer[0], "uds") == 0 && strcmp(buffer[1], "dgram") == 0) {
        time = s_uds_dgram();
        protocol = "uds_dgram";
    } else if (strcmp(buffer[0], "uds") == 0 && strcmp(buffer[1], "stream") == 0) {
        time = s_uds_stream();
        protocol = "uds_stream";
    } else {
        exit(1);
    }

    printf("%s,%ld\n", protocol, time);

    return 0;
}






