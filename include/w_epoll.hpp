// 323CA Dumitrascu Filip-Teodor
#ifndef __W_EPOLL_H__
#define __W_EPOLL_H__

#include <sys/epoll.h>

#include "utils.hpp"

static inline int w_epoll_create()
{
    int rc = epoll_create1(0);
    DIE(rc < 0, "epoll_create1 failed\n");

	return rc;
}

static inline void w_epoll_add_fd_in(int epollfd, int fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;

    int rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    DIE(rc < 0, "epoll_ctl failed\n");
}

static inline void w_epoll_remove_fd(int epollfd, int fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;

	int rc = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    DIE(rc < 0, "epoll_ctl failed\n");
}

static inline void w_epoll_wait_infinite(int epollfd, struct epoll_event *rev)
{
	int rc = epoll_wait(epollfd, rev, 1, EPOLL_TIMEOUT_INFINITE);
    DIE(rc < 0, "epoll_wait failed\n");
}


#endif /* EPOLL_H_ */
