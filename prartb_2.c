#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <sys/time.h>
#define BUFFER_SIZE 1024
#define DATA_SIZE (1024 * 1024 * 100)
#define HASH_SIZE EVP_MAX_MD_SIZE

// void calculate_md5(const void *data, size_t size, unsigned char *md) {
//     EVP_MD_CTX *md_ctx;
//     const EVP_MD *md_type;
//     unsigned int md_len;

//     md_ctx = EVP_MD_CTX_new();
//     md_type = EVP_md5();

//     EVP_DigestInit_ex(md_ctx, md_type, NULL);
//     EVP_DigestUpdate(md_ctx, data, size);
//     EVP_DigestFinal_ex(md_ctx, md, &md_len);

//     EVP_MD_CTX_free(md_ctx);
// }



// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}

void perform_ipv4_tcp_client_test(const char *ip, const char *port, const char *filename, bool is_quiet_mode) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    struct timeval start_time, end_time;
    double elapsed_time;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, ip, &(server_addr.sin_addr)) <= 0) {
        perror("inet_pton");
        close(sock);
        return;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        return;
    }

    // Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        close(sock);
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file data
    char *data = malloc(filesize);
    if (data == NULL) {
        printf("Failed to allocate memory for data.\n");
        fclose(file);
        close(sock);
        return;
    }

    // Read the file into the buffer
    fread(data, filesize, 1, file);
   

    // Generate checksum (hash) for the data
    unsigned short checksum = calculate_checksum((unsigned short *)data, filesize);

    // Convert the checksum to network byte order (big-endian)
    unsigned short checksum_network_order = htons(checksum);

    // Start the timer
    gettimeofday(&start_time, NULL);

    // Transmit the checksum and data to the server
    if (send(sock, &checksum_network_order, sizeof(checksum_network_order), 0) == -1) {
        perror("send");
        free(data);
        close(sock);
        return;
    }

    // Transmit the data to the server
    if (send(sock, data, filesize, 0) == -1) {
        perror("send");
        free(data);
        close(sock);
        return;
    }

    // Receive the result from the server
    ssize_t recv_size = recv(sock, buffer, BUFFER_SIZE, 0);

    // Stop the timer
    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                   (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    // Report the result
    if (!is_quiet_mode) {
        printf("IPv4 TCP Performance Test\n");
        printf("IP: %s\n", ip);
        printf("Port: %s\n", port);
        printf("Data Size: %ld bytes\n", filesize);
        printf("Elapsed Time: %.2f ms\n", elapsed_time);
        printf("Received Size: %zd bytes\n", recv_size);
        // Print the checksum (hash) if available
        printf("Checksum: %hu\n", checksum);
    } else {
        printf("ipv4_tcp,%ld,%.2f\n", filesize, elapsed_time);
    }

    // Clean up
    close(sock);
    free(data);
    fclose(file);
}


void perform_ipv4_udp_client_test(const char *ip, const char *port, bool is_quiet_mode) {
    // Create UDP socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return;
    }

    // Set up server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, ip, &(server_addr.sin_addr)) <= 0) {
        perror("inet_pton");
        close(sock);
        return;
    }

    // Generate data
    char data[DATA_SIZE];
    memset(data, 'A', DATA_SIZE);

    // Calculate checksum
    unsigned short checksum = calculate_checksum((unsigned short *)data, DATA_SIZE);


    // Start the timer
    clock_t start_time = clock();

  ssize_t sent_bytes = sendto(sock, &checksum, sizeof(checksum), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (sent_bytes == -1) {
        perror("sendto");
        close(sock);
        return;
    }

    sent_bytes = sendto(sock, data, DATA_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (sent_bytes == -1) {
        perror("sendto");
        close(sock);
        return;
    }

    // Receive acknowledgement from the server
    char ack;
    ssize_t recv_bytes = recvfrom(sock, &ack, sizeof(ack), 0, NULL, NULL);
    if (recv_bytes == -1) {
        perror("recvfrom");
        close(sock);
        return;
    }

    // Calculate elapsed time in milliseconds
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    // Close the socket
    close(sock);

    // Print the result
    if (!is_quiet_mode) {
        printf("IPv4 UDP Performance Test:\n");
        printf("Sent %zd bytes to %s:%s\n", sent_bytes, ip, port);
        printf("Received acknowledgement: %c\n", ack);
        printf("Elapsed time: %.2f ms\n", elapsed_time);
        printf("-----------------------------\n");
    } else {
        printf("ipv4_udp,%ld\n", (long)elapsed_time);
    }
}

void perform_ipv6_tcp_client_test(const char* ip, const char* port, bool is_quiet_mode) {
    // Create socket
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return;
    }

    // Set up server address
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    if (inet_pton(AF_INET6, ip, &(server_addr.sin6_addr)) <= 0) {
        perror("inet_pton");
        close(sock);
        return;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        return;
    }

    // Generate test data
    char* test_data = (char*)malloc(DATA_SIZE);
    if (test_data == NULL) {
        perror("malloc");
        close(sock);
        return;
    }

    memset(test_data, 'A', DATA_SIZE);

    // Perform the performance test
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // ssize_t total_bytes_sent = 0;
    ssize_t bytes_sent;
    // while (total_bytes_sent < DATA_SIZE) {
    //     bytes_sent = send(sock, test_data + total_bytes_sent, DATA_SIZE - total_bytes_sent, 0);
    //     if (bytes_sent == -1) {
    //         perror("send");
    //         close(sock);
    //         free(test_data);
    //         return;
    //     }
    //     total_bytes_sent += bytes_sent;
    // }

   

    // Calculate checksum (hash) for the test data (not shown here)
    unsigned short checksum = calculate_checksum((unsigned short*)test_data, DATA_SIZE);
    
     // Include the checksum in the data
    memcpy(test_data + DATA_SIZE - sizeof(checksum), &checksum, sizeof(checksum));

        // Send the data with checksum
    bytes_sent = send(sock, test_data, DATA_SIZE, 0);
    if (bytes_sent == -1) {
        perror("send");
        close(sock);
        free(test_data);
        return;
    }
     gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                          (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    // Report the result

    if (!is_quiet_mode) {
        printf("IPv6 TCP Performance Test\n");
        printf("Data Size: %d bytes\n", DATA_SIZE);
        printf("Elapsed Time: %.2f ms\n", elapsed_time);
        printf("Checksum: %04X\n", checksum);
    } else {
        printf("ipv6_tcp,%0.2f\n", elapsed_time);
    }

    // Clean up resources
    close(sock);
    free(test_data);
}


void perform_ipv6_udp_client_test(char* ip, char* port, bool is_quiet_mode) {
    int sockfd;
    struct sockaddr_in6 server_addr;
    char buffer[BUFFER_SIZE];
    struct timeval start_time, end_time;
    double elapsed_time;
    
    // Create socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return;
    }
    
    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return;
    }
    
    // Generate data
    char* data = (char*)malloc(DATA_SIZE);
    memset(data, 'A', DATA_SIZE);
    
    // Calculate checksum (hash) for data (optional)
    unsigned short checksum = calculate_checksum((unsigned short*)data, DATA_SIZE);
    memcpy(data + DATA_SIZE - sizeof(checksum), &checksum, sizeof(checksum));

    
    // Start timer
    gettimeofday(&start_time, NULL);
    
    // Send data
    ssize_t sent_bytes = sendto(sockfd, data, DATA_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent_bytes < 0) {
        perror("Error sending data");
        close(sockfd);
        free(data);
        return;
    }
    
    // Receive response (optional)
    // ...
    
    // Stop timer
    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                   (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    // Report result
    if (!is_quiet_mode) {
        printf("IPv6 UDP performance test:\n");
        printf("Server: %s\n", ip);
        printf("Port: %s\n", port);
        printf("Data size: %d bytes\n", DATA_SIZE);
        printf("Elapsed time: %.2f ms\n", elapsed_time);
    } else {
        printf("ipv6_udp,%s\n", port);
        printf("%.0f\n", elapsed_time);
    }
    
    // Cleanup
    close(sockfd);
    free(data);
}

void perform_uds_dgram_client_test(const char *ip, const char *port, bool is_quiet_mode) {
    struct sockaddr_un server_addr;
    int sock;
    char buffer[BUFFER_SIZE];
    struct timeval start_time, end_time;
    double elapsed_time;

    // Create socket
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return;
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, ip, sizeof(server_addr.sun_path) - 1);

    // Generate test data
    char *data = (char *)malloc(DATA_SIZE);
    memset(data, 'A', DATA_SIZE);

    // Calculate checksum of the test data (optional)
      unsigned short checksum = calculate_checksum((unsigned short *)data, DATA_SIZE);
    
    // Create a buffer to hold the test data and checksum
    char send_buffer[DATA_SIZE + sizeof(checksum)];

    // Copy the test data into the send buffer
    memcpy(send_buffer, data, DATA_SIZE);

    // Copy the checksum into the send buffer after the test data
    memcpy(send_buffer + DATA_SIZE, &checksum, sizeof(checksum));
    // Start the timer
    gettimeofday(&start_time, NULL);

     // Send the test data and checksum to the server
    ssize_t sent_bytes = sendto(sock, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (sent_bytes == -1) {
        perror("sendto");
        free(data);
        close(sock);
        return;
    }

    // Receive the test data back from the server (optional)
    // ssize_t received_bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
    // ...

    // Stop the timer
    gettimeofday(&end_time, NULL);

    // Calculate elapsed time
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                   (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    // Report the result
    if (!is_quiet_mode) {
        printf("UDS Datagram Performance Test\n");
        printf("Test Data Size: %d bytes\n", DATA_SIZE);
        printf("Elapsed Time: %.2f ms\n", elapsed_time);
        // ...
    } else {
        printf("uds_dgram,%0.2f\n", elapsed_time);
    }

    // Clean up
    free(data);
    close(sock);
}


void perform_uds_stream_client_test(const char *ip, const char *port, bool is_quiet_mode) {
    // Create a UDS Stream socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return;
    }

    // Set up the server address
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, ip);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return;
    }

    // Generate a chunk of data with 100MB size
    char *data = (char *)malloc(100 * 1024 * 1024); // Allocate 100MB
    memset(data, 'A', 100 * 1024 * 1024); // Fill with 'A' characters

    // Generate a checksum (hash) for the data
    unsigned short checksum = calculate_checksum((unsigned short *)data, 100 * 1024 * 1024);


    // Copy the checksum into the data buffer
    memcpy(data + 100 * 1024 * 1024, &checksum, sizeof(unsigned short));

    // Start the timer
   clock_t start_time = clock();

    // Transmit the data and checksum
    ssize_t total_sent = 0;
    ssize_t remaining = 100 * 1024 * 1024; // Total data size to send
    ssize_t sent;
    while (remaining > 0) {
        sent = send(sockfd, data + total_sent, remaining, 0);
        if (sent == -1) {
            perror("send");
            break;
        }
        total_sent += sent;
        remaining -= sent;
    }

    // Stop the timer
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    // double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
    //                       (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0;

    // Report the result
    if (!is_quiet_mode) {
        printf("UDS Stream Performance Test\n");
        printf("Data Size: %d bytes\n", 100 * 1024 * 1024);
        printf("Elapsed Time: %.2f ms\n", elapsed_time);
    } else {
        printf("uds_stream,%d\n", (int)elapsed_time);
    }

    // Clean up
    close(sockfd);
    free(data);
}

void perform_mmap_client_test(char *filename, bool is_quiet_mode) {
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return;
    }

    // Extend the file to the desired size
    if (ftruncate(fd, DATA_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return;
    }

    // Map the file into memory
    void *data = mmap(NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    // Generate random data
    memset(data, 'A', DATA_SIZE);

    // Generate a checksum (hash) for the data
     // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    // Define the server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345);  // Port number goes here
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {  // IP address goes here
        perror("inet_pton");
        return;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return;
    }
unsigned short checksum = calculate_checksum((unsigned short*)data, DATA_SIZE / sizeof(unsigned short));
 // Create a buffer large enough to hold the data and the checksum
    char *send_buffer = malloc(DATA_SIZE + sizeof(checksum));
    if (send_buffer == NULL) {
        perror("malloc");
        return;
    }

    // Copy the data into the buffer
    memcpy(send_buffer, data, DATA_SIZE);

    // Copy the checksum into the buffer, after the data
    unsigned short network_order_checksum = htons(checksum);  // Convert to network byte order
    memcpy(send_buffer + DATA_SIZE, &network_order_checksum, sizeof(network_order_checksum));


    // Transmit the data with the selected communication style, while measuring the time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // TODO: Send the data over the desired communication style
 if (send(sockfd, send_buffer, DATA_SIZE + sizeof(network_order_checksum), 0) < 0) {
        perror("send buffer");
        return;
    }
    gettimeofday(&end_time, NULL);

    // Calculate elapsed time in milliseconds
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                          (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    // Report the result
    if (!is_quiet_mode) {
        printf("MMAP performance test:\n");
        printf("File: %s\n", filename);
        printf("Data size: %d bytes\n", DATA_SIZE);
    
        printf("\n");
        printf("Elapsed time: %.2f ms\n", elapsed_time);
    } else {
        // Print the result in a compact format for quiet mode
        printf("mmap,%s,%.2f\n", filename, elapsed_time);
    }

    // Clean up
    munmap(data, DATA_SIZE);
    free(send_buffer);
    close(fd);
}


void perform_pipe_client_test(const char *pipe_name, bool is_quiet_mode) {
    // Generate 100MB data chunk
    const int CHUNK_SIZE = 100 * 1024 * 1024;
    char *data = (char *)malloc(CHUNK_SIZE);
    memset(data, 'A', CHUNK_SIZE);

    // Calculate checksum (RFC 1071) for the data
    unsigned short checksum = calculate_checksum((unsigned short*)data, CHUNK_SIZE / sizeof(unsigned short));

  
 

    int pipe_fd = open(pipe_name, O_WRONLY);
    if (pipe_fd == -1) {
        perror("open");
        free(data);
        return;
    }
    clock_t start_time = clock();

    // Transmit the data through the named pipe
    ssize_t total_written = 0;
    ssize_t written;
    while (total_written < CHUNK_SIZE) {
        written = write(pipe_fd, data + total_written, CHUNK_SIZE - total_written);
        if (written == -1) {
            perror("write");
            close(pipe_fd);
            free(data);
            return;
        }
        total_written += written;
    }

    // Write checksum
    written = write(pipe_fd, &checksum, sizeof(checksum));
    if (written == -1) {
        perror("write");
        close(pipe_fd);
        free(data);
        return;
    }


    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    close(pipe_fd);
    free(data);

    // Report the result
    if (!is_quiet_mode) {
        printf("Named Pipe Performance Test\n");
        printf("Transmitted data size: %d bytes\n", CHUNK_SIZE);
        printf("Elapsed time: %.2f ms\n", elapsed_time);
        // printf("Checksum: ");
        // for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        //     printf("%02x", hash[i]);
        // }
        // printf("\n");
    } else {
        // Print only the performance result
        printf("pipe,%s,%.2f\n", pipe_name, elapsed_time);
    }
}


void perform_server_performance_test(char *port, bool is_quiet_mode) {
    int server_sock;
    struct sockaddr_in server_addr;

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));

    // Bind socket to address
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sock);
        return;
    }

    // Listen for connections
    if (listen(server_sock, 1) == -1) {
        perror("listen");
        close(server_sock);
        return;
    }

    // Accept a client connection
    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("accept");
        close(server_sock);
        return;
    }

    // Prepare test data
    char *buffer = (char *)malloc(DATA_SIZE);
    if (!buffer) {
        perror("malloc");
        close(client_sock);
        close(server_sock);
        return;
    }

    // Generate test data
    memset(buffer, 'A', DATA_SIZE);
    unsigned short checksum = calculate_checksum((unsigned short *)buffer, DATA_SIZE / 2);
   
    char *buffer_with_checksum = (char *)malloc(DATA_SIZE + sizeof(checksum));
    if (!buffer_with_checksum) {
        perror("malloc");
        close(client_sock);
        close(server_sock);
        return;
    }

    memcpy(buffer_with_checksum, buffer, DATA_SIZE);
    memcpy(buffer_with_checksum + DATA_SIZE, &checksum, sizeof(checksum));


     // Start the timer
    clock_t start_time = clock();


    // Transmit the data
    ssize_t sent_size = 0;
    ssize_t remaining_size = DATA_SIZE + sizeof(checksum);
    ssize_t write_size;
   while (remaining_size > 0) {
    write_size = write(client_sock, buffer_with_checksum + sent_size, remaining_size);
    if (write_size == -1) {
        perror("write");
        break;
    }
    sent_size += write_size;
    remaining_size -= write_size;
}

    // Stop the timer
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    // Report the result
    if (!is_quiet_mode) {
        printf("Test completed.\n");
        printf("Data size: %d bytes\n", DATA_SIZE);
        printf("Elapsed time: %.0f ms\n", elapsed_time);
    } else {
        printf("performance_test,%s,%d\n", port, (int)elapsed_time);
    }

    // Clean up
    free(buffer);
    close(client_sock);
    close(server_sock);
}


void generateFile (){
    FILE * file = fopen("sample.txt","w");
    
    if (file == NULL) {
        perror("ERROR: Could not open file");
        return;
    }
    
    // Generate data and write to file
    char *data = malloc(DATA_SIZE);
    if (data == NULL) {
        printf("Failed to allocate memory for data.\n");
        fclose(file);
        return;
    }
    memset(data, 'A', DATA_SIZE);  // Fill data with 'A'
    
    // Write data to file
    if (fwrite(data, 1, DATA_SIZE, file) != DATA_SIZE) {
        printf("Failed to write data to file.\n");
        free(data);
        fclose(file);
        return;
    }

    // Write the string at the end of the file
    fputs("this is the end of the file", file);

    // Clean up
    free(data);
    fclose(file);
}



int main(int argc, char *argv[]) {

 // Check the number of command line arguments
    if (argc < 4) {
        printf("Usage:\n");
        printf("  Client: stnc -c IP PORT -p <type> <param>\n");
        printf("  Server: stnc -s port -p -q\n");
        return 1;
    }
    generateFile();
    bool is_client = (strcmp(argv[1], "-c") == 0);
    bool is_server = (strcmp(argv[1], "-s") == 0);
    bool is_performance_test = false;
    bool is_quiet_mode = false;

    // Check if performance test flag is set
    if (argc >= 5 && strcmp(argv[4], "-p") == 0) {
        is_performance_test = true;
    }

    // Check if quiet mode flag is set
    if (argc >= 6 && strcmp(argv[5], "-q") == 0) {
        is_quiet_mode = true;
    }

    if (is_client) {
        // Client-side implementation
        // Parse command line arguments
        char *ip = argv[2];
        char *port = argv[3];
        char *type = argv[5];
        char *param = argv[6];

        // Perform the performance test if requested
        if (is_performance_test) {
            if (strcmp(type, "ipv4") == 0 && strcmp(param, "tcp") == 0) {
                // Perform IPv4 TCP performance test
                perform_ipv4_tcp_client_test(ip, port,"sample.txt", is_quiet_mode);
            } else if (strcmp(type, "ipv4") == 0 && strcmp(param, "udp") == 0) {
                // Perform IPv4 UDP performance test
                perform_ipv4_udp_client_test(ip, port, is_quiet_mode);
            } else if (strcmp(type, "ipv6") == 0 && strcmp(param, "tcp") == 0) {
                // Perform IPv6 TCP performance test
                perform_ipv6_tcp_client_test(ip, port, is_quiet_mode);
            } else if (strcmp(type, "ipv6") == 0 && strcmp(param, "udp") == 0) {
                // Perform IPv6 UDP performance test
                perform_ipv6_udp_client_test(ip, port, is_quiet_mode);
            } else if (strcmp(type, "uds") == 0 && strcmp(param, "dgram") == 0) {
                // Perform UDS Datagram performance test
                perform_uds_dgram_client_test(ip, port, is_quiet_mode);
            } else if (strcmp(type, "uds") == 0 && strcmp(param, "stream") == 0) {
                // Perform UDS Stream performance test
                perform_uds_stream_client_test(ip, port, is_quiet_mode);
            } else if (strcmp(type, "mmap") == 0) {
                // Perform MMAP performance test
                perform_mmap_client_test(param, is_quiet_mode);
            } else if (strcmp(type, "pipe") == 0) {
                // Perform Named Pipe performance test
                perform_pipe_client_test(param, is_quiet_mode);
            } else {
                printf("Invalid performance test parameters.\n");
                return 1;
            }
        } else {
            printf("No performance test specified.\n");
            return 1;
        }
    }  else if (is_server) {
        // Server-side implementation
        // Parse command line arguments
        if (argc < 3) {
            printf("Server mode requires a port argument.\n");
            return 1;
        }
        char *port = argv[2];
        // Check if performance test flag is set
        if (is_performance_test) {
            if (is_quiet_mode) {
                // Perform the performance test in quiet mode
                perform_server_performance_test(port, is_quiet_mode);
            } else {
                printf("Invalid server options.\n");
                return 1;
            }
        } else {
            printf("No performance test specified.\n");
            return 1;
        }
    } else {
        printf("Invalid command.\n");
        return 1;
    }

    return 0;
}


