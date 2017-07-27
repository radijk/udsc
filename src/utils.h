#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>

#define print(fmt, ...)	fprintf(stdout, fmt, ##__VA_ARGS__)

/* Makes a file descriptor non blocking */
int fdnb(int fd);

/* Get user input from stdin */
ssize_t getinput(char *buf, size_t len, const char *eol);

/* Try to close a fd and log a warning message on failure */
void closew(int fd);

#endif /* utils.h */
