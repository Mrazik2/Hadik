#include "communication.h"

int sendAll(int fd, void* buffer, size_t size) {
  size_t sent = 0;
  while (sent < size) {
    int now = send(fd, (char*)buffer + sent, size - sent, 0);
    if (now <= 0) {
      return -1;
    } else {
      sent += now;
    }
  }
  return 0;
}


int recvAll(int fd, void* buffer, size_t size) {
  size_t received = 0;
  while (received < size) {
    int now = recv(fd, (char*)buffer + received, size - received, 0);
    if (now <= 0) {
      return -1;
    } else {
      received += now;
    }
  }
  return 0;
}
