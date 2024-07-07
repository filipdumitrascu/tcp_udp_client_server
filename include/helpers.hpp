// 323CA Dumitrascu Filip-Teodor
#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <netinet/tcp.h>
#include <cmath>

#include "common.hpp"

/**
 * @brief Splits a string with "/" as a delimitator
 * and stores the substrings in an array.
 * 
 * @param str the string
 * @param substrs the substrings
*/
void split_string(char *str, std::vector<char *> &substrs);

/**
 * @brief Parses a buffer into a udp packet.
 * 
 * @param buf the buffer
 * 
 * @return the packet
*/
udp_packet parse_packet(char *buf);

/**
 * @brief Checks if a client's subscription matches the topic
 * 
 * @param subs the subscription
 * @param topic the topic
 * 
 * @return boolean
*/
bool is_subscribed(char *subs, char *topic);

/**
 * @brief Connects a tcp client to the server.
 * 
 * @param clients an array to store the clients
 * @param epoll_fd the epoll file descriptor
 * @param tcp_fd the listening fd for tcp clients
*/
void handle_tcp_conn(std::vector<tcp_client> &clients, int epoll_fd,
                     int tcp_fd);

/**
 * @brief Disconnects a tcp client from the server.
 * 
 * @param clients an array where the clients are stored 
 * @param epoll_fd the epoll file descriptor
 * @param client_fd the client fd
*/
void handle_tcp_disconn(std::vector<tcp_client> &clients,  int epoll_fd,
                        int client_fd);

/**
 * @brief (Un)subscribes a client to a topic.
 * 
 * @param clients an array where the clients are stored 
 * @param client_fd the client fd
 * @param buf the buffer with the request
*/
void handle_tcp_request(std::vector<tcp_client> &clients, int client_fd,
                        char *buf);

/**
 * @brief Receives an udp packet and sends
 * the content to interested tcp clients.
 * 
 * @param clients an array where the clients are stored
 * @param udp_fd the listening fd for udp clients
*/
void handle_udp_packet(std::vector<tcp_client> &clients, int udp_fd);

/**
 * @brief Closes the sockets and frees the clients memory
 * 
 * @param clients an array where the clients are stored
 * @param epoll_fd the epoll file descriptor
 * @param tcp_fd the listening fd for tcp clients
 * @param udp_fd the listening fd for udp clients
*/
void cleanup_server(std::vector<tcp_client> &clients, int epoll_fd,
                    int tcp_fd, int udp_fd);
                

#endif