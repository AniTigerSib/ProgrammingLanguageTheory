#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  DEFAULT = 0,
  FIRST_COM_SYMBOL,
  IN_COMMENT,
  FIRST_UNCOM_SYMBOL
} status_e;

void comment_remove(FILE *src, FILE *dest);

int main(int argc, char **argv) {
  errno = 0;
  FILE *src = NULL;
  FILE *dest = NULL;
  char src_dir[50] = "test_data/";
  char dst_dir[50] = "test_result/";

  if (argc < 2) {
    errno = EINVAL;
    fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
  }

  if (errno == 0) {
    strcat(src_dir, argv[1]);
    strcat(dst_dir, argv[1]);

    src = fopen(src_dir, "r");
    if (src == NULL) {
      perror(src_dir);
    }
  }

  if (errno == 0) {
    dest = fopen(dst_dir, "w");
    if (dest == NULL) {
      perror(dst_dir);
      fclose(src);
    }
  }
  
  if (errno == 0) {
    comment_remove(src, dest);
    fclose(src);
    fclose(dest);
  }

  return errno;
}

void comment_remove(FILE *src, FILE *dest) {
  int c = 0;
  status_e status = DEFAULT;

  while ((c = fgetc(src)) != EOF) {
    if (status == DEFAULT) {
      if (c == '/') {
        status = FIRST_COM_SYMBOL;
        continue;
      }
      fprintf(dest, "%c", c);
    } else if (status == FIRST_COM_SYMBOL) {
      if (c == '*') {
        status = IN_COMMENT;
        continue;
      } else if (c != '/') {
        status = DEFAULT;
        fprintf(dest, "%c", '/');
        fprintf(dest, "%c", c);
      } else {
        fprintf(dest, "%c", '/');
      }
    } else if (status == IN_COMMENT && c == '*') {
      status = FIRST_UNCOM_SYMBOL;
    } else if (status == FIRST_UNCOM_SYMBOL) {
      if (c == '/') {
        status = DEFAULT;
        fprintf(dest, "%c", ' ');
      } else if (c != '*') {
        status = IN_COMMENT;
      }
    }
  }
}