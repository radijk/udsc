#include <stddef.h>
#include <sys/socket.h>
#include "udsc.h"

static const struct types types[] = {
	{"STREAM",	SOCK_STREAM,	1},
	{"DGRAM",	SOCK_DGRAM,	0},
	{"RAW",		SOCK_RAW,	0},
	{"RDM",		SOCK_RDM,	0},
	{"SEQPACKET",	SOCK_SEQPACKET,	1},
	{NULL,		-1,		0}
};

int strt(const char *type)
{
	int i = 0;

	while (types[i].str) {
		if (!strcmp(types[i].str, type)) {
			if (!types[i].supported) {
				return -1;
			}
			return types[i].val;
		}
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
	if (!strcmp("SERVER", mode))
		return SERVER_MODE;

	if (!strcmp("CLIENT", mode))
		return CLIENT_MODE;

	return -1;
}

const char *streol(const char *eol)
{
	if (!strcmp("DOS", eol))
		return "\r\n";

	if (!strcmp("UNIX", eol))
		return "\n";

	return NULL;
}
