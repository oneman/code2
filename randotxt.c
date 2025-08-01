#include "header.h"

int main(int argc, char **argv) {
  int HALT = 1000000;
  int len = 0;
  char str[256];
  char buf[256];
  mset(str, 0, 256);  
  mset(buf, 0, 256);
  int TOP = 0;
  for (;;) {
    if (getrandom(buf, 256, 0) != 256) return HALT;
    for (int i = 0; i < 256; i++) {
      len = text_len((u8 *)&buf[i], 256 - i);
      if (len > TOP) {
        TOP = len;
        printf("%d\n", TOP);
        for (;len--;) {
          printf("%c ", buf[i++]);
        }
        printf("\n");
      }
    }
  }
  return HALT;
}
