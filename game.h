#ifndef GAME
#define GAME

// aj s okrajom
#define MAX_WIDTH 20
#define MAX_HEIGHT 20
#define COOLDOWN_TIME_MS 3000
#define NO_SNAKE_SHUTDOWN_MS 10000

#define SNAKE_HEAD 'O'
#define SNAKE_BODY 'o'
#define APPLE_IMG '@'

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
  GAME_OVER = 0,
  FROZEN_IN_MENU = 1,
  DEAD_IN_MENU = 2,
  FROZEN_IN_GAME = 3,
  DEAD = 4,
  ALIVE = 5,
  LEFT = 6
} state_t;

typedef enum {
  EMPTY,
  FROM_FILE
} map_type_t;

typedef struct {
  int x;
  int y;
} position_t;

typedef struct {
  position_t position[(MAX_HEIGHT - 2) * (MAX_WIDTH - 2) + 1];
  int size;
  state_t state;
  int direction[2];
  int nDirection[2];
  int timeAlive[100];
  int snakeNum;
  int points[100];
} snake_t;

typedef struct {
  char map[MAX_HEIGHT][MAX_WIDTH];
  int actualWidth;
  int actualHeight;
  int spawnX;
  int spawnY;
  map_type_t type;
} map_t;

void movement(snake_t* snake, map_t* map);
int collisionCheck(snake_t* snake, position_t* apple, map_t* map);
void redraw(snake_t* snake, position_t* apple, map_t* map, int collision);
int initMap(map_t* map, int fromFile, char* fileName, int width, int height);
void initGame(map_t* map, snake_t* snake, position_t* apple);
void initSnake(map_t* map, snake_t* snake);
void setSpawn(map_t* map, int posX, int posY);
void placeApple(map_t* map, position_t* apple);

#endif
