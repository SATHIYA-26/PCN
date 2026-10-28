
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#define PORT 8080
#define SIZE 100
#define MAX_CACHE 50
#define MAX_RETRIES 3

struct Cache {
    char domain[SIZE];
    char ip[SIZE];
};

struct Cache cache[MAX_CACHE];
int cache_count = 0;

/* Return elapsed time in milliseconds */
double elapsed_ms(struct timespec start,
                  struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

/* Search client cache */
int find_cache(char *domain, char *ip) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].domain, domain) == 0) {
            strcpy(ip, cache[i].ip);
            return 1;
        }
    }
    return 0;
}

/* Store successful lookup in cache */
void add_cache(char *domain, char *ip) {
    if (cache_count < MAX_CACHE) {
        strcpy(cache[cache_count].domain, domain);
        strcpy(cache[cache_count].ip, ip);
        cache_count++;
    }
}

int main() {
    int sockfd;
    char domain[SIZE], ip[SIZE], response[SIZE];

    struct sockaddr_in server;
    struct timeval timeout;
    struct timespec start, end;

    socklen_t len = sizeof(server);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /* Wait up to 2 seconds for each response */
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1",
              &server.sin_addr);

    while (1) {
        printf("Enter domain (or exit): ");
        if (scanf("%99s", domain) != 1)
            break;

        if (strcmp(domain, "exit") == 0)
            break;

        clock_gettime(CLOCK_MONOTONIC, &start);

        /* Step 1: Check cache */
        if (find_cache(domain, ip)) {
            clock_gettime(CLOCK_MONOTONIC, &end);

            printf("Cache HIT!\n");
            printf("IP Address: %s\n", ip);
            printf("Lookup time: %.3f ms\n\n",
                   elapsed_ms(start, end));
            continue;
        }

        printf("Cache MISS!\n");

        /* Step 2: Query server, retry up to 3 times */
        int found = 0;

        for (int attempt = 1;
             attempt <= MAX_RETRIES; attempt++) {

            memset(response, 0, sizeof(response));

            printf("Attempt %d of %d...\n",
                   attempt, MAX_RETRIES);

            if (sendto(sockfd, domain, strlen(domain), 0,
                       (struct sockaddr *)&server,
                       len) < 0) {
                perror("Send failed");
                continue;
            }

            int n = recvfrom(sockfd, response,
                             SIZE - 1, 0,
                             (struct sockaddr *)&server,
                             &len);

            if (n < 0) {
                printf("Timeout: no server response.\n");
                continue;
            }

            response[n] = '\0';

            if (strcmp(response, "NOT_FOUND") == 0) {
                printf("Domain not found by server.\n");
                continue;
            }

            strcpy(ip, response);
            found = 1;
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        /* Step 3: Display final result */
        if (found) {
            add_cache(domain, ip);
            printf("IP Address: %s\n", ip);
            printf("Saved in client cache.\n");
        } else {
            printf("Domain not found after %d attempts.\n",
                   MAX_RETRIES);
        }

        printf("Total lookup time: %.3f ms\n\n",
               elapsed_ms(start, end));
    }

    close(sockfd);
    return 0;
}
