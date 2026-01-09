#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <curses.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "communication.h"
#include "menu.h"


// 0 - nie je v hre, 1 - je pozastaveny, 2 - nema hadika, 3 - ma hadika
typedef struct {
  int stavHry;
  int client_fd;
  struct sockaddr_in server_addr;
  pthread_mutex_t* mutex;
} data_t;



void* checkInput(void* arg) {
  data_t* data = (data_t*)arg;
  /*
  char buffer[10];
  memset(buffer, 0, 10);

  while (data->stavHry > 1) {
    int input = getch();
    if (input == KEY_UP || tolower(input) == 'w') {
      strcpy(buffer, "w");
    } else if (input == KEY_RIGHT || tolower(input) == 'd') {
      strcpy(buffer, "d");
    } else if (input == KEY_DOWN || tolower(input) == 's') {
      strcpy(buffer, "s");
    } else if (input == KEY_LEFT || tolower(input) == 'a') {
      strcpy(buffer, "a");
    } else if (tolower(input) == 'x') {
      strcpy(buffer, "x");
    } else {
      continue;
    }
    send(data->client_fd, buffer, strlen(buffer), 0);
    if (tolower(input) == 'x') {
      break;
    }
  }
  */

  return NULL;
}

void* draw(void* arg) {
  data_t* data = (data_t*)arg;
  int bufferSize = 30;
  char buffer[bufferSize];
 
  while(data->stavHry > 1) {
    memset(buffer, 0, bufferSize);
    if (recv(data->client_fd, buffer, bufferSize, 0) <= 0) {
      break;
    }
    clear();
    mvaddstr(1, 10, buffer);
    data->stavHry = atoi(buffer);
    memset(buffer, 0, bufferSize);
    if (recv(data->client_fd, buffer, bufferSize, 0) <= 0) {
      break;
    }
    mvaddstr(2, 10, buffer);
    int lines = atoi(buffer);
    for (int i = 0; i < lines; i++) {
      memset(buffer, 0, bufferSize);
      if (recv(data->client_fd, buffer, bufferSize, 0) <= 0) {
        break;
      }
      mvaddstr(3 + i, 30, buffer);
    }
    refresh();
  }



  return NULL;
}


int connectToServer(data_t* data) {
  clear();
  if (data->stavHry == 0) {
    data->client_fd = socket(AF_INET, SOCK_STREAM, 0);
  }
  if (data->client_fd < 0) {
    return -1;
  }

  memset(&data->server_addr, 0, sizeof(data->server_addr));
  data->server_addr.sin_family = AF_INET;
  data->server_addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, "127.0.0.1", (struct sockaddr*)&data->server_addr.sin_addr) < 0) {
    close(data->client_fd);
    return -2;
  }

  if (connect(data->client_fd, (struct sockaddr*)&data->server_addr, sizeof(data->server_addr)) < 0) {
    close(data->client_fd);
    return -3;
  }

  return 0;
}

void startGame(data_t* data, int continueGame) {
  pthread_t reader;
  pthread_t writer;

  data->stavHry = 2;


  if (pthread_create(&reader, NULL, checkInput, data) < 0) {
    perror("Chyba pri vytvarani threadu");
    exit(-1);
  }
  if (pthread_create(&writer, NULL, draw, data) < 0) {
    perror("Chyba pri vytvarani threadu");
    exit(-1);
  }

  if (pthread_join(reader, NULL) < 0) {
    perror("Chyba pri joine threadu");
    exit(-1);
  }

  if (pthread_join(writer, NULL) < 0) {
    perror("Chyba pri joine threadu");
    exit(-1);
  }  

}


int main(void) {
  data_t data;
  data.stavHry = 0;

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  data.mutex = &mutex;


  initscr();
  keypad(stdscr, true);
  cbreak();
  noecho();
  curs_set(0);

  int selected = 0;

  while (1) {
    selected = mainMenu(selected);

    if (selected == 0) {
      if (newGameMenu()) {
        connectToServer(&data);
        startGame(&data, 0);
      }
    } else if (selected == 1) {
      continue; 
      //startGame(&data, 1);
    } else {
      break;
    }
  }

  pthread_mutex_destroy(&mutex);
  
  endwin();
  struct stat buf;
  if (fstat(data.client_fd, &buf) != -1) {
    close(data.client_fd);
  }
  return 0;
}
