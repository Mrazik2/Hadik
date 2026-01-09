#include "game.h"



void movement(snake_t* snake) {
  int xTo = snake->position[0].x;
  int yTo = snake->position[0].y;

  snake->position[0].x += snake->direction[0];
  snake->position[0].y += snake->direction[1];
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
    //map->map[apple->y][apple->x] = APPLE_IMG;
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
    int fWidth;
    int fHeight;
    char buffer[MAX_WIDTH + 1];
    if (fscanf(fptr, "%d %d", &fWidth, &fHeight) < 2) {
      fclose(fptr);
      perror("Chyba vo formate file");
      return -1;
    }
    map->actualWidth = fWidth;
    map->actualHeight = fHeight;
    for (int i = 0; i < fHeight; i++) {
      memset(buffer, 0, MAX_WIDTH + 1);
      fgets(buffer, MAX_WIDTH, fptr);
      strcpy(map->map[i], buffer);
    }

    fclose(fptr);
  } else {
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

int initGame(map_t* map, snake_t* snake, position_t* apple) {
  snake->size = 1;
  snake->direction[0] = 1;
  snake->direction[1] = 0;
  snake->state = ALIVE;

  snake->position[0].x = 4;
  snake->position[0].y = 8;
  apple->x = 8;
  apple->y = 8;
  map->map[snake->position[0].y][snake->position[0].x] = SNAKE_HEAD;
  map->map[apple->y][apple->x] = APPLE_IMG;
  return 0;
}
