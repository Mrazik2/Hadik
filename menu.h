#ifndef MENU
#define MENU

#define NEW "Nova hra"
#define CONTINUE "Pokracuj v hre"
#define JOIN "Pripoj sa k hre"
#define END "Koniec"

#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int mainMenu(int oldSelected);
int newGameMenu(int port);
int joinGameMenu();

#endif
