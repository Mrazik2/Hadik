#include <asm-generic/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>



#define PORT 16769

typedef struct {
  int client_fd;
  struct sockaddr_in client_addr;
} client_data_t;

typedef struct {
  int server_fd;
  struct sockaddr_in server_addr;
} server_data_t;

int main(int argc, char **argv) {
  server_data_t serverData;
  client_data_t clientData;

  serverData.server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverData.server_fd < 0) {
    perror("Chyba pri otvarani socketu");
    return -1;
  }

  int opt = 1;
  if (setsockopt(serverData.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Chyba pri volani setsockopt");
    return -2;
  }

  memset(&serverData.server_addr, 0, sizeof(serverData.server_addr));
  serverData.server_addr.sin_family = AF_INET;
  serverData.server_addr.sin_addr.s_addr = INADDR_ANY;
  serverData.server_addr.sin_port = htons(PORT);

  if (bind(serverData.server_fd, (struct sockaddr*)&serverData.server_addr, sizeof(serverData.server_addr)) < 0) {
    close(serverData.server_fd);
    perror("Zlyhal bind");
    return -3;
  }
  
  if (listen(serverData.server_fd, 1) < 0) {
    close(serverData.server_fd);
    perror("Zlyhal listen");
    return -4;
  }
  
  socklen_t client_len = sizeof(clientData.client_addr);
  clientData.client_fd = accept(serverData.server_fd, (struct sockaddr*)&clientData.client_addr, &client_len);
  if (clientData.client_fd < 0) {
    close(serverData.server_fd);
    perror("Zlyhal accept");
    return -5;
  }

  char* sprava = "Sprava zo servera";
  send(clientData.client_fd, sprava, strlen(sprava), 0);
  

  close(clientData.client_fd);
  close(serverData.server_fd);

  return 0;
}
