// 323CA Dumitrascu Filip-Teodor
#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vector>

#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

#define MAX_ID 11
#define MAX_TOPIC 51
#define START_PAYLOAD 51
#define SUBSCRB 15
#define MAX_BUF 1600
#define MAX_PAYLOAD 1500

#define EPOLL_TIMEOUT_INFINITE -1
#define SERVER_LOCALHOST_IP "127.0.0.1"

enum data_type {
    INT,
    SHORT_REAL,
    FLOAT,
    STRING
};

enum id_state {
    UNKNOWN,
    AVAILABLE,
    IN_USE
};

/**
 * @struct Tcp client type.
 * 
 * @param id his id
 * @param fd his fd with the server
 * @param connected if he's connected
 * @param subscriptions all his subcriptions 
*/
typedef struct tcp_client {
    int fd;
    char id[MAX_ID];
    bool connected;
    std::vector<char *> subscriptions;
} tcp_client;

/**
 * @struct Udp packet
 * 
 * @param topic the topic
 * @param data_type payload data type
 * @param payload the payload
*/
typedef struct udp_packet {
    char topic[MAX_TOPIC];
    unsigned int data_type;
    char payload[MAX_PAYLOAD];
} udp_packet;

#endif
