#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: openport [port number]\n");
        return 1;
    }
    int port = atoi(argv[1]);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Failed to create socket");
        return 1;
    }

    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        perror("Failed to set socket options");
        close(socket_fd);
        return 1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Failed to bind socket");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, 1) == -1) {
        perror("Failed to listen on socket");
        close(socket_fd);
        return 1;
    }

    printf("Port %d opened successfully.\n", port);

    while (1) {
        sleep(1);
    }

    return 0;
}