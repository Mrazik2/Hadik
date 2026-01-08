#include <asm-generic/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define PORT 17777

#define MAX_WIDTH 20
#define MAX_HEIGHT 20
// aj s okrajom
#define FRAME_TIME_MS 125
#define SNAKE_HEAD '@'
#define SNAKE_BODY 'o'
#define APPLE_IMG 'A'

typedef enum {
  FROZEN = 1,
  DEAD = 2,
  ALIVE = 3
} state_t;

typedef struct {
  int client_fd;
  struct sockaddr_in client_addr;
} client_data_t;

typedef struct {
  int server_fd;
  struct sockaddr_in server_addr;
} server_data_t;

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

typedef struct {
  client_data_t* client_data;
  pthread_mutex_t* mutex;
  int timeLeft;
  map_t* map;
  snake_t* snake;
  position_t* apple;

  char skuska[10];
} data_t;

int isNotOpposite(int curX, int curY, int newX, int newY) {
  return (curX * newX + curY * newY) != -1;
}

void* readInput(void* arg) {
  data_t* data = (data_t*)arg;
  int bufferSize = 10;
  char buffer[bufferSize];
  int x = 1;
  int y = 0;

  while (data->snake->state != FROZEN) {
    if (recv(data->client_data->client_fd, buffer, bufferSize, 0) <= 0) {
      data->snake->state = FROZEN;
      break;
    }
    if (strcmp(buffer, "x") == 0) {
      pthread_mutex_lock(data->mutex);
      data->snake->state = FROZEN;
      pthread_mutex_unlock(data->mutex);
      continue;
    } else if (strcmp(buffer, "w") == 0 && isNotOpposite(x, y, 0, -1)) {
      x = 0;
      y = -1;
    } else if (strcmp(buffer, "d") == 0 && isNotOpposite(x, y, 1, 0)) {
      x = 1;
      y = 0;
    } else if (strcmp(buffer, "s") == 0 && isNotOpposite(x, y, 0, 1)) {
      x = 0;
      y = 1;
    } else if (strcmp(buffer, "a") == 0 && isNotOpposite(x, y, -1, 0)) {
      x = -1;
      y = 0;
    } else {
      continue;
    }
    pthread_mutex_lock(data->mutex);
    data->snake->direction[0] = x;
    data->snake->direction[1] = y;
    pthread_mutex_unlock(data->mutex);
  }

  return NULL;
}

void movement(data_t* data) {
  pthread_mutex_lock(data->mutex);
  int xTo = data->snake->position[0].x;
  int yTo = data->snake->position[0].y;

  data->snake->position[0].x += data->snake->direction[0];
  data->snake->position[0].y += data->snake->direction[1];
  for (int i = 1; i < data->snake->size; i++) {
    int xFrom = data->snake->position[i].x;
    int yFrom = data->snake->position[i].y;
    
    data->snake->position[i].x = xTo;
    data->snake->position[i].y = yTo;

    xTo = xFrom;
    yTo = yFrom;
  }
  data->snake->position[data->snake->size].x = xTo;
  data->snake->position[data->snake->size].y = yTo;
  pthread_mutex_unlock(data->mutex);
}

int collisionCheck(data_t* data) {
  return 0;
}

void redraw(data_t* data, int collision) {
  if (collision == 0) {
    data->map->map[data->snake->position[data->snake->size].y][data->snake->position[data->snake->size].x] = ' ';
    data->map->map[data->snake->position[0].y][data->snake->position[0].x] = SNAKE_HEAD;
    if (data->snake->size > 1) {
      data->map->map[data->snake->position[1].y][data->snake->position[1].x] = SNAKE_BODY;
    }
  } else if (collision == 1) {
    data->map->map[data->snake->position[0].y][data->snake->position[0].x] = SNAKE_HEAD;
    data->map->map[data->snake->position[1].y][data->snake->position[1].x] = SNAKE_BODY;
    data->map->map[data->apple->y][data->apple->x] = APPLE_IMG;
  } else {
    for (int i = 1; i < data->snake->size + 1; i++) {
      data->map->map[data->snake->position[i].y][data->snake->position[i].x] = ' ';
    }
  }
}

void* gameLoop(void* arg) {
  data_t* data = (data_t*)arg;

  struct timeval tvalBefore, tvalAfter;
  gettimeofday(&tvalBefore, NULL);

  /*
  data->snake->position[0].x = 4;
  data->snake->position[0].y = 8;
  data->apple->x = 8;
  data->apple->y = 8;
  data->map->map[data->snake->position[0].y][data->snake->position[0].x] = SNAKE_HEAD;
  data->map->map[data->apple->y][data->apple->x] = APPLE_IMG;
  */

  char buffer[30];

  while (data->snake->state != FROZEN) {
    sprintf(buffer, "%d", data->snake->state);
    int sent = 0;

    send(data->client_data->client_fd, buffer, strlen(buffer), 0);




    sprintf(buffer, "%d", data->map->actualHeight);
    if (send(data->client_data->client_fd, buffer, strlen(buffer), 0) <= 0) {
      break;
    }

    for (int i = 0; i < data->map->actualHeight; i++) {
      if (send(data->client_data->client_fd, data->map->map[i], data->map->actualWidth, 0) <= 0) {
        break;
      }
    }


    gettimeofday(&tvalAfter, NULL);
    long difference = ((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec;
    if (difference < 125000) {
      usleep(125000 - difference);
    }
    gettimeofday(&tvalBefore, NULL);
  }

  return NULL;
}


int main(int argc, char **argv) {
  server_data_t serverData;
  client_data_t clientData;
  data_t data;
  data.client_data = &clientData;

  if (argc != 7) {
    perror("Malo argumentov");
    exit(-1);
  }
  
  
  serverData.server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverData.server_fd < 0) {
    perror("Chyba pri otvarani socketu");
    return -1;
  }

  int opt = 1;
  if (setsockopt(serverData.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Chyba pri volani setsockopt");
    return -2;
  }

  memset(&serverData.server_addr, 0, sizeof(serverData.server_addr));
  serverData.server_addr.sin_family = AF_INET;
  serverData.server_addr.sin_addr.s_addr = INADDR_ANY;
  serverData.server_addr.sin_port = htons(PORT);

  if (bind(serverData.server_fd, (struct sockaddr*)&serverData.server_addr, sizeof(serverData.server_addr)) < 0) {
    close(serverData.server_fd);
    perror("Zlyhal bind");
    return -3;
  }
  
  if (listen(serverData.server_fd, 1) < 0) {
    close(serverData.server_fd);
    perror("Zlyhal listen");
    return -4;
  }
  
  socklen_t client_len = sizeof(clientData.client_addr);
  clientData.client_fd = accept(serverData.server_fd, (struct sockaddr*)&clientData.client_addr, &client_len);
  if (clientData.client_fd < 0) {
    close(serverData.server_fd);
    perror("Zlyhal accept");
    return -5;
  }
  
 

  map_t map;

  
  if (strcmp(argv[2], "s") == 0) {
    FILE* fptr = fopen(argv[6], "r");
    int width;
    int height;
    char buffer[21];
    if (fscanf(fptr, "%d %d", &width, &height) < 2) {
      close(clientData.client_fd);
      close(serverData.server_fd);
      fclose(fptr);
      perror("Chyba vo formate file");
      exit(-1);
    }
    map.actualWidth = width;
    map.actualHeight = height;
    for (int i = 0; i < height; i++) {
      fgets(buffer, sizeof(buffer), fptr);
      strcpy(map.map[i], buffer);
    }

    fclose(fptr);
  } else {
    map.actualWidth = atoi(argv[4]) + 2;
    map.actualHeight = atoi(argv[5]) + 2;
    for (int i = 0; i < map.actualHeight; i++) {
      map.map[i][map.actualWidth] = 0;
    }
    memset(map.map[0], '#', map.actualWidth);
    for (int i = 1; i < map.actualHeight - 1; i++) {
      memset(map.map[i], ' ', map.actualWidth);
      map.map[i][0] = '#';
      map.map[i][map.actualWidth - 1] = '#';
    }
    memset(map.map[map.actualHeight - 1], '#', map.actualWidth);
  }

  data.map = &map;

  snake_t snake;
  snake.size = 1;
  snake.direction[0] = 1;
  snake.direction[1] = 0;
  snake.state = ALIVE;
  data.snake = &snake;
  if (strcmp(argv[1], "c") == 0) {
    data.timeLeft = atoi(argv[3]);
  } else {
    data.timeLeft = -5;
  }

  position_t apple;

  data.apple = &apple;

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  data.mutex = &mutex;



  pthread_t reader;
  pthread_t writer;

  if (pthread_create(&reader, NULL, readInput, &data) < 0) {
    perror("Chyba pri vytvarani threadu");
    pthread_mutex_destroy(&mutex);
    close(clientData.client_fd);
    close(serverData.server_fd);
    exit(-1);
  }
  if (pthread_create(&writer, NULL, gameLoop, &data) < 0) {
    perror("Chyba pri vytvarani threadu");
    pthread_mutex_destroy(&mutex);
    close(clientData.client_fd);
    close(serverData.server_fd);
    exit(-1);
  }

  if (pthread_join(reader, NULL) < 0) {
    perror("Chyba pri joine threadu");
    pthread_mutex_destroy(&mutex);
    close(clientData.client_fd);
    close(serverData.server_fd);
    exit(-1);
  }
  if (pthread_join(writer, NULL) < 0) {
    perror("Chyba pri joine threadu");
    pthread_mutex_destroy(&mutex);
    close(clientData.client_fd);
    close(serverData.server_fd);
    exit(-1);
  }


  pthread_mutex_destroy(&mutex);
  close(clientData.client_fd);
  close(serverData.server_fd);

  return 0;
}
