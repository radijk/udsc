#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <sys/types.h>

/* Send data to socket */
ssize_t senddata(int fd, void *buf, size_t len);

/* Receive data from socket */
ssize_t recvdata(int fd);

#endif /* connection.h */
