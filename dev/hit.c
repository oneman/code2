#include <stdio.h>
#include <unistd.h>
typedef struct {
  int number;
  char name[96];
  int sz;
} label;
static label unicode[40454];
int main(int argc, char **argv) {
  printf("hi uc %zu\n", sizeof(unicode));
  return 0;
}
