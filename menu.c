#include "menu.h"

int mainMenu(int oldSelected) {
  int itemCount = 3;
  int selected = oldSelected;

  while(1) {
    clear();

    mvaddstr(1, 10, NEW);
    mvaddstr(2, 10, CONTINUE);
    mvaddstr(3, 10, END);

    if (selected >= 0 && selected < itemCount) {
      mvchgat(selected + 1, 10, 14, A_STANDOUT, 1, NULL);
    }

    refresh();

    int input = getch();

    if ((input == KEY_DOWN || tolower(input) == 's') && selected < itemCount - 1) {
      selected++;
    } else if ((input == KEY_UP || tolower(input) == 'w') && selected > 0) {
      selected--;
    } else if (input == 10) {
      return selected;
    }
  }
}

int newGameMenu() {
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
