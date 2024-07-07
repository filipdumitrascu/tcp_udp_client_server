// 323CA Dumitrascu Filip-Teodor
#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.hpp"
#include "w_epoll.hpp"

/**
 * @brief Sends all len bytes from buf to sock_fd
 * 
 * @param sock_fd the fd
 * @param buff the buffer
 * @param len the size 
 * 
 * @return the number of bytes sent
*/
size_t send_all(int sock_fd, void *buff, size_t len);

/**
 * @brief Receives all len bytes from sock_fd to buf
 * 
 * @param sock_fd the fd
 * @param buff the buffer
 * @param len the size 
 * 
 * @return the number of bytes received
*/
size_t recv_all(int sock_fd, void *buff, size_t len);

#endif
