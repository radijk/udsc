#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include "udsc.h"
#include "utils.h"
#include "client.h"

#define MAX_EVENTS	3

int client(struct sockinfo *sockinfo)
{
	int sockfd, sigfd;
	int epfd, nfds, n;
	ssize_t count;
	char buf[BUFSIZ];
	struct sockaddr_un addr = {0};
	struct epoll_event event = {0}, events[MAX_EVENTS] = {0};
	sigset_t sigmask;
	struct signalfd_siginfo fdsi;
	size_t size = BUFSIZ;

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

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
		warn("sigprocmask failed");
		goto err_sock;
	}

	sigfd = signalfd(-1, &sigmask, SFD_NONBLOCK);
	if (sigfd == -1) {
		warn("signalfd failed");
		goto err_sock;
	}

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = sigfd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sigfd, &event)) {
		warn("epoll_ctl failed");
		goto err_sig;
	}

	while (1) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			warn("epoll_wait failed");
			goto err_sig;
		}
		for (n = 0; n < nfds; n++) {
			if (events[n].data.fd == sockfd) {
				if (events[n].events & EPOLLIN) {
					memset(buf, 0, size);
					count = read(sockfd, buf, size);
					if (count == -1) {
						warn("read failed: %m");
						goto err_sig;
					}
					if (!count) {
						print("connection lost\n");
						goto err_sig;
					}
					print("%s", buf);
				}
			}
			if (events[n].data.fd == STDIN_FILENO) {
				if (events[n].events & EPOLLIN) {
					count = getinput(buf, size,
							sockinfo->eol);
					if (count == -1) {
						warn("getinput failed");
						goto err_sig;
					}
					count = write(sockfd, buf, count);
					if (count == -1) {
						warn("write failed");
						goto err_sig;
					}
				}
			}
			if (events[n].data.fd == sigfd) {
				if (events[n].events & EPOLLIN) {
					size = sizeof(struct signalfd_siginfo);
					count = read(sigfd, &fdsi, size);
					if (count == -1) {
						warn("read failed");
						goto err_sig;
					}
					if ((fdsi.ssi_signo == SIGINT) ||
					(fdsi.ssi_signo == SIGTERM)) {
						closew(sigfd);
						closew(sockfd);
						closew(epfd);
						return 0;
					}
				}
			}
		}
	}

err_sig:
	closew(sigfd);
err_sock:
	closew(sockfd);
err_epoll:
	closew(epfd);
	return -1;
}
