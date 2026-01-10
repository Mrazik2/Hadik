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
#include <fcntl.h>

#include "communication.h"
#include "menu.h"


// 0 - nie je v hre, 1 - je pozastaveny a v menu, 2 - nema hadika a v menu, 3 - je pozastaveny a v hre, 4 - nema hadika a v hre, 5 - ma hadika
typedef struct {
  int stavHry;
  int client_fd;
  struct sockaddr_in server_addr;
  pthread_mutex_t* mutex;
} data_t;



void* checkInput(void* arg) {
  data_t* data = (data_t*)arg;

  int temp = 'c';
  sendAll(data->client_fd, &temp, sizeof(temp));

  while (data->stavHry > 2) {
    int input = getch();
    if (input == KEY_UP || tolower(input) == 'w') {
      input = 'w';
    } else if (input == KEY_RIGHT || tolower(input) == 'd') {
      input = 'd';
    } else if (input == KEY_DOWN || tolower(input) == 's') {
      input = 's';
    } else if (input == KEY_LEFT || tolower(input) == 'a') {
      input = 'a';
    } else if (tolower(input) == 'x' && data->stavHry > 3) {
      input = 'x';
    } else if (tolower(input) == 'r' && data->stavHry == 4) {
      input = 'r';
    } else {
      continue;
    }
    sendAll(data->client_fd, &input, sizeof(input));
    
    if (input == 'x' && data->stavHry > 3) {
      pthread_mutex_lock(data->mutex);
      if (data->stavHry == 5) {
        data->stavHry = 1;
      } else if (data->stavHry == 4) {
        data->stavHry = 2;
      }
      pthread_mutex_unlock(data->mutex);
      perror("Odchadzam");
      refresh();
      sleep(2);
      break;
    }
  }

  return NULL;
}

void* draw(void* arg) {
  data_t* data = (data_t*)arg;

  int width = 0;
  int height = 0;
  recvAll(data->client_fd, &width, sizeof(width));
  recvAll(data->client_fd, &height, sizeof(height));
  int buffer;
  int buffer2;
  int buffer3;
  char* map = malloc(width * height + 1);


  while (data->stavHry > 2) {
    if (recvAll(data->client_fd, &buffer, sizeof(buffer)) < 0) {
      perror("Menej ako 0");
      break;
    }
    pthread_mutex_lock(data->mutex);
    data->stavHry = buffer;
    char buff[50];
    sprintf(buff, "%d", buffer);
    clear();
    mvaddstr(1, 10, buff);
    pthread_mutex_unlock(data->mutex);
    if (recvAll(data->client_fd, map, width * height) < 0) {
      perror("Nastala chyba");
      break;
    }

    for (int i = 0; i < height; i++) {
      mvaddnstr(3 + i, 30, map + i * width, width);
    }

    recvAll(data->client_fd, &buffer, sizeof(buffer));
    buffer = buffer / 1000;
    sprintf(buff, "Cas hry: %d sekund", buffer);
    mvaddstr(2, 10, buff);

    if (data->stavHry == 0) {
      recvAll(data->client_fd, &buffer, sizeof(buffer));
      for (int i = 0; i < buffer; i++) {
        recvAll(data->client_fd, &buffer2, sizeof(buffer2));
        recvAll(data->client_fd, &buffer3, sizeof(buffer3));
        sprintf(buff, "Hadik %d: %d bodov, zil %d sekund", i + 1, buffer2, buffer3 / 1000);
        mvaddstr(40 + i, 30, buff);
      }

      refresh();
      break;
    }

    recvAll(data->client_fd, &buffer, sizeof(buffer));
    recvAll(data->client_fd, &buffer2, sizeof(buffer2));
    sprintf(buff, "Hadik %d: %d bodov", buffer, buffer2);
    mvaddstr(3, 10, buff);
  
    mvaddstr(40, 30, "Stlac wasd alebo sipky pre pohyb");
    mvaddstr(41, 30, "Stlac x pre odchod do hlavneho menu");
    if (data->stavHry == 4) {
      mvaddstr(42, 30, "Stlac r pre noveho hadika");
    }

    refresh();
  }


  free(map);

  return NULL;
}


int connectToServer(data_t* data) {
  clear();
  
  if (data->client_fd != -1) {
    close(data->client_fd);
  }

  if (data->stavHry == 0) {
    data->client_fd = socket(AF_INET, SOCK_STREAM, 0);
  }
  if (data->client_fd < 0) {
    perror("-1");
    return 1;
  }

  memset(&data->server_addr, 0, sizeof(data->server_addr));
  data->server_addr.sin_family = AF_INET;
  data->server_addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, "127.0.0.1", (struct sockaddr*)&data->server_addr.sin_addr) < 0) {
    close(data->client_fd);
    perror("-2");
    return 2;
  }
  
  int flag = 0;
  for (int i = 0; i < 5; i++) {
    if (connect(data->client_fd, (struct sockaddr*)&data->server_addr, sizeof(data->server_addr)) < 0) {
      sleep(1);
      continue;
    } else {
      flag = 1;
      break;
    }
  }
  if (flag == 0) {
    perror("-3");
    return 3;
  }

  return 0;
}

void startGame(data_t* data, int continueGame) {
  pthread_t reader;
  pthread_t writer;

  if (data->stavHry < 2) {
    data->stavHry = 3;
  } else if (data->stavHry == 2) {
    data->stavHry = 4;
  }


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

  data->stavHry = 0;

}


int main(void) {
  data_t data;
  data.stavHry = 0;
  data.client_fd = -1;

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
        if (connectToServer(&data) == 0) {
          startGame(&data, 0);
        }
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
  close(data.client_fd);

  return 0;
}
