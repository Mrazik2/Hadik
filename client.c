#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <curses.h>

#define PORT 16767

typedef struct {
  int stavHry;
} data_t;



void* checkInput(void* arg) {

  while (1) {
    int input = getch();


  }
  return NULL;
}

int newGame(data_t* data) {  
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


int main(void) {
  data_t data;
  data.stavHry = 0;
  
  /*
  const pid_t id = fork();
  if (id < 0) {
    perror("Fork failed");
    return -1;
  } else if (id == 0) {
    execlp("./server", "./server", NULL);
    perror("execlp failed");
    return -2;
  }

  wait(NULL);
  printf("Hello from id: %d\n", id);

  */

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

  getch();
  exit(-1);


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
        data.stavHry = newGame(&data);
      } else if (selected == 1) {
        continue;
      } else {
        break;
      }
    }

  }


  
  endwin();

  return 0;
}
