#ifndef COMMUNICATION
#define COMMUNICATION

#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 17777

int sendAll(int fd, void* buffer, size_t size);
int recvAll(int fd, void* buffer, size_t size);

#endif
