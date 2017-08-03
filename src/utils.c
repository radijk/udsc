#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "utils.h"

int fdnb(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		warn("fcntl getfl failed");
		return -1;
	}

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) == -1) {
		warn("fcntl setfl failed");
		return -1;
	}

	return 0;
}

ssize_t getinput(char *buf, size_t len, const char *eol)
{
	ssize_t count;

	memset(buf, 0, len);

	count = read(STDIN_FILENO, buf, len);
	if (count == -1) {
		warn("read failed");
		return -1;
	}

	memcpy(buf+count-1, eol, strlen(eol));

	return (count - 1 + strlen(eol));
}

void closew(int fd)
{
	if (close(fd) == -1)
		warn("close fd %d failed", fd);
}
