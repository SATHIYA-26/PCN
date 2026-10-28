
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 100

int main() {
    int sockfd;
    char domain[SIZE], response[SIZE];

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    char *domains[] = {
        "google.com",
        "youtube.com",
        "ssn.edu.in",
        "example.com"
    };

    char *ips[] = {
        "142.250.183.14",
        "142.250.72.206",
        "203.110.243.10",
        "93.184.216.34"
    };

    int count = 4;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("DNS Server running on port %d...\n", PORT);

    while (1) {
        memset(domain, 0, sizeof(domain));

        int n = recvfrom(sockfd, domain, SIZE - 1, 0,
                         (struct sockaddr *)&client, &len);

        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        domain[n] = '\0';
        domain[strcspn(domain, "\r\n")] = '\0';

        printf("Received query: %s\n", domain);

        strcpy(response, "NOT_FOUND");

        for (int i = 0; i < count; i++) {
            if (strcmp(domain, domains[i]) == 0) {
                strcpy(response, ips[i]);
                break;
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client, len);

        printf("Response sent: %s\n\n", response);
    }

    close(sockfd);
    return 0;
}
