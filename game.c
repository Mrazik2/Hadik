#include "game.h"



void movement(snake_t* snake) {
  int xTo = snake->position[0].x;
  int yTo = snake->position[0].y;

  snake->position[0].x += snake->nDirection[0];
  snake->position[0].y += snake->nDirection[1];
  snake->direction[0] = snake->nDirection[0];
  snake->direction[1] = snake->nDirection[1];
  for (int i = 1; i < snake->size; i++) {
    int xFrom = snake->position[i].x;
    int yFrom = snake->position[i].y;
    
    snake->position[i].x = xTo;
    snake->position[i].y = yTo;

    xTo = xFrom;
    yTo = yFrom;
  }
  snake->position[snake->size].x = xTo;
  snake->position[snake->size].y = yTo;
}

int collisionCheck(snake_t* snake, position_t* apple, map_t* map) {
  int headX = snake->position[0].x;
  int headY = snake->position[0].y;
  if (map->map[headY][headX] == ' ') {
    return 0;
  } else if (map->map[headY][headX] == APPLE_IMG) {
    return 1;
  } else {
    return 2;
  }
}

void redraw(snake_t* snake, position_t* apple, map_t* map, int collision) {
  if (collision == 0) {
    map->map[snake->position[snake->size].y][snake->position[snake->size].x] = ' ';
    map->map[snake->position[0].y][snake->position[0].x] = SNAKE_HEAD;
    if (snake->size > 1) {
      map->map[snake->position[1].y][snake->position[1].x] = SNAKE_BODY;
    }
  } else if (collision == 1) {
    map->map[snake->position[0].y][snake->position[0].x] = SNAKE_HEAD;
    map->map[snake->position[1].y][snake->position[1].x] = SNAKE_BODY;
    snake->size++;
    placeApple(map, apple);
  } else {
    for (int i = 1; i < snake->size + 1; i++) {
      map->map[snake->position[i].y][snake->position[i].x] = ' ';
      snake->state = DEAD;
    }
  }
}

int initMap(map_t* map, int fromFile, char* fileName, int width, int height) {
  if (fromFile) {
    FILE* fptr = fopen(fileName, "r");
    char buffer[MAX_WIDTH + 10];
    int fWidth, fHeight, snakeX, snakeY;
    if (fscanf(fptr, "%d %d %d %d", &fWidth, &fHeight, &snakeX, &snakeY) < 4) {
      fclose(fptr);
      perror("Chyba vo formate file");
      return -1;
    }
    setSpawn(map, snakeX, snakeY);
    map->actualWidth = fWidth;
    map->actualHeight = fHeight;
    fgets(buffer, MAX_WIDTH + 10, fptr);
    for (int i = 0; i < fHeight; i++) {
      fgets(buffer, MAX_WIDTH + 10, fptr);
      strncpy(map->map[i], buffer, fWidth);
    }

    fclose(fptr);
  } else {
    setSpawn(map, 0, 0);
    map->actualWidth = width + 2;
    map->actualHeight = height + 2;
    memset(map->map[0], '#', map->actualWidth);
    for (int i = 1; i < map->actualHeight - 1; i++) {
      memset(map->map[i], ' ', map->actualWidth);
      map->map[i][0] = '#';
      map->map[i][map->actualWidth - 1] = '#';
    }
    memset(map->map[map->actualHeight - 1], '#', map->actualWidth);
  }

  return 0;
}

void initGame(map_t* map, snake_t* snake, position_t* apple) {
  initSnake(map, snake);

  placeApple(map, apple);
}

void initSnake(map_t* map, snake_t* snake) {
  snake->size = 1;
  snake->direction[0] = 1;
  snake->direction[1] = 0;
  snake->nDirection[0] = 1;
  snake->nDirection[1] = 0;
  snake->state = ALIVE;
  snake->position[0].x = map->spawnX;
  snake->position[0].y = map->spawnY;
  snake->position[1].x = map->spawnX;
  snake->position[1].y = map->spawnY;
  map->map[snake->position[0].y][snake->position[0].x] = SNAKE_HEAD;
}

void setSpawn(map_t* map, int posX, int posY) {
  if (posX == 0 && posY == 0) {
    map->spawnX = 2;
    map->spawnY = 2;
  } else {
    map->spawnX = posX;
    map->spawnY = posY;
  }
}

void placeApple(map_t* map, position_t* apple) {
  int x = 0;
  int y = 0;
  while (map->map[y][x] != ' ' || (x == map->spawnX && y == map->spawnY)) {
    x = 1 + rand() % (map->actualWidth - 2);
    y = 1 + rand() % (map->actualHeight - 2);
  }
  apple->x = x;
  apple->y = y;
  map->map[apple->y][apple->x] = APPLE_IMG;
}
