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
    char *a = un[i].a;
    int n = un[i].n;
    int sz = un[i].sz;        
    //printf("%d %d %d %s\n", i, n, sz, a);
    if (sz == 1) {
      if (((n >= 8192) && (n <= 11264)) || ((n >= 127024) && (n <= 130048))) {
        printf("%d %d %d %s\n", i, n, sz, a);
      } else {
        //printf("%d %d %d %s\n", i, n, sz, a);
      }
    }
  }
  return 0;
}
