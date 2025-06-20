#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int lines = 0;
  int longest_num = 0;
  int longest_len = 0;
  int len = 0;
  int ret = 0;
  char buf[4096];
  for (;1;) {
    ret = read(0, buf, 4096);
    if (!ret) break;
    for (int i = 0; i < ret; i++) {
      if (buf[i] == '\n') {
        lines++;
	if (len > longest_len) {
	  longest_len = len;
	  longest_num = lines;
	}
	len = 0;
	continue;
      }
      len++;
    }
  }
  printf("%d lines, line %d was the longest being %d bytes\n", lines,
   longest_num, longest_len);
  return ret;
}
