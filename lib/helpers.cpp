// 323CA Dumitrascu Filip-Teodor
#include "helpers.hpp"

void split_string(char *str, std::vector<char *> &substrs)
{
    char *token = strtok(str, "/");

    while (token) {
        substrs.push_back(token);
        token = strtok(NULL, "/");
    }
}

udp_packet parse_packet(char *buf)
{
    udp_packet packet;

    /* Parse the packet. */
    strncpy(packet.topic, buf, MAX_TOPIC - 1);
    packet.topic[MAX_TOPIC - 1] = '\0';

    /* the data type. */
    packet.data_type = buf[MAX_TOPIC - 1];

    double p;
    int sign = 1;

    /* and the payload. */
    switch (packet.data_type) {
        case INT:
            if (buf[START_PAYLOAD] == 1) {
                sign = -1;
            }

            sprintf(packet.payload, "%d", sign *
                    ntohl(*(uint32_t *)(buf + START_PAYLOAD + 1)));
            break;

        case SHORT_REAL:
            sprintf(packet.payload, "%f",
                    (double)ntohs(*(uint16_t *)(buf + START_PAYLOAD)) / 100.0);
            break;

        case FLOAT:
            if (buf[START_PAYLOAD] == 1) {
                sign = -1;
            }

            p = pow(10, (uint8_t)buf[START_PAYLOAD + 5]);

            sprintf(packet.payload, "%f", sign * 
                    (double)ntohl(*(uint32_t *)(buf + START_PAYLOAD + 1)) / p);
            break;

        case STRING:
            strcpy(packet.payload, buf + START_PAYLOAD);
            break;

        default:
            fprintf(stderr, "Unrecognized data type\n");
    }

    return packet;
}

bool is_subscribed(char *subs, char *topic)
{
    std::vector<char *> subs_layers;
    std::vector<char *> topic_layers;

    /* Separate memory area is needed. */
    char *copy_subs = strdup(subs);
    DIE(!copy_subs, "strdup failed\n");

    char *copy_topic = strdup(topic);
    DIE(!copy_topic, "strdup failed\n");

    /* Split the layyers and put them into arrays. */
    split_string(copy_subs, subs_layers);
    split_string(copy_topic, topic_layers);

    size_t i = 0;
    size_t j = 0;

    while (i < subs_layers.size() && j < topic_layers.size()) {
        if (!strcmp(subs_layers[i], "*")) {
            i++;

            /* If the '*' is the last layer, any topic matches. */
            if (i == subs_layers.size()) {
                free(subs_layers[0]);
                free(topic_layers[0]);
                return true;
            }

            /* Search through the topic layers and try to
            match with the next subscription layer. */
            while (strcmp(topic_layers[j], subs_layers[i])) {
                j++;

                /* If there is no match the topic does't match. */
                if (j == topic_layers.size()) {
                    free(subs_layers[0]);
                    free(topic_layers[0]);
                    return false;
                }
            }

            i++;
            j++;

            continue;
        }

        if (!strcmp(subs_layers[i], "+")) {
            /* This topic layer can be anything */
            i++;
            j++;

            continue;
        }

        /* If the current layers don't match, the topic doesn't match.  */
        if (strcmp(subs_layers[i], topic_layers[j])) {
            free(subs_layers[0]);
            free(topic_layers[0]);
            return false;
        }

        i++;
        j++;
    }

    free(subs_layers[0]);
    free(topic_layers[0]);

    /* If there are still layers unmatched, the topic doesn't match. */
    return (i == subs_layers.size()) && (j == topic_layers.size());
}

void handle_tcp_conn(std::vector<tcp_client> &clients, int epoll_fd,
                     int tcp_fd)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    /* Accept the connection between the tcp client and the server. */
    int data_tcp_fd = accept(tcp_fd, (struct sockaddr *)&cli_addr,
                             &cli_len);
    DIE(data_tcp_fd < 0, "accept failed\n");

    /* Sizeof(sent id) [<= MAX_ID] bytes are received from the client. */
    char id[MAX_ID];
    int rc = recv(data_tcp_fd, id, sizeof(id), 0);
    DIE(rc < 0, "recv failed\n");

    int pos = -1;
    id_state state = UNKNOWN;

    /* Checks if the id is unkwon, available or already in use. */
    for (size_t i = 0; i < clients.size(); i++) {
        if (!strcmp(clients[i].id, id)) {
            state = AVAILABLE;

            if (clients[i].connected) {
                state = IN_USE;
            }
            pos = i;

            break;
        }
    }

    /* If isn't already in use, adds the fd to the epoll. */
    if (state != IN_USE) {
        w_epoll_add_fd_in(epoll_fd, data_tcp_fd);
        printf("New client %s connected from %s:%d.\n",
               id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    }

    tcp_client client;

    switch (state) {
        /* If the id is uknwon, stores it to the array of clients. */
        case UNKNOWN:
            client.fd = data_tcp_fd;
            client.connected = true;

            strcpy(client.id, id);
            clients.push_back(client);

            break;

        /* Else, updates the fd and connected status for the client. */
        case AVAILABLE:
            clients[pos].connected = true;
            clients[pos].fd = data_tcp_fd; 

            break;

        /* If the id is in use, close the accepted socket. */
        case IN_USE:
            printf("Client %s already connected.\n", id);
            
            rc = close(data_tcp_fd);
            DIE(rc < 0, "close failed\n");

            break;

        default:
            fprintf(stderr, "Unrecognized id state\n");
    }
}

void handle_tcp_disconn(std::vector<tcp_client> &clients,  int epoll_fd,
                        int client_fd)
{
    char *id;
    int pos = -1;

    /* Find the id for the disconnected client. */
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == client_fd) {
            id = clients[i].id;
            pos = i;
            break;
        }
    }

    printf("Client %s disconnected.\n", id);

    /* Remove the client fd fromt he epoll and close it. */
    w_epoll_remove_fd(epoll_fd, client_fd);
    
    int rc = close(client_fd);
    DIE(rc < 0, "close failed\n");

    /* Update the client fd and connection status. */
    clients[pos].fd = -1;
    clients[pos].connected = false;
}

void handle_tcp_request(std::vector<tcp_client> &clients, int client_fd,
                        char *buf)
{
    char subs[SUBSCRB];
    char topic[MAX_TOPIC];

    memset(subs, 0, SUBSCRB);
    memset(topic, 0, MAX_TOPIC);

    sscanf(buf, "%s %s", subs, topic);

    int pos = -1;

    /* Find the client with the client_fd */
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == client_fd) {
            pos = i;
            break;
        }
    }
    
    if (!strcmp(subs, "subscribe")) {
        /* Search through his subscriptions to see
        if he is already subscribed to topic. */
        for (size_t i = 0; i < clients[pos].subscriptions.size(); i++) {
            if (!strcmp(clients[pos].subscriptions[i], topic)) {
                fprintf(stderr, "Already subscribed to topic %s\n", topic);
                return;
            }
        }

        /* Store the topic into clinet's subscriptions. */
        char *copy_topic = strdup(topic);
        DIE(!copy_topic, "strdup failed\n");

        clients[pos].subscriptions.push_back(copy_topic);

        memset(buf, 0, MAX_BUF);
        sprintf(buf, "Subscribed to topic %s\n", topic);

        send_all(client_fd, buf, MAX_BUF);

        return;
    }

    if (!strcmp(subs, "unsubscribe")) {
        /* Search through his subscriptions to see
        if he is subscribed to the topic. */
        bool unsubscribed = false;

        for (size_t i = 0; i < clients[pos].subscriptions.size(); i++) {
            if (!strcmp(clients[pos].subscriptions[i], topic)) {
                clients[pos].subscriptions.erase(clients[pos].subscriptions.begin() + i);
                unsubscribed = true;
                break;
            }
        }

        if (!unsubscribed) {
            fprintf(stderr, "Client is not subscribed to topic %s\n", topic);
            return;
        }

        memset(buf, 0, MAX_BUF);
        sprintf(buf, "Unsubscribed from topic %s\n", topic);

        send_all(client_fd, buf, MAX_BUF);

        return;
    }

    fprintf(stderr, "Unrecognized client command\n");
}

void handle_udp_packet(std::vector<tcp_client> &clients, int udp_fd)
{
    char buf[MAX_BUF];

    const char *data_type_string[4] = {"INT", "SHORT_REAL", "FLOAT", "STRING"};
    struct sockaddr_in client_addr;

    socklen_t clen = sizeof(client_addr);
    memset(buf, 0, MAX_BUF);

    /* Receives the data from the udp clients and parse it. */
    int rc = recvfrom(udp_fd, buf, MAX_BUF, 0,
                      (struct sockaddr *)&client_addr, &clen);
    DIE(rc < 0, "recvfrom failed\n");

    udp_packet packet = parse_packet(buf);

    /* If the client is subscribed to the packet
    topic and he is connected, send the packet to him. */
    for (auto client : clients) {
        for (auto subscription : client.subscriptions) {
            if (!is_subscribed(subscription, packet.topic)) {
                continue;
            }

            memset(buf, 0, MAX_BUF);
            sprintf(buf, "%s - %s - %s\n", // "%s:d" - 
                    // inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
                    packet.topic, data_type_string[packet.data_type],
                    packet.payload);

            if (client.connected) {
                send_all(client.fd, buf, MAX_BUF);
            }

            break;
        }
    }
}

void cleanup_server(std::vector<tcp_client> &clients, int epoll_fd,
                    int tcp_fd, int udp_fd)
{
    int rc;

    /* Frees the memory allocated with strdup
    removes every client fd and close them. */
    for (auto client : clients) {
        for (auto subscription : client.subscriptions) {
            free(subscription);
        }

        if (client.connected) {
            w_epoll_remove_fd(epoll_fd, client.fd);

            rc = close(client.fd);
            DIE(rc < 0, "close failed\n");
        }
    }

    /* Removes the sockets form the epoll. */
    w_epoll_remove_fd(epoll_fd, STDIN_FILENO);
    w_epoll_remove_fd(epoll_fd, tcp_fd);
    w_epoll_remove_fd(epoll_fd, udp_fd);

    /* Close the sockets. */
    rc = close(tcp_fd);
    DIE(rc < 0, "close failed\n");

    rc = close(udp_fd);
    DIE(rc < 0, "close failed\n");

    rc = close(epoll_fd);
    DIE(rc < 0, "close failed\n");
}
