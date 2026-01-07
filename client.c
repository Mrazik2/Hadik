#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <curses.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 16769

typedef struct {
  int stavHry;
  int client_fd;
  struct sockaddr_in server_addr;
} data_t;



void* checkInput(void* arg) {

  while (1) {
    int input = getch();


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
  char fileName[30];

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
      mvaddstr(1, 10, "Zadaj sirku sveta");


      getnstr(width, 3);

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

      if (input == 'y' && atoi(width) > 0) {
        break;
      }
    }

    while (1) {
      clear();
      int input = 0;
      mvaddstr(1, 10, "Zadaj vysku sveta");


      getnstr(height, 3);

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

      if (input == 'y' && atoi(height) > 0) {
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
        FILE *fptr = fopen(fileName, "r");
        if (fptr != NULL) {
          fclose(fptr);
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
  data->client_fd = socket(AF_INET, SOCK_STREAM, 0);
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


int main(void) {
  data_t data;
  data.stavHry = 0;


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

    if (input == KEY_DOWN && selected < count - 1) {
      selected++;
    }
    if (input == KEY_UP && selected > 0) {
      selected--;
    }

    if (input == 10) {
      if (selected == 0) {
        if (newGame()) {
          sleep(1);
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
        }
      } else if (selected == 1) {
        continue;
      } else {
        break;
      }
    }
  }


  
  endwin();
  close(data.client_fd);

  return 0;
}
