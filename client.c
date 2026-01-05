#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <curses.h>

#define PORT 16767

int main(void) {
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

  initscr();
  keypad(stdscr, true);
  cbreak();
  noecho();
  curs_set(0);

  const char *menuItems[3] = {
  "Nova hra",
  "Pokracuj v hre",
  "koniec"
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
      if (selected == 2) {
        break;
      }
    }

  }


  
  endwin();

  return 0;
}
