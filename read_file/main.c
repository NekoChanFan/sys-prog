#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500

void close_fd(int fd) {
  int err = close(fd);
  if (err != 0) {
    perror("In close_fd");
  }
}

int main() {
  int inp_fd = open("input.txt", O_RDONLY);
  if (inp_fd < 0) {
    perror("Openning input.txt");
    return -1;
  }

  int outp_fd = open("output.txt", O_RDWR | O_APPEND);
  if (outp_fd < 0) {
    perror("Openning input.txt");
    close_fd(inp_fd);
    return -1;
  }

  ssize_t read_b;
  char *buf = malloc(sizeof(char) * (BUF_SIZE + 1));
  char *init_buf = buf;
  int len = BUF_SIZE;
  int really_read = 0;

  while ((read_b = read(inp_fd, buf, len)) != 0) {
    if (read_b < len) {
      len -= read_b;
      buf += read_b;
      really_read += read_b;
      if (read_b == -1) {
        if (errno == EINTR || errno == EAGAIN)
          continue;
        else {
          perror("While reading input.txt");
          close_fd(inp_fd);
          close_fd(outp_fd);
          free(init_buf);
          return -1;
        }
      }
      while ((len != 0) && ((read_b = read(inp_fd, buf, len)) != 0)) {
        if (read_b == -1) {
          if (errno == EINTR || errno == EAGAIN)
            continue;
          else {
            perror("While reading input.txt");
            close_fd(inp_fd);
            close_fd(outp_fd);
            free(init_buf);
            return -1;
          }
        }
        len -= read_b;
        buf += read_b;
        really_read += read_b;
      }
    }
  again:
    read_b = write(outp_fd, init_buf, really_read);
    if (read_b == -1) {
      if (errno == EINTR || errno == EAGAIN)
        goto again;
      else {
        perror("While reading input.txt");
        close_fd(inp_fd);
        close_fd(outp_fd);
        free(init_buf);
        return -1;
      }
    }

    len = BUF_SIZE;
    buf = init_buf;
  }

  close_fd(inp_fd);
  close_fd(outp_fd);
  free(init_buf);
  return 0;
}
