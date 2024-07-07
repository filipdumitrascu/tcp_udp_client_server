// 323CA Dumitrascu Filip-Teodor
#include "common.hpp"

void cleanup_client(int epoll_fd, int sock_fd)
{
    /* Close the connection and the created socket. */
    w_epoll_remove_fd(epoll_fd, STDIN_FILENO);
    w_epoll_remove_fd(epoll_fd, sock_fd);

    int rc = close(epoll_fd);
    DIE(rc < 0, "close failed\n");

    rc = close(sock_fd);
    DIE(rc < 0, "close failed\n");
}

void run_tcp_client(int sock_fd)
{
    char buf[MAX_BUF];

    /* Create the file descriptors:
    - epoll_fd for epoll management
    - stdin and 
    - socketfd (the connection to the server)
    Event type expected to appear is EPOLLIN
    to have readable data available.*/
    int epoll_fd = w_epoll_create();

    w_epoll_add_fd_in(epoll_fd, STDIN_FILENO);
    w_epoll_add_fd_in(epoll_fd, sock_fd);

    while (true) {
        /* Memory area marked with events occurred.
        (so as not to iterate through all the fds) */
        struct epoll_event rev;

        /* Thread waits for an I/O event to
        occur on a set of descriptors */
        w_epoll_wait_infinite(epoll_fd, &rev);

        if (!(rev.events & EPOLLIN)) {
            continue;
        }

        if (rev.data.fd == STDIN_FILENO) {
            memset(buf, 0, MAX_BUF);
            fgets(buf, sizeof(buf), stdin);

            /* If "exit" was received from stdin, close
            the connection and the socket. */
            if (!strncmp(buf, "exit", 4)) {
                cleanup_client(epoll_fd, sock_fd);
                return;
            }

            /* If "(un)subscribe" has been received from stdin, send
            it to the server, which takes care of the request */
            send_all(sock_fd, buf, MAX_BUF);

            continue;
        }

        memset(buf, 0, MAX_BUF);
        int rc = recv_all(sock_fd, buf, MAX_BUF);

        /* If the server is down, close the
        connection and the socket. */
        if (rc == 0) {
            cleanup_client(epoll_fd, sock_fd);
            return;
        }

        /* Print the message sent by the server to stdout */
        printf("%s", buf);
    }
}

int main(int argc, char *argv[])
{
    /* Disable buffering on display. */
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* Check the arguments received. */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <id> <ip> <port>\n", argv[0]);
        return 1;
    }

    /* Parse the port as a number. */
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    DIE(rc != 1, "Given port is invalid\n");

    /* Get a TCP socket to connect to the server */
    const int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sock_fd < 0, "tcp socket failed\n");

    /* Fill in serv_addr with the server address,
    address family and port for connection. */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed\n");

    /* Connecting to the server */
    rc = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "tcp client connect failed\n");

    /* Send TCP client ID to server
    immediately after connection */
    send_all(sock_fd, argv[1], sizeof(argv[1]));

    /* Run the program. */
    run_tcp_client(sock_fd);

    return 0;
}
