#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500

void close_fd(int fd){
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
  char buf[BUF_SIZE + 1];
  int len = BUF_SIZE;

  while ((read_b = read(inp_fd, buf, len)) != 0) {
    if (read_b == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      else {
        perror("While reading input.txt");
        close_fd(inp_fd);
        close_fd(outp_fd);
        return -1;
      }
    }

    read_b = write(outp_fd, buf, read_b);
    if (read_b == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      else {
        perror("While reading input.txt");
        return -1;
      }
    }
  }

  close_fd(inp_fd);
  close_fd(outp_fd);
  return 0;
}
