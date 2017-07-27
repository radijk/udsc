#include <stddef.h>
#include <sys/socket.h>
#include "udsc.h"

static const struct types types[] = {
	{"stream",	SOCK_STREAM},
	{"dgram",	SOCK_DGRAM},
	{"raw",		SOCK_RAW},
	{"rdm",		SOCK_RDM},
	{"seqpacket",	SOCK_SEQPACKET},
	{NULL,		-1}
};

int strt(const char *type)
{
	int i = 0;

	while (types[i].str) {
		if (!strcmp(types[i].str, type))
			return types[i].val;
		i++;
	}

	return -1;
}

const char *tstr(int type)
{
	int i = 0;

	while (types[i].val != -1) {
		if (types[i].val == type)
			return (char *)types[i].str;
		i++;
	}

	return NULL;
}

int strm(const char *mode)
{
	if (!strcmp("server", mode))
		return SERVER_MODE;

	if (!strcmp("client", mode))
		return CLIENT_MODE;

	return -1;
}

const char *streol(const char *eol)
{
	if (!strcmp("win", eol))
		return "\r\n";

	if (!strcmp("unix", eol))
		return "\n";

	return NULL;
}
