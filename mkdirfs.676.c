#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

int main() {
  char name[4];
  for (char c1 = 'a'; c1 != '{'; c1++) {
    sprintf(name, "%c", c1);
    printf("mkdir %s\n", name);
    mkdir(name, S_IRWXU | S_IRWXG | S_IROTH);
    for (char c2 = 'a'; c2 != '{'; c2++) {
      sprintf(name, "%c/%c", c1, c2);
      printf("mkdir %s\n", name);
      mkdir(name, S_IRWXU | S_IRWXG | S_IROTH);
    }
  }
  return 676;
}
