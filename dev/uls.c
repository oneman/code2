#include <stdio.h>
#include <unistd.h>
typedef struct {
  char a[96];
  int n;
  int sz;
} codeblockpointname;
#define UNICODEVERSION16NAMECOUNT 40454
#define UNICODEVERSION16VOIDCOUNT 1073658
#define UNICODESETSIZE 1114112
#define UNCSZ UNICODEVERSION16NAMECOUNT
static codeblockpointname un[UNICODEVERSION16NAMECOUNT] = {
#include "unicode16codepointsblockblocknames.h"
};
int main(int argc, char **argv) {
  for (int i = 0; i < 40454; i++) {
    printf("%d %d %d %s\n", i, un[i].n, un[i].sz, un[i].a);
  }
  return 0;
}
