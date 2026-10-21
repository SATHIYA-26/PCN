
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080
#define SIZE 1024
#define COUNT 100

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

// Send all bytes, handling partial TCP writes
int send_all(int sock, const void *data, size_t len)
{
    const char *p = data;
    size_t sent = 0;

    while (sent < len) {
        ssize_t n = send(sock, p + sent, len - sent, 0);

        if (n <= 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

int main()
{
    int sock;
    struct sockaddr_in server;
    char message[SIZE];
    unsigned char packet[SIZE + 1];
    int packet_size;
    struct timespec start, end;

    long total_bytes = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server.sin_addr) != 1) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < COUNT; i++) {
        snprintf(message, sizeof(message),
                 "CRC test packet number %d", i + 1);

        int len = strlen(message);
        memcpy(packet, message, len);

        // Append CRC byte after the payload
        packet[len] = crc8((unsigned char *)message, len);
        packet_size = len + 1;

        if (send_all(sock, packet, packet_size) < 0) {
            perror("send");
            close(sock);
            return 1;
        }

        total_bytes += len;
    }

    // Half-close: tell server that no more data will be sent
    shutdown(sock, SHUT_WR);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    double throughput =
        elapsed > 0 ? total_bytes * 8.0 / elapsed / 1e6 : 0;

    printf("\n--- Client Performance Metrics ---\n");
    printf("Packets sent: %d\n", COUNT);
    printf("Payload sent: %ld bytes\n", total_bytes);
    printf("Elapsed time: %.6f seconds\n", elapsed);
    printf("Throughput: %.4f Mbps\n", throughput);

    close(sock);

    return 0;
}
