#ifndef GAME
#define GAME

// aj s okrajom
#define MAX_WIDTH 20
#define MAX_HEIGHT 20

#define SNAKE_HEAD 'O'
#define SNAKE_BODY 'o'
#define APPLE_IMG '@'

#include <stdio.h>
#include <string.h>

typedef enum {
  FROZEN = 1,
  DEAD = 2,
  ALIVE = 3
} state_t;

typedef struct {
  int x;
  int y;
} position_t;

typedef struct {
  position_t position[(MAX_HEIGHT - 2) * (MAX_WIDTH - 2) + 1];
  int size;
  state_t state;
  int direction[2];
} snake_t;

typedef struct {
  char map[MAX_HEIGHT + 1][MAX_WIDTH + 1];
  int actualWidth;
  int actualHeight;
} map_t;

void movement(snake_t* snake);
int collisionCheck(snake_t* snake, position_t* apple, map_t* map);
void redraw(snake_t* snake, position_t* apple, map_t* map, int collision);
int initMap(map_t* map, int fromFile, char* fileName, int width, int height);
int initGame(map_t* map, snake_t* snake, position_t* apple);

#endif
