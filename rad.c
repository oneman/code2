#include <stdio.h>
int radical(int n) {
    if (n < 1) return 0;
    int radn = 1;
    if (n % 2 == 0) {
        radn *= 2;
        while (n % 2 == 0) n /= 2;
    }
    for (int i = 3; i * i <= n; i += 2) {
      if (n % i == 0) {
      radn *= i;
      while (n % i == 0) n /= i;
    }
  }
  return  n * radn;
}

int main() {
  int n = 1;
  int nr = 0;
  for (;n<4294967295;n++) {
    if (n == radical(n)) continue;
    printf("#%d rad(%d) = %d\n", ++nr, n, radical(n));
  }
  return 0;
}
