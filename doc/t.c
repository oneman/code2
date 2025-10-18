#include <stdio.h>
#include <time.h>

int main(int argc, char **argv) {
  char *str = "ThirteenChars";
  if (argc > 1) str = argv[1];
  time_t current_time = time(NULL);
  if (current_time == ((time_t) -1)) { perror("time"); return 1; }
  printf("%ld %s\n%s", current_time, str, ctime(&current_time));
  return 0;
}
