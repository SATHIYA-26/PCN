
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080
#define SIZE 1024

// CRC-8 calculation
unsigned char crc8(const unsigned char *data, int len)
{
    unsigned char crc = 0x00;

    for (int i = 0; i < len; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }

    return crc;
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    char buffer[SIZE];
    unsigned char received_crc, calculated_crc;
    struct timespec start, end;

    long total_bytes = 0;
    int packets = 0, errors = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("CRC Server waiting on port %d...\n", PORT);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client,
                       &client_len);

    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        int n = recv(client_fd, buffer, SIZE, 0);

        if (n <= 0)
            break;

        // Each message contains payload followed by one CRC byte.
        if (n < 2) {
            errors++;
            continue;
        }

        received_crc = (unsigned char)buffer[n - 1];
        calculated_crc = crc8((unsigned char *)buffer, n - 1);

        packets++;
        total_bytes += n - 1;

        if (received_crc == calculated_crc)
            printf("Packet %d: CRC OK\n", packets);
        else {
            printf("Packet %d: CRC ERROR\n", packets);
            errors++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    double throughput =
        elapsed > 0 ? total_bytes * 8.0 / elapsed / 1e6 : 0;

    double error_rate =
        packets > 0 ? errors * 100.0 / packets : 0;

    printf("\n--- Server Performance Metrics ---\n");
    printf("Packets received: %d\n", packets);
    printf("CRC errors: %d\n", errors);
    printf("CRC error rate: %.2f%%\n", error_rate);
    printf("Payload received: %ld bytes\n", total_bytes);
    printf("Elapsed time: %.6f seconds\n", elapsed);
    printf("Throughput: %.4f Mbps\n", throughput);

    close(client_fd);
    close(server_fd);

    return 0;
}
