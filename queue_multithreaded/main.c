#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

typedef struct Node {
  int num;
  struct Node *next;
} Node;

typedef struct Queue {
  Node *head;
  Node *tail;
} Queue;

typedef struct PopArg {
  Queue *Q;
  const char *file_name;
} PopArg;

Queue *Q;
pthread_mutex_t mut;
sem_t semaphore;

void reverse(char s[]) {
  int i, j;
  char c;

  for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

void my_itoa(int n, char s[]) {
  int i, sign;

  if ((sign = n) < 0) {
    n = -n;
  }
  i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

void push(Queue *Q, int n) {
  Node *tmp = malloc(sizeof(Node));
  if (tmp == NULL) {
    perror("Malloc error in push");
    return;
  }

  tmp->num = n;
  tmp->next = NULL;
  if (Q->head == NULL) {
    Q->head = tmp;
    Q->tail = tmp;
    return;
  }
  Q->tail->next = tmp;
  Q->tail = tmp;
}

void push_random(Queue *Q, int amount) {
  int k;
  pthread_mutex_lock(&mut);
  for (int i = 0; i < amount; ++i) {
    k = rand() % 10;
    push(Q, k);
    sem_post(&semaphore);
  }
  pthread_mutex_unlock(&mut);
}

void printQueue(Queue *Q) {
  Node *tmp = Q->head;
  while (tmp != NULL) {
    tmp = tmp->next;
  }
}

void *pop(void *PopArgs) {
  Queue *Q = ((PopArg *)(PopArgs))->Q;
  const char *txt_name = ((PopArg *)(PopArgs))->file_name;
  int fd = open(txt_name, O_WRONLY);

  if (fd == 0) {
    perror("Error while openning file in pop");
    return NULL;
  }
  while (1) {
    sem_wait(&semaphore);
    pthread_mutex_lock(&mut);
    char buff[10];
    if (Q->head == NULL) {
      pthread_mutex_unlock(&mut);
      break;
    }
    my_itoa(Q->head->num, buff);
    write(fd, buff, strlen(buff));
    if (Q->head->next == NULL) {
      free(Q->head);
      Q->tail = NULL;
      Q->head = NULL;
      pthread_mutex_unlock(&mut);
      break;
    }
    Node *tmp;
    tmp = Q->head->next;
    free(Q->head);
    Q->head = tmp;
    pthread_mutex_unlock(&mut);
  }
  close(fd);
  return NULL;
}

int main() {
  srand(time(0));
  pthread_mutex_init(&mut, NULL);
  sem_init(&semaphore, 0, 0);
  Q = malloc(sizeof(Queue));
  Q->head = Q->tail = NULL;
  push_random(Q, 10000);
  pthread_t thr1, thr2, thr3;
  PopArg arg1, arg2, arg3;
  arg1.Q = arg2.Q = arg3.Q = Q;
  arg1.file_name = "file1.txt";
  arg2.file_name = "file2.txt";
  arg3.file_name = "file3.txt";
  pthread_create(&thr1, NULL, &pop, (void *)&arg1);
  pthread_create(&thr2, NULL, &pop, (void *)&arg2);
  pthread_create(&thr3, NULL, &pop, (void *)&arg3);
  pthread_join(thr1, NULL);
  pthread_join(thr2, NULL);
  pthread_join(thr3, NULL);
  return 1;
}
