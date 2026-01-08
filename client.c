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

#define PORT 17777


// 0 - nie je v hre, 1 - je pozastaveny, 2 - nema hadika, 3 - ma hadika
typedef struct {
  int stavHry;
  int client_fd;
  struct sockaddr_in server_addr;
  pthread_mutex_t* mutex;
} data_t;



void* checkInput(void* arg) {
  data_t* data = (data_t*)arg;
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

int newGame() {  
  char gameMode[2];
  gameMode[1] = 0;
  char worldType[2];
  worldType[1] = 0;

  char width[4];
  width[3] = 0;
  char height[4];
  height[3] = 0;

  char timeIn[4];
  char fileName[50];

  clear();
  mvaddstr(1, 10, "Zvol herny rezim - standardny alebo casovy (s/c)");
  while (1) {
    int input = getch();
    if (input == 's' || input == 'c') {
      gameMode[0] = input;
      break;
    }
  }
  
  clear();
  mvaddstr(1, 10, "Zvol typ herneho sveta - bez prekazok alebo s prekazkami (b/s)");
  while (1) {
    int input = getch();
    if (input == 'b' || input == 's') {
      worldType[0] = input;
      break;
    }
  }
  
  if (gameMode[0] == 'c') {
    while (1) {
      clear();
      int input = 0;
      mvaddstr(1, 10, "Zadaj cas hry v sekundach (max 999)");


      getnstr(timeIn, 3);

      while (1) {
        clear();
        mvaddstr(1, 10, "Napisal si:");
        mvaddstr(2, 10, timeIn);
        mvaddstr(4, 10, "Potvrd vyber (y/n)");

        input = getch();
      
        if (input == 'y' || input == 'n') {
          break;
        }
      }

      if (input == 'y' && atoi(timeIn) > 0) {
        break;
      }
    }
  }

  if (worldType[0] == 'b') {
    while (1) {
      clear();
      int input = 0;
      mvaddstr(1, 10, "Zadaj sirku sveta <9, 18>");


      getnstr(width, 2);

      while (1) {
        clear();
        mvaddstr(1, 10, "Napisal si:");
        mvaddstr(2, 10, width);
        mvaddstr(4, 10, "Potvrd vyber (y/n)");

        input = getch();
        
        if (input == 'y' || input == 'n') {
          break;
        }
      }

      if (input == 'y' && atoi(width) > 8 && atoi(width) <= 20) {
        break;
      }
    }

    while (1) {
      clear();
      int input = 0;
      mvaddstr(1, 10, "Zadaj vysku sveta <9, 18>");


      getnstr(height, 2);

      while (1) {
        clear();
        mvaddstr(1, 10, "Napisal si:");
        mvaddstr(2, 10, height);
        mvaddstr(4, 10, "Potvrd vyber (y/n)");

        input = getch();
        
        if (input == 'y' || input == 'n') {
          break;
        }
      }

      if (input == 'y' && atoi(height) > 8 && atoi(height) <= 20) {
        break;
      }
    }

  }

  if (worldType[0] == 's') {
    while (1) {
      clear();
      int input = 0;
      mvaddstr(1, 10, "Zadaj nazov suboru so svetom");


      getnstr(fileName, 29);

      while (1) {
        clear();
        mvaddstr(1, 10, "Napisal si:");
        mvaddstr(2, 10, fileName);
        mvaddstr(4, 10, "Potvrd vyber (y/n) alebo stlac q pre ukoncenie");

        input = getch();
        
        if (input == 'q') {
          return 0;
        }
        if (input == 'y' || input == 'n') {
          break;
        }
      }

      if (input == 'y') {
        char fileBuffer[50];
        strcpy(fileBuffer, "../maps/");
        strcat(fileBuffer, fileName);
        FILE* fptr = fopen(fileBuffer, "r");
        if (fptr != NULL) {
          fclose(fptr);
          strcpy(fileName, fileBuffer);
          break;
        }
      }
    }

  }

  clear();
  mvaddstr(1, 10, "Stlac hociake tlacidlo pre start");
  getch();

  
  const pid_t id = fork();
  if (id < 0) {
    perror("Fork zlyhal");
    exit(-1);
  } else if (id == 0) {
    execlp("./server", "./server", gameMode, worldType, timeIn, width, height, fileName, NULL);
    fprintf(stderr, "execlp failed\n");
    exit(-1);
  }
  
  
  return 1;
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

  const char *menuItems[3] = {
  "Nova hra",
  "Pokracuj v hre",
  "Koniec"
  };

  int selected = 0;
  int count = 3;

  while(1) {
    clear();

    mvaddstr(1, 10, menuItems[0]);
    mvaddstr(2, 10, menuItems[1]);
    mvaddstr(3, 10, menuItems[2]);

    if (selected >= 0 && selected < count) {
      mvchgat(selected + 1, 10, 14, A_STANDOUT, 1, NULL);
    }

    refresh();

    int input = getch();

    if ((input == KEY_DOWN || tolower(input) == 's') && selected < count - 1) {
      selected++;
    }
    if ((input == KEY_UP || tolower(input) == 'w') && selected > 0) {
      selected--;
    }

    if (input == 10) {
      if (selected == 0) {
        if (newGame()) {
          sleep(1);
          connectToServer(&data);
          /*
          int vysl = connectToServer(&data);
          if (vysl == 0) {
            mvaddstr(6, 10, "Ok");
          } else if (vysl == -1) {
            mvaddstr(6, 10, "socket");
          } else if (vysl == -2) {
            mvaddstr(6, 10, "inet_pton");
          } else {
            mvaddstr(6, 10, "connect");
          }

          refresh();

          char buffer[100];
          recv(data.client_fd, buffer, 99, 0);
          mvaddstr(10, 10, buffer);
          getch();
          */
          startGame(&data, 0);
        }
      } else if (selected == 1) {
        startGame(&data, 1);
      } else {
        break;
      }
    }
  }

  pthread_mutex_destroy(&mutex);
  
  endwin();
  close(data.client_fd);

  return 0;
}
