#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <errno.h>
#include <err.h>
#include "udsc.h"
#include "utils.h"
#include "client.h"

#define MAX_EVENTS	2

int client(struct sockinfo *sockinfo)
{
	int sockfd;
	int epfd, nfds, n;
	ssize_t count;
	char buf[BUFSIZ];
	struct sockaddr_un addr = {0};
	struct epoll_event event, events[MAX_EVENTS] = {0};

	epfd = epoll_create1(0);
	if (epfd == -1) {
		warn("epoll_create1 failed");
		return -1;
	}

	sockfd = socket(AF_UNIX, sockinfo->type | SOCK_NONBLOCK, 0);
	if (sockfd == -1) {
		warn("socket failed");
		goto err_epoll;
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sockinfo->path, sizeof(addr.sun_path)-1);

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		warn("connect failed");
		goto err_sock;
	}

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = sockfd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event)) {
		warn("epoll_ctl failed");
		goto err_sock;
	}

	if (fdnb(STDIN_FILENO) == -1)
		warn("fdnb failed");

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = STDIN_FILENO;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event)) {
		warn("epoll_ctl failed");
		goto err_sock;
	}

	while (1) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			warn("epoll_wait failed");
			goto err_sock;
		}
		for (n = 0; n < nfds; n++) {
			if (events[n].data.fd == sockfd) {
				if (events[n].events & EPOLLIN) {
					memset(buf, 0, BUFSIZ);
					count = read(sockfd, buf, BUFSIZ);
					if (count == -1) {
						warn("read failed: %m");
						goto err_sock;
					}
					if (!count) {
						print("connection lost\n");
						goto err_sock;
					}
					print("%s", buf);
				}
			}
			if (events[n].data.fd == STDIN_FILENO) {
				if (events[n].events & EPOLLIN) {
					count = getinput(buf, BUFSIZ,
							sockinfo->eol);
					if (count == -1) {
						warn("getinput failed");
						goto err_sock;
					}
					count = write(sockfd, buf, count);
					if (count == -1) {
						warn("write failed");
						goto err_sock;
					}
				}
			}
		}
	}

	return 0;

err_sock:
	closew(sockfd);
err_epoll:
	closew(epfd);
	return -1;
}
