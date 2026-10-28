
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

/* Session performance counters */
long total_lookups = 0;
long cache_hits = 0, cache_misses = 0;
long successful_lookups = 0, failed_lookups = 0;
long requests_sent = 0, responses_received = 0;
long timeout_count = 0, retry_count = 0;
long bytes_sent = 0, bytes_received = 0;

double total_network_delay_ms = 0;
double total_cache_delay_ms = 0;
double total_lookup_delay_ms = 0;

struct timespec session_start;

double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 +
           (b.tv_nsec - a.tv_nsec) / 1000000.0;
}

int find_cache(char *domain, char *ip) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].domain, domain) == 0) {
            strcpy(ip, cache[i].ip);
            return 1;
        }
    }
    return 0;
}

void add_cache(char *domain, char *ip) {
    if (cache_count < MAX_CACHE) {
        strcpy(cache[cache_count].domain, domain);
        strcpy(cache[cache_count].ip, ip);
        cache_count++;
    }
}

void show_stats(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double seconds = elapsed_ms(session_start, now) / 1000.0;

    printf("\n========== DNS PERFORMANCE REPORT ==========\n");
    printf("Total lookups             : %ld\n", total_lookups);
    printf("Cache hits                : %ld\n", cache_hits);
    printf("Cache misses              : %ld\n", cache_misses);
    printf("Successful resolutions    : %ld\n", successful_lookups);
    printf("Failed resolutions        : %ld\n", failed_lookups);
    printf("Requests sent             : %ld\n", requests_sent);
    printf("Responses received        : %ld\n", responses_received);
    printf("Timed-out requests        : %ld\n", timeout_count);
    printf("Retries after first try   : %ld\n", retry_count);

    printf("Cache hit ratio           : %.2f%%\n",
           total_lookups ?
           100.0 * cache_hits / total_lookups : 0.0);

    printf("Resolution success rate   : %.2f%%\n",
           total_lookups ?
           100.0 * successful_lookups / total_lookups : 0.0);

    printf("Request timeout rate      : %.2f%%\n",
           requests_sent ?
           100.0 * timeout_count / requests_sent : 0.0);

    printf("Average network delay     : %.3f ms\n",
           responses_received ?
           total_network_delay_ms / responses_received : 0.0);

    printf("Average cache lookup time : %.3f ms\n",
           cache_hits ?
           total_cache_delay_ms / cache_hits : 0.0);

    printf("Average lookup time       : %.3f ms\n",
           total_lookups ?
           total_lookup_delay_ms / total_lookups : 0.0);

    printf("Session duration          : %.3f seconds\n",
           seconds);

    printf("Resolution throughput    : %.3f lookups/sec\n",
           seconds > 0 ?
           successful_lookups / seconds : 0.0);

    printf("Response throughput      : %.3f responses/sec\n",
           seconds > 0 ?
           responses_received / seconds : 0.0);

    printf("Data throughput           : %.2f bytes/sec\n",
           seconds > 0 ?
           (bytes_sent + bytes_received) / seconds : 0.0);

    printf("=============================================\n\n");
}

int main(void) {
    int sockfd;
    char domain[SIZE], ip[SIZE], response[SIZE];

    struct sockaddr_in server;
    struct timeval timeout;
    socklen_t len = sizeof(server);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    clock_gettime(CLOCK_MONOTONIC, &session_start);

    while (1) {
        printf("Enter domain (stats/exit): ");

        if (scanf("%99s", domain) != 1)
            break;

        if (strcmp(domain, "exit") == 0)
            break;

        if (strcmp(domain, "stats") == 0) {
            show_stats();
            continue;
        }

        struct timespec lookup_start, lookup_end;
        clock_gettime(CLOCK_MONOTONIC, &lookup_start);

        total_lookups++;

        /* 1. Client-side cache lookup */
        struct timespec cache_start, cache_end;
        clock_gettime(CLOCK_MONOTONIC, &cache_start);

        int hit = find_cache(domain, ip);

        clock_gettime(CLOCK_MONOTONIC, &cache_end);

        double cache_time = elapsed_ms(cache_start, cache_end);

        if (hit) {
            cache_hits++;
            total_cache_delay_ms += cache_time;
            successful_lookups++;

            clock_gettime(CLOCK_MONOTONIC, &lookup_end);
            double lookup_time =
                elapsed_ms(lookup_start, lookup_end);
            total_lookup_delay_ms += lookup_time;

            printf("\nCache HIT!\n");
            printf("Domain: %s\nIP: %s\n", domain, ip);
            printf("Cache lookup time: %.3f ms\n", cache_time);
            printf("Total lookup time: %.3f ms\n\n", lookup_time);
            continue;
        }

        cache_misses++;
        printf("\nCache MISS!\n");

        int found = 0;

        /* 2. Query server: at most 3 attempts */
        for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
            memset(response, 0, sizeof(response));

            if (attempt > 1)
                retry_count++;

            printf("Attempt %d of %d...\n",
                   attempt, MAX_RETRIES);

            struct timespec request_start, request_end;
            clock_gettime(CLOCK_MONOTONIC, &request_start);

            ssize_t sent = sendto(
                sockfd, domain, strlen(domain), 0,
                (struct sockaddr *)&server, len
            );

            if (sent < 0) {
                perror("Send failed");
                continue;
            }

            requests_sent++;
            bytes_sent += sent;

            ssize_t n = recvfrom(
                sockfd, response, SIZE - 1, 0,
                NULL, NULL
            );

            clock_gettime(CLOCK_MONOTONIC, &request_end);

            double delay = elapsed_ms(request_start, request_end);

            if (n < 0) {
                timeout_count++;
                printf("No response (timeout/error), %.3f ms\n",
                       delay);
                continue;
            }

            responses_received++;
            bytes_received += n;
            total_network_delay_ms += delay;

            response[n] = '\0';

            printf("Server response time: %.3f ms\n", delay);

            if (strcmp(response, "NOT_FOUND") == 0) {
                printf("Server says domain not found.\n");
                continue;
            }

            strcpy(ip, response);
            found = 1;
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &lookup_end);

        double lookup_time = elapsed_ms(lookup_start, lookup_end);
        total_lookup_delay_ms += lookup_time;

        if (found) {
            add_cache(domain, ip);
            successful_lookups++;

            printf("Resolved IP: %s\n", ip);
            printf("Saved in client cache.\n");
        } else {
            failed_lookups++;
            printf("Domain not found after %d attempts.\n",
                   MAX_RETRIES);
        }

        printf("Total lookup time: %.3f ms\n\n", lookup_time);
    }

    show_stats();
    close(sockfd);
    return 0;
}
