#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 10

struct aiocb readF;
struct aiocb writeF;

void close_file(int input_fd) {
  int err = close(input_fd);
  if (err != 0) {
    perror("In close file\n");
  }
}

void clean_up() {
  close_file(readF.aio_fildes);
  close_file(writeF.aio_fildes);
  free((void *)readF.aio_buf);
  free((void *)writeF.aio_buf);
}

void init() {
  memset(&readF, 0, sizeof(struct aiocb));
  memset(&writeF, 0, sizeof(struct aiocb));
  int readDesc = open("read.txt", O_RDONLY);
  if (readDesc < 0) {
    perror("In read file\n");
    return;
  }
  int writeDesc = open("write.txt", O_WRONLY);
  if (writeDesc < 0) {
    perror("In read file\n");
    close_file(readDesc);
    return;
  }

  readF.aio_fildes = readDesc;
  writeF.aio_fildes = writeDesc;
  readF.aio_offset = 0;
  writeF.aio_offset = 0;
  readF.aio_buf = malloc(BUFF_SIZE * (sizeof(char)));
  if (readF.aio_buf == NULL) {
    perror("In malloc\n");
    close_file(readF.aio_fildes);
    close_file(writeF.aio_fildes);
    return;
  }
  writeF.aio_buf = malloc(BUFF_SIZE * (sizeof(char)));
  if (writeF.aio_buf == NULL) {
    perror("In malloc\n");
    close_file(readF.aio_fildes);
    close_file(writeF.aio_fildes);
    free((void *)readF.aio_buf);
    return;
  }
  readF.aio_nbytes = BUFF_SIZE * (sizeof(char));
  writeF.aio_nbytes = BUFF_SIZE * (sizeof(char));
  readF.aio_sigevent.sigev_notify = SIGEV_NONE;
  writeF.aio_sigevent.sigev_notify = SIGEV_NONE;
}

void copy(volatile void *str, volatile void *str2, int realRead) {

  for (int i = 0; i < realRead; ++i) {
    ((char *)str)[i] = ((char *)str2)[i];
  }
}

void read_and_write_async() {
  const struct aiocb *listRW[2];

  int readB;
  int err;
  int firstI = 1;
  int len = BUFF_SIZE;
  int realRead;
  volatile void *initBuff = readF.aio_buf;
  int i = 0;
  do {
    i++;
    memset((void *)readF.aio_buf, 0, BUFF_SIZE);
    aio_read(&readF);
    if (firstI) {
      listRW[0] = &readF;
      listRW[1] = &writeF;
      aio_suspend(listRW, 1, NULL);
      firstI = 0;
    } else {
      while (aio_error(&readF) == EINPROGRESS ||
             aio_error(&writeF) == EINPROGRESS) {
        listRW[0] = &readF;
        listRW[1] = &writeF;
        aio_suspend(listRW, 2, NULL);
      }

      if ((aio_return(&writeF)) == -1) {
        err = aio_error(&writeF);
        if (err == EINTR || err == EAGAIN)
          goto WR;
        else {
          perror("With write");
          clean_up();
          return;
        }
      }
      writeF.aio_offset += readB;
    }
    readB = aio_return(&readF);
    if ((readB = aio_return(&readF)) == -1) {
      err = aio_error(&readF);
      if (err == EINTR || err == EAGAIN)
        continue;
      else {
        perror("With read");
        clean_up();
        return;
      }
    }
    if (readB < len) {
      realRead = 0;
      len -= readB;
      readF.aio_buf += readB;
      readF.aio_nbytes = len;
      readF.aio_offset += readB;
      realRead += readB;
      do {
        aio_read(&readF);
        aio_suspend(listRW, 1, NULL);

        if ((readB = aio_return(&readF)) == -1) {
          err = aio_error(&readF);
          if (err == EINTR || err == EAGAIN)
            continue;
          else {
            perror("With read");
            clean_up();
            return;
          }
        }
        len -= readB;
        readF.aio_buf += readB;
        readF.aio_nbytes = len;
        readF.aio_offset += readB;
        realRead += readB;
      } while ((len) != 0 && (readB != 0));
      readF.aio_buf = initBuff;
      readF.aio_nbytes = realRead;
      len = BUFF_SIZE;
    } else {
      realRead = readB;
      readF.aio_offset += realRead;
    }

    writeF.aio_nbytes = realRead;

    copy(writeF.aio_buf, readF.aio_buf, realRead);

    if (realRead < 0) {
      continue;
    } else if (realRead == 0) {
      continue;
    }

  WR:
    aio_write(&writeF);
  } while (realRead > 0);
}

int main(int argz, char **argv) {
  init();
  read_and_write_async();
  return 0;
}
