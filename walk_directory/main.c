#include <asm-generic/errno-base.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void print_spaces(int n) {
  for (; n > 0; n--) {
    printf(" ");
  }
}

void walk_dir(char *path, int depth) {
  struct stat buff;
  DIR *cur_dir = opendir(path);
  if (cur_dir == NULL) {
    perror("Openning directory");
    return;
  }

  struct dirent *cur_entr;
  while ((cur_entr = readdir(cur_dir)) != NULL) {
    char *new_path =
        malloc(sizeof(char) * (strlen(path) + strlen(cur_entr->d_name) + 2));
    sprintf(new_path, "%s/%s", path, cur_entr->d_name);
    stat(new_path, &buff);
    if (cur_entr->d_name[0] == '.')
      continue;
    if ((buff.st_mode & S_IFMT) == S_IFDIR) {
      print_spaces(depth);
      printf("%s\n", cur_entr->d_name);
      walk_dir(new_path, depth + 1);
    } else {
      print_spaces(depth);
      printf("%s\n", cur_entr->d_name);
    }
  }

  if (errno == EBADF) {
    perror("Reading next entry with readdir()");
  }
  closedir(cur_dir);
}

int main(int argc, char *argv[]) {
  if (!(argc == 2)) {
    return -1;
  }
  walk_dir(argv[1], 0);
  return 0;
}
