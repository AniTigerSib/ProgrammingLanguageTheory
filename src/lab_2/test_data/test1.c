int a = 152145l;
int b = 124u + 6/* yrd76346 635*/12/* yrd76346 635*/;
long g = 125125;
unsigned long lg = 524ul;
int as = 0124;
int bs = 0x151l;
int k = 0xL + 0x;
int bk = 5lul;
int c = 065129"23 ";

int main(int argc, char **argv) {
  errno = 0;
  FILE *src = NULL;
  FILE *dest = NULL;
  char src_dir[50] = "test_data/";
  char dst_dir[50] = "test_result/";

  if (argc < 2) {
    errno = EINVAL;
    fprintf(stderr, "Usage: я %s <source_file>\n", argv[0]);
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