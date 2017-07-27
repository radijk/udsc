#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <errno.h>
#include <err.h>
#include "udsc.h"
#include "utils.h"
#include "server.h"

#define MAX_EVENTS	2

int server(struct sockinfo *sockinfo)
{
	int sockfd, clifd;
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

	sockfd = socket(AF_UNIX, sockinfo->type, 0);
	if (sockfd == -1) {
		warn("socket failed");
		goto err_epoll;
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sockinfo->path, sizeof(addr.sun_path)-1);
	unlink(sockinfo->path);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		warn("bind failed");
		goto err_sock;
	}

	if (listen(sockfd, 1) == -1) {
		warn("listen failed");
		goto err_sock;
	}

	clifd = accept(sockfd, NULL, NULL);
	if (clifd == -1) {
		warn("accept failed");
		goto err_sock;
	}

	if (fdnb(clifd) == -1)
		warn("fdnb failed");

	event.data.fd = clifd;
	event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, clifd, &event) == -1) {
		warn("epoll_ctl failed");
		goto err_client;
	}

	if (fdnb(STDIN_FILENO) == -1)
		warn("fdnb failed");

	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	event.data.fd = STDIN_FILENO;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event)) {
		warn("epoll_ctl failed");
		goto err_client;
	}

	while (1) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			warn("epoll_wait failed");
			goto err_client;
		}
		for (n = 0; n < nfds; n++) {
			if (events[n].data.fd == clifd) {
				if (events[n].events &
				   (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
					print("client disconnected\n");
					goto err_client;
				}
				if (events[n].events & EPOLLIN) {
					memset(buf, 0, BUFSIZ);
					count = read(clifd, buf, BUFSIZ);
					if (count == -1) {
						warn("read failed");
						goto err_client;
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
						goto err_client;
					}
					count = write(clifd, buf, count);
					if (count == -1) {
						warn("write failed");
						goto err_client;
					}
				}
			}
		}
	}

	return 0;

err_client:
	closew(clifd);
err_sock:
	closew(sockfd);
err_epoll:
	closew(epfd);
	return -1;
}
