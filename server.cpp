// 323CA Dumitrascu Filip-Teodor
#include "helpers.hpp"

void run_server(int tcp_fd, int udp_fd)
{
    char buf[MAX_BUF];
    std::vector<tcp_client> tcp_clients;

    int rc = listen(tcp_fd, SOMAXCONN);
    DIE(rc < 0, "listen failed\n");

    /* Create the file descriptors:
    - epoll_fd for epoll management
    - stdin
    - listen_tcp_fd
    - listen_udp_fd and
    - upcoming fds for the tcp_clients
    Event type expected to appear is EPOLLIN
    to have readable data available.*/
    int epoll_fd = w_epoll_create();

    w_epoll_add_fd_in(epoll_fd, STDIN_FILENO);
    w_epoll_add_fd_in(epoll_fd, tcp_fd);
    w_epoll_add_fd_in(epoll_fd, udp_fd);

    while (true) {
        /* Memory area marked with events occurred.
        (so as not to iterate through all the fds) */
        struct epoll_event rev;

        /* Thread waits for an I/O event to
        occur on a set of descriptors */
        w_epoll_wait_infinite(epoll_fd, &rev);

        /* Skip no EPOLLIN event */
        if (!(rev.events & EPOLLIN)) {
            continue;
        }

        /* Receive a connection from a TCP client */
        if (rev.data.fd == tcp_fd) {
            handle_tcp_conn(tcp_clients, epoll_fd, tcp_fd);
            continue;
        }

        /* Receive a packet from a UDP client */
        if (rev.data.fd == udp_fd) {
            handle_udp_packet(tcp_clients, udp_fd);
            continue;
        }

        /* Receive a command from stdin */
        if (rev.data.fd == STDIN_FILENO) {
            memset(buf, 0, MAX_BUF);
            fgets(buf, sizeof(buf), stdin);

            if (strncmp(buf, "exit", 4)) {
                fprintf(stderr, "Unrecognized stdin command\n");
                continue;
            }

            cleanup_server(tcp_clients, epoll_fd, tcp_fd, udp_fd);
            return;
        }

        /* Data is received from the socket of
        one of the connected TCP clients */
        memset(buf, 0, MAX_BUF);
        int rc = recv_all(rev.data.fd, buf, MAX_BUF);

        /* If there was no data received, the client closed the connection. */
        if (rc == 0) {
            handle_tcp_disconn(tcp_clients, epoll_fd, rev.data.fd);
            continue;
        } 

        handle_tcp_request(tcp_clients, rev.data.fd, buf);
    }
}

int main(int argc, char *argv[])
{
    /* Disable buffering on display. */
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* Check the arguments received. */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    /* Parse the port as a number. */
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Given port is invalid\n");

    /* Get a TCP socket and a UDP socket for receiving connections. */
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_fd < 0, "tcp socket failed\n");

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_fd < 0, "udp socket failed\n");

    /* Make the socket addresses reusable and sets
    TCP_NODELAY for TCP socket to disable Nagle. */
    int enable = 1;
    rc = setsockopt(tcp_fd, IPPROTO_TCP, SO_REUSEADDR |
                    TCP_NODELAY, &enable, sizeof(int));
    DIE(rc < 0, "tcp setsockopt failed\n");

    rc = setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &enable,
                    sizeof(int));
    DIE(rc < 0, "udp setsockopt failed\n");

    /* Fill in serv_addr with the server address,
    address family and port for connection. */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    rc = inet_pton(AF_INET, SERVER_LOCALHOST_IP, &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed\n");

    /* Associate the server addresses with the sockets created using bind. */
    rc = bind(tcp_fd, (const struct sockaddr *)&serv_addr,
              sizeof(serv_addr));
    DIE(rc < 0, "tcp bind failed\n");

    rc = bind(udp_fd, (const struct sockaddr *)&serv_addr,
              sizeof(serv_addr));
    DIE(rc < 0, "udp bind failed\n");

    /* Run the program. */
    run_server(tcp_fd, udp_fd);

    return 0;
}
