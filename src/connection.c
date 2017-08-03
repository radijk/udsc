#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "utils.h"
#include "connection.h"

ssize_t senddata(int fd, void *buf, size_t len)
{
	ssize_t count;
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = BUFSIZ,
	};
	struct msghdr msg = {
		.msg_name = NULL,
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	count = sendmsg(fd, &msg, 0);
	if ((count == -1) && (errno != EINTR)) {
		warn("sendmsg failed");
		return -1;
	}

	return count;
}

ssize_t recvdata(int fd)
{
	ssize_t count;
	char buf[BUFSIZ];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = BUFSIZ,
	};
	struct msghdr msg = {
		.msg_name = NULL,
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	count = recvmsg(fd, &msg, 0);
	if ((count == -1) && (errno != EINTR)) {
		warn("recvmsg failed");
		return -1;
	}

	print("%s", buf);

	return count;
}
