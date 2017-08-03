#ifndef _UDSC_H
#define _UDSC_H

#define SERVER_MODE	0
#define CLIENT_MODE	1

struct types {
	const char *str;
	int val;
	int supported;
};

struct sockinfo {
	const char *path;
	int type;
	const char *eol;
};

/* Convert string to socket type */
int strt(const char *type);

/* Convert socket type string */
const char *tstr(int type);

/* Convert string to mode */
int strm(const char *mode);

/* Return end of line string */
const char *streol(const char *eol);

#endif /* udsc.h */
