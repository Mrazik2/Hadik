#include <asm-generic/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <ncurses.h>

#include "communication.h"
#include "game.h"


#define FRAME_TIME_MS 200

typedef struct {
  int client_fd;
  struct sockaddr_in client_addr;
} client_data_t;

typedef struct {
  int server_fd;
  struct sockaddr_in server_addr;
} server_data_t;

typedef struct {
  client_data_t* client_data;
  pthread_mutex_t* mutex;
  int timeLeft;
  map_t* map;
  snake_t* snake;
  position_t* apple;
} data_t;

int isNotOpposite(int curX, int curY, int newX, int newY) {
  return (curX * newX + curY * newY) != -1;
}

void* readInput(void* arg) {
  data_t* data = (data_t*)arg;

  int buffer;

  while (data->snake->state != FROZEN) {
    if (recvAll(data->client_data->client_fd, &buffer, sizeof(buffer)) < 0) {
      pthread_mutex_lock(data->mutex);
      data->snake->state = FROZEN;
      pthread_mutex_unlock(data->mutex);
      break;
    }

    pthread_mutex_lock(data->mutex);
    int x = data->snake->direction[0];
    int y = data->snake->direction[1];
    pthread_mutex_unlock(data->mutex);
    
    if (buffer == 'x') {
      pthread_mutex_lock(data->mutex);
      data->snake->state = FROZEN;
      pthread_mutex_unlock(data->mutex);
      continue;
    } else if (buffer == 'w' && isNotOpposite(x, y, 0, -1)) {
      x = 0;
      y = -1;
    } else if (buffer == 'd' && isNotOpposite(x, y, 1, 0)) {
      x = 1;
      y = 0;
    } else if (buffer == 's' && isNotOpposite(x, y, 0, 1)) {
      x = 0;
      y = 1;
    } else if (buffer == 'a' && isNotOpposite(x, y, -1, 0)) {
      x = -1;
      y = 0;
    } else {
      continue;
    }

    pthread_mutex_lock(data->mutex);
    data->snake->nDirection[0] = x;
    data->snake->nDirection[1] = y;
    pthread_mutex_unlock(data->mutex);
  }

  return NULL;
}

void* gameLoop(void* arg) {
  data_t* data = (data_t*)arg;

  initGame(data->map, data->snake, data->apple);

  struct timeval tvalBefore, tvalAfter;
  gettimeofday(&tvalBefore, NULL);

  sendAll(data->client_data->client_fd, &data->map->actualWidth, sizeof(data->map->actualWidth));
  sendAll(data->client_data->client_fd, &data->map->actualHeight, sizeof(data->map->actualHeight));

  int state;
  while (data->snake->state != FROZEN) {
    pthread_mutex_lock(data->mutex);
    state = data->snake->state;
    pthread_mutex_unlock(data->mutex);
    sendAll(data->client_data->client_fd, &state, sizeof(state));

    for (int i = 0; i < data->map->actualHeight; i++) {
      sendAll(data->client_data->client_fd, data->map->map[i], data->map->actualWidth);
    }

    if (data->snake->state != DEAD) {
      pthread_mutex_lock(data->mutex);
      movement(data->snake);
      int coll = collisionCheck(data->snake, data->apple, data->map);
      redraw(data->snake, data->apple, data->map, coll);
      pthread_mutex_unlock(data->mutex);
    }

    gettimeofday(&tvalAfter, NULL);
    long difference = ((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec;
    if (difference < FRAME_TIME_MS * 1000) {
      usleep(FRAME_TIME_MS * 1000 - difference);
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
  if (setsockopt(serverData.server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
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
  int fromFile;
  if (strcmp(argv[2], "s") == 0) {
    fromFile = 1;
  } else {
    fromFile = 0;
  }
  if (initMap(&map, fromFile, argv[6], atoi(argv[4]), atoi(argv[5])) == -1) {
    close(clientData.client_fd);
    close(serverData.server_fd);
    perror("Chyba pri nacitani mapy");
    return -1;
  }

  data.map = &map;

  snake_t snake;
  
  if (strcmp(argv[1], "c") == 0) {
    data.timeLeft = atoi(argv[3]);
  } else {
    data.timeLeft = -5;
  }
  
  data.snake = &snake;

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
