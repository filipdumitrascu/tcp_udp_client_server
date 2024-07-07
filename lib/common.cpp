// 323CA Dumitrascu Filip-Teodor
#include "common.hpp"

size_t recv_all(int sock_fd, void *buffer, size_t len)
{
    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = static_cast<char *>(buffer);

    while (bytes_remaining) {
        size_t bytes = recv(sock_fd, buff + bytes_received, bytes_remaining, 0);
        DIE(bytes < 0, "recv failed\n");

        if (bytes == 0) {
            break;
        }

        bytes_remaining -= bytes;
        bytes_received += bytes;
    }

    return bytes_received;
}

size_t send_all(int sock_fd, void *buffer, size_t len)
{
    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = static_cast<char *>(buffer);

    while (bytes_remaining) {
        size_t bytes = send(sock_fd, buff + bytes_sent, bytes_remaining, 0);
        DIE(bytes < 0, "send failed\n");

        if (bytes == 0) {
            break;
        }

        bytes_remaining -= bytes;
        bytes_sent += bytes;
    }

    return bytes_sent;
}
